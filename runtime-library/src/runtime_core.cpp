#include <atomic>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
struct sem_t {
    HANDLE h;
};
static inline int sem_init(sem_t* s, int, unsigned int value) {
    s->h = CreateSemaphoreW(nullptr, (LONG)value, 0x7FFFFFFF, nullptr);
    return s->h ? 0 : -1;
}
static inline int sem_wait(sem_t* s) { return WaitForSingleObject(s->h, INFINITE) == WAIT_OBJECT_0 ? 0 : -1; }
static inline int sem_trywait(sem_t* s) { return WaitForSingleObject(s->h, 0) == WAIT_OBJECT_0 ? 0 : -1; }
static inline int sem_timedwait_ms(sem_t* s, int ms) {
    return WaitForSingleObject(s->h, (DWORD)ms) == WAIT_OBJECT_0 ? 0 : -1;
}
static inline int sem_post(sem_t* s) { return ReleaseSemaphore(s->h, 1, nullptr) ? 0 : -1; }
static inline int sem_destroy(sem_t* s) { return CloseHandle(s->h) ? 0 : -1; }
#else
#include <errno.h>
#include <semaphore.h>
#include <time.h>
static inline int sem_timedwait_ms(sem_t* s, int ms) {
    struct timespec deadline;
    clock_gettime(CLOCK_REALTIME, &deadline);
    deadline.tv_sec += ms / 1000;
    deadline.tv_nsec += (long)(ms % 1000) * 1000000L;
    if (deadline.tv_nsec >= 1000000000L) {
        deadline.tv_sec++;
        deadline.tv_nsec -= 1000000000L;
    }
    int r;
    do {
        r = sem_timedwait(s, &deadline);
    } while (r == -1 && errno == EINTR);
    return r;
}
#endif

#include <onnxruntime_cxx_api.h>
#include <spdlog/spdlog.h>

#include "concurrentqueue.h"
#include "oaax_runtime.h"
#include "runtime_utils.hpp"

// ─── Internal helpers ────────────────────────────────────────────────────────

static void deep_free_tensors(Tensors* t) {
    if (!t) return;
    if (t->tensors) {
        for (int i = 0; i < t->num_tensors; ++i) {
            free(t->tensors[i].name);
            free(t->tensors[i].shape);
            free(t->tensors[i].data);
        }
        free(t->tensors);
    }
    free(t);
}

// ─── Per-model state ─────────────────────────────────────────────────────────

struct ModelState {
    int id{0};
    std::unique_ptr<Ort::SessionOptions> session_options;
    std::unique_ptr<Ort::Session> session;
    std::vector<std::string> input_names;
    std::vector<std::string> output_names;
    moodycamel::ConcurrentQueue<Tensors*> input_queue;
    std::atomic<bool> stop{false};
    std::thread worker_thread;
};

struct OutputItem {
    int model_id;
    Tensors* tensors;
};

// ─── Global state ────────────────────────────────────────────────────────────

static std::unique_ptr<Ort::Env> g_env;
static std::vector<ModelState*> g_models;
static std::atomic<bool> g_initialized{false};
static std::atomic<bool> g_models_loaded{false};

static moodycamel::ConcurrentQueue<OutputItem> g_output_queue;
static sem_t g_output_sem;

static std::shared_ptr<spdlog::logger> g_logger;
static std::string g_last_error;
static std::string g_info_json;

static int g_log_level = spdlog::level::info;
static std::string g_log_file = "runtime.log";
static int g_num_threads = 4;

// ─── Config helpers ───────────────────────────────────────────────────────────

static std::string config_get(const Config& cfg, const char* key, const std::string& fallback) {
    for (int i = 0; i < cfg.length; ++i)
        if (cfg.keys[i] && strcmp(cfg.keys[i], key) == 0) return cfg.values[i] ? cfg.values[i] : fallback;
    return fallback;
}

static void set_error(const std::string& msg) {
    g_last_error = msg;
    if (g_logger) g_logger->error("{}", msg);
}

// ─── Wait-for-output helper ───────────────────────────────────────────────────

static bool wait_for_output(int timeout_ms) {
    if (timeout_ms == 0) return sem_trywait(&g_output_sem) == 0;
    if (timeout_ms < 0) {
#ifndef _WIN32
        while (sem_wait(&g_output_sem) == -1 && errno == EINTR) continue;
        return true;
#else
        return WaitForSingleObject(g_output_sem.h, INFINITE) == WAIT_OBJECT_0;
#endif
    }
    return sem_timedwait_ms(&g_output_sem, timeout_ms) == 0;
}

// ─── Output builder ───────────────────────────────────────────────────────────

static Tensors* build_output(int model_id, const std::vector<Ort::Value>& ort_outputs, int request_id) {
    ModelState& m = *g_models[model_id];
    int n = (int)ort_outputs.size();

    Tensors* out = (Tensors*)malloc(sizeof(Tensors));
    if (!out) return nullptr;
    out->id = request_id;
    out->num_tensors = n;
    out->tensors = (TensorDescriptor*)malloc((size_t)n * sizeof(TensorDescriptor));
    if (!out->tensors) {
        free(out);
        return nullptr;
    }

    for (int i = 0; i < n; ++i) {
        auto info = ort_outputs[i].GetTensorTypeAndShapeInfo();
        auto shape = info.GetShape();
        TensorElementType elem_type = map_from_ort_type(info.GetElementType());

        out->tensors[i].name = strdup(m.output_names[i].c_str());
        out->tensors[i].data_type = elem_type;
        out->tensors[i].rank = (int)shape.size();
        out->tensors[i].shape = (int*)malloc(shape.size() * sizeof(int));
        for (size_t j = 0; j < shape.size(); ++j) out->tensors[i].shape[j] = (int)shape[j];

        size_t elem_count = info.GetElementCount();
        size_t elem_size = get_element_byte_size(elem_type);
        out->tensors[i].data_size = elem_count * elem_size;
        out->tensors[i].data = malloc(out->tensors[i].data_size);
        if (out->tensors[i].data)
            memcpy(out->tensors[i].data, ort_outputs[i].GetTensorData<void>(), out->tensors[i].data_size);
    }
    return out;
}

// ─── Per-model worker thread ──────────────────────────────────────────────────

static void worker_loop(int model_id) {
    ModelState& m = *g_models[model_id];
    Ort::MemoryInfo mem_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeDefault);

    while (!m.stop) {
        Tensors* input = nullptr;
        if (!m.input_queue.try_dequeue(input)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        try {
            std::vector<const char*> input_name_ptrs;
            std::vector<Ort::Value> ort_inputs;

            for (int i = 0; i < input->num_tensors; ++i) {
                TensorDescriptor& td = input->tensors[i];
                input_name_ptrs.push_back(td.name);

                std::vector<int64_t> shape;
                for (int j = 0; j < td.rank; ++j) shape.push_back((int64_t)td.shape[j]);

                size_t elem_count = 1;
                for (auto s : shape) elem_count *= (size_t)s;
                size_t data_bytes = elem_count * get_element_byte_size(td.data_type);

                ort_inputs.push_back(Ort::Value::CreateTensor(mem_info, td.data, data_bytes, shape.data(), shape.size(),
                                                              map_to_ort_type(td.data_type)));
            }

            std::vector<const char*> output_name_ptrs;
            for (const auto& n : m.output_names) output_name_ptrs.push_back(n.c_str());

            auto ort_outputs = m.session->Run(Ort::RunOptions{nullptr}, input_name_ptrs.data(), ort_inputs.data(),
                                              ort_inputs.size(), output_name_ptrs.data(), output_name_ptrs.size());

            int request_id = input->id;
            deep_free_tensors(input);

            Tensors* output = build_output(model_id, ort_outputs, request_id);
            if (output) {
                if (g_output_queue.try_enqueue({model_id, output}))
                    sem_post(&g_output_sem);
                else
                    deep_free_tensors(output);
            }
        } catch (const std::exception& e) {
            g_logger->error("[model {}] Inference error: {}", model_id, e.what());
            deep_free_tensors(input);
        }
    }
}

// ─── Public API ───────────────────────────────────────────────────────────────

RuntimeStatus runtime_init(Config config) {
    if (g_initialized) {
        set_error(
            "runtime_init called while already initialized — call "
            "runtime_cleanup() first");
        return RUNTIME_STATUS_ALREADY_INITIALIZED;
    }

    g_log_file = config_get(config, "log_file", "runtime.log");

    try {
        g_log_level = std::stoi(config_get(config, "log_level", "2"));
        if (g_log_level < 0 || g_log_level > 6) g_log_level = spdlog::level::info;
    } catch (...) {
        g_log_level = spdlog::level::info;
    }

    try {
        g_num_threads = std::stoi(config_get(config, "num_threads", "4"));
        if (g_num_threads < 1) g_num_threads = 1;
        if (g_num_threads > 16) g_num_threads = 16;
    } catch (...) {
        g_num_threads = 4;
    }

    try {
        g_logger = initialize_logger(g_log_file, g_log_level, g_log_level, runtime_get_name());
        g_logger->info("Initializing runtime");
        g_logger->info("  log_level:   {}", g_log_level);
        g_logger->info("  log_file:    {}", g_log_file);
        g_logger->info("  num_threads: {}", g_num_threads);

        g_env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_ERROR, runtime_get_name());
        g_initialized = true;
        return RUNTIME_STATUS_SUCCESS;
    } catch (const std::exception& e) {
        g_last_error = e.what();
        if (g_logger) g_logger->error("runtime_init failed: {}", g_last_error);
        return RUNTIME_STATUS_ERROR;
    }
}

RuntimeStatus runtime_load_models(int num_models, const ModelConfig* model_configs) {
    if (!g_initialized) {
        g_last_error = "runtime_load_models: runtime not initialized";
        return RUNTIME_STATUS_NOT_INITIALIZED;
    }
    if (g_models_loaded) {
        set_error("runtime_load_models called twice — call runtime_cleanup() first");
        return RUNTIME_STATUS_ALREADY_INITIALIZED;
    }
    if (num_models <= 0 || !model_configs) {
        set_error("runtime_load_models: invalid arguments");
        return RUNTIME_STATUS_INVALID_ARGUMENT;
    }

    sem_init(&g_output_sem, 0, 0);

    for (int m_idx = 0; m_idx < num_models; ++m_idx) {
        const ModelConfig& mc = model_configs[m_idx];
        ModelState* ms = nullptr;

        try {
            int num_threads = g_num_threads;
            try {
                num_threads = std::stoi(config_get(mc.config, "num_threads", std::to_string(g_num_threads)));
                if (num_threads < 1) num_threads = 1;
                if (num_threads > 16) num_threads = 16;
            } catch (...) {
            }

            ms = new ModelState();
            ms->id = m_idx;
            ms->session_options = std::make_unique<Ort::SessionOptions>();
            ms->session_options->SetIntraOpNumThreads(num_threads);

            if (mc.model_data && mc.model_size > 0) {
                g_logger->info("[model {}] Loading from memory ({} bytes)", m_idx, mc.model_size);
                ms->session =
                    std::make_unique<Ort::Session>(*g_env, mc.model_data, mc.model_size, *ms->session_options);
            } else if (mc.file_path) {
                g_logger->info("[model {}] Loading from: {}", m_idx, mc.file_path);
#ifdef _WIN32
                int n = MultiByteToWideChar(CP_UTF8, 0, mc.file_path, -1, nullptr, 0);
                std::wstring wpath(n, 0);
                MultiByteToWideChar(CP_UTF8, 0, mc.file_path, -1, &wpath[0], n);
                ms->session = std::make_unique<Ort::Session>(*g_env, wpath.c_str(), *ms->session_options);
#else
                ms->session = std::make_unique<Ort::Session>(*g_env, mc.file_path, *ms->session_options);
#endif
            } else {
                set_error("model_configs[" + std::to_string(m_idx) + "]: file_path and model_data are both null");
                delete ms;
                goto fail;
            }

            ms->input_names = get_input_names(*ms->session);
            ms->output_names = get_output_names(*ms->session);

            g_models.push_back(ms);
            ms->worker_thread = std::thread(worker_loop, m_idx);
            g_logger->info("[model {}] Ready ({} inputs, {} outputs, {} threads)", m_idx, ms->input_names.size(),
                           ms->output_names.size(), num_threads);
        } catch (const std::exception& e) {
            set_error("Failed to load model " + std::to_string(m_idx) + ": " + e.what());
            if (ms) delete ms;
            goto fail;
        }
    }

    g_models_loaded = true;
    return RUNTIME_STATUS_SUCCESS;

fail:
    for (auto* ms : g_models) {
        ms->stop = true;
        if (ms->worker_thread.joinable()) ms->worker_thread.join();
        Tensors* t;
        while (ms->input_queue.try_dequeue(t)) deep_free_tensors(t);
        delete ms;
    }
    g_models.clear();
    sem_destroy(&g_output_sem);
    OutputItem item;
    while (g_output_queue.try_dequeue(item)) deep_free_tensors(item.tensors);
    return RUNTIME_STATUS_ERROR;
}

RuntimeStatus runtime_enqueue_input(int model_id, Tensors* input_tensors) {
    if (!g_initialized) return RUNTIME_STATUS_NOT_INITIALIZED;
    if (!g_models_loaded) return RUNTIME_STATUS_MODEL_NOT_LOADED;
    if (model_id < 0 || model_id >= (int)g_models.size()) return RUNTIME_STATUS_INVALID_MODEL_ID;
    if (!input_tensors) {
        set_error("runtime_enqueue_input: null input_tensors");
        return RUNTIME_STATUS_INVALID_ARGUMENT;
    }

    if (!g_models[model_id]->input_queue.try_enqueue(input_tensors)) {
        set_error("runtime_enqueue_input: input queue full");
        return RUNTIME_STATUS_ERROR;
    }
    g_logger->trace("[model {}] Input queued (id={})", model_id, input_tensors->id);
    return RUNTIME_STATUS_SUCCESS;
}

RuntimeStatus runtime_retrieve_output(int* model_id, Tensors** output_tensors, int timeout_ms) {
    if (!g_initialized) return RUNTIME_STATUS_NOT_INITIALIZED;
    if (!model_id || !output_tensors) {
        set_error("runtime_retrieve_output: null output parameter");
        return RUNTIME_STATUS_INVALID_ARGUMENT;
    }

    if (!wait_for_output(timeout_ms)) return RUNTIME_STATUS_NO_OUTPUT_AVAILABLE;

    OutputItem item;
    if (!g_output_queue.try_dequeue(item)) return RUNTIME_STATUS_NO_OUTPUT_AVAILABLE;

    *model_id = item.model_id;
    *output_tensors = item.tensors;
    g_logger->debug("[model {}] Output retrieved (id={})", item.model_id, item.tensors->id);
    return RUNTIME_STATUS_SUCCESS;
}

RuntimeStatus runtime_cleanup(void) {
    if (!g_initialized) return RUNTIME_STATUS_SUCCESS;  // idempotent

    g_logger->info("Cleaning up runtime...");

    for (auto* ms : g_models) {
        ms->stop = true;
        if (ms->worker_thread.joinable()) ms->worker_thread.join();
    }

    for (auto* ms : g_models) {
        Tensors* t;
        while (ms->input_queue.try_dequeue(t)) deep_free_tensors(t);
        delete ms;
    }
    g_models.clear();

    OutputItem item;
    while (g_output_queue.try_dequeue(item)) deep_free_tensors(item.tensors);
    sem_destroy(&g_output_sem);

    g_env.reset();
    g_initialized = false;
    g_models_loaded = false;
    g_last_error.clear();

    g_logger->info("Runtime cleanup complete.");
    destroy_logger(g_logger);
    g_logger = nullptr;

    return RUNTIME_STATUS_SUCCESS;
}

const char* runtime_get_error(void) { return g_last_error.empty() ? nullptr : g_last_error.c_str(); }

const char* runtime_get_version(void) { return RUNTIME_VERSION; }

const char* runtime_get_name(void) { return "OAAX CPU Runtime"; }

const char* runtime_get_info(void) {
    if (!g_initialized) return nullptr;

    int in_flight = 0;
    for (const auto* ms : g_models) in_flight += (int)ms->input_queue.size_approx();

    const char* ort_ver = OrtGetApiBase()->GetVersionString();

    g_info_json = "{";
    g_info_json += "\"loaded_models\":" + std::to_string(g_models.size()) + ",";
    g_info_json += "\"requests_in_flight\":" + std::to_string(in_flight) + ",";
    g_info_json += "\"backend_version\":\"" + std::string(ort_ver ? ort_ver : "unknown") + "\"";
    g_info_json += "}";

    return g_info_json.c_str();
}
