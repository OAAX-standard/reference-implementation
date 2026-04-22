/**
 * Runtime lifecycle tests.
 *
 * Verifies that the runtime can be safely initialized, used, torn down, and
 * re-initialized an arbitrary number of times, with correct state resets at
 * each boundary.
 *
 * Tests that do not require a model file (always run):
 *   1. API guards before first init — functions return NOT_INITIALIZED
 *   2. Init/cleanup cycle repeated 5 times without a model
 *   3. State is clean after cleanup: get_info returns null, get_error returns null
 *   4. Error string cleared by cleanup; stays null after a clean reinit
 *   5. Enqueue/retrieve after cleanup return NOT_INITIALIZED
 *
 * Tests that require a model file (pass .onnx path as first argument):
 *   6. Full init/load/cleanup cycle repeated 3 times
 *   7. runtime_get_info() reflects correct loaded_models count
 *   8. Double runtime_load_models without cleanup returns ALREADY_INITIALIZED
 *   9. Inference round-trip succeeds on cycle 2 (after a full teardown/reinit)
 *
 * Usage:
 *   ./lifecycle_test [model.onnx]
 */

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "oaax_runtime.h"

#define PASS(msg) std::cout << "  PASS: " << msg << std::endl
#define FAIL(msg)                                    \
    do {                                             \
        std::cerr << "  FAIL: " << msg << std::endl; \
        runtime_cleanup();                           \
        return 1;                                    \
    } while (0)
#define ASSERT(cond, msg)       \
    do {                        \
        if (!(cond)) FAIL(msg); \
    } while (0)

static Config make_cfg() {
    static const char* keys[] = {"log_level"};
    static const char* vals[] = {"2"};
    return Config{1, keys, vals};
}

static void free_tensors(Tensors* t) {
    if (!t) return;
    for (int i = 0; i < t->num_tensors; ++i) {
        free(t->tensors[i].name);
        free(t->tensors[i].shape);
        free(t->tensors[i].data);
    }
    free(t->tensors);
    free(t);
}

// Allocate a zero-filled float32 YOLO input (1 x 3 x imgsz x imgsz).
static Tensors* make_yolo_input(int imgsz) {
    Tensors* t = (Tensors*)calloc(1, sizeof(Tensors));
    if (!t) return nullptr;
    t->id = 1;
    t->num_tensors = 1;
    t->tensors = (TensorDescriptor*)calloc(1, sizeof(TensorDescriptor));
    if (!t->tensors) {
        free(t);
        return nullptr;
    }

    TensorDescriptor& td = t->tensors[0];
    td.name = strdup("images");
    td.data_type = DATA_TYPE_FLOAT;
    td.rank = 4;
    td.shape = (int*)malloc(4 * sizeof(int));
    if (!td.shape) {
        free_tensors(t);
        return nullptr;
    }
    td.shape[0] = 1;
    td.shape[1] = 3;
    td.shape[2] = imgsz;
    td.shape[3] = imgsz;

    size_t n = (size_t)3 * imgsz * imgsz;
    td.data_size = n * sizeof(float);
    td.data = calloc(n, sizeof(float));
    if (!td.data) {
        free_tensors(t);
        return nullptr;
    }
    return t;
}

int main(int argc, char** argv) {
    const char* model_path = (argc > 1) ? argv[1] : nullptr;
    Config cfg = make_cfg();

    std::cout << "=== OAAX v2 Runtime Lifecycle Tests ===" << std::endl;

    // ── 1. API guards before any init ────────────────────────────────────────
    std::cout << "\n[1] API guards before runtime_init()" << std::endl;
    {
        Tensors dummy{};
        int mid = -1;
        Tensors* out = nullptr;
        ASSERT(runtime_load_models(0, nullptr) == RUNTIME_STATUS_NOT_INITIALIZED,
               "load_models before init should return NOT_INITIALIZED");
        ASSERT(runtime_enqueue_input(0, &dummy) == RUNTIME_STATUS_NOT_INITIALIZED,
               "enqueue before init should return NOT_INITIALIZED");
        ASSERT(runtime_retrieve_output(&mid, &out, 0) == RUNTIME_STATUS_NOT_INITIALIZED,
               "retrieve before init should return NOT_INITIALIZED");
        ASSERT(runtime_get_info() == nullptr, "get_info before init should return nullptr");
    }
    PASS("all pre-init guards hold");

    // ── 2. Repeated init/cleanup cycles without a model ──────────────────────
    std::cout << "\n[2] Init/cleanup cycle x5 (no model)" << std::endl;
    for (int i = 1; i <= 5; ++i) {
        ASSERT(runtime_init(cfg) == RUNTIME_STATUS_SUCCESS,
               std::string("cycle ") + std::to_string(i) + ": runtime_init failed");
        ASSERT(runtime_init(cfg) == RUNTIME_STATUS_ALREADY_INITIALIZED,
               std::string("cycle ") + std::to_string(i) + ": double-init should return ALREADY_INITIALIZED");
        ASSERT(runtime_cleanup() == RUNTIME_STATUS_SUCCESS,
               std::string("cycle ") + std::to_string(i) + ": runtime_cleanup failed");
        ASSERT(runtime_cleanup() == RUNTIME_STATUS_SUCCESS,
               std::string("cycle ") + std::to_string(i) + ": second cleanup should be idempotent");
    }
    PASS("5 init/cleanup cycles with double-init and idempotent-cleanup checks");

    // ── 3. State clean after cleanup ─────────────────────────────────────────
    std::cout << "\n[3] State is clean after cleanup" << std::endl;
    ASSERT(runtime_get_info() == nullptr, "get_info after cleanup must return nullptr");
    ASSERT(runtime_get_error() == nullptr, "get_error after cleanup must return nullptr");
    PASS("state clean after cleanup");

    // ── 4. Error string cleared by cleanup ───────────────────────────────────
    std::cout << "\n[4] Error string cleared by cleanup; stays null after clean reinit" << std::endl;
    {
        ASSERT(runtime_init(cfg) == RUNTIME_STATUS_SUCCESS, "init failed");
        ModelConfig bad{};
        bad.file_path = "/nonexistent/path/model.onnx";
        ASSERT(runtime_load_models(1, &bad) != RUNTIME_STATUS_SUCCESS, "bad-path load should fail");
        ASSERT(runtime_get_error() != nullptr, "error should be set after failed load");

        ASSERT(runtime_cleanup() == RUNTIME_STATUS_SUCCESS, "cleanup failed");
        ASSERT(runtime_get_error() == nullptr, "error should be cleared after cleanup");

        ASSERT(runtime_init(cfg) == RUNTIME_STATUS_SUCCESS, "reinit after error cycle failed");
        ASSERT(runtime_get_error() == nullptr, "error should remain null after clean reinit");
        ASSERT(runtime_cleanup() == RUNTIME_STATUS_SUCCESS, "cleanup failed");
    }
    PASS("error string is cleared by cleanup and absent after clean reinit");

    // ── 5. Enqueue/retrieve after cleanup return NOT_INITIALIZED ─────────────
    std::cout << "\n[5] Enqueue/retrieve after cleanup return NOT_INITIALIZED" << std::endl;
    {
        ASSERT(runtime_init(cfg) == RUNTIME_STATUS_SUCCESS, "init failed");
        ASSERT(runtime_cleanup() == RUNTIME_STATUS_SUCCESS, "cleanup failed");

        Tensors dummy{};
        int mid = -1;
        Tensors* out = nullptr;
        ASSERT(runtime_enqueue_input(0, &dummy) == RUNTIME_STATUS_NOT_INITIALIZED,
               "enqueue after cleanup must return NOT_INITIALIZED");
        ASSERT(runtime_retrieve_output(&mid, &out, 0) == RUNTIME_STATUS_NOT_INITIALIZED,
               "retrieve after cleanup must return NOT_INITIALIZED");
    }
    PASS("enqueue/retrieve after cleanup return NOT_INITIALIZED");

    // ── Model-dependent tests ─────────────────────────────────────────────────
    if (!model_path) {
        std::cout << "\n[6-9] Skipping model-dependent tests (no model path provided)" << std::endl;
        std::cout << "      Usage: " << argv[0] << " <model.onnx>" << std::endl;
        std::cout << "\n=== All tests passed ===" << std::endl;
        return 0;
    }

    // ── 6. Full init/load/cleanup cycle repeated 3 times ─────────────────────
    std::cout << "\n[6] Full init/load/cleanup cycle x3: " << model_path << std::endl;
    for (int i = 1; i <= 3; ++i) {
        ASSERT(runtime_init(cfg) == RUNTIME_STATUS_SUCCESS,
               std::string("cycle ") + std::to_string(i) + ": init failed");
        ModelConfig mc{};
        mc.file_path = model_path;
        ASSERT(runtime_load_models(1, &mc) == RUNTIME_STATUS_SUCCESS,
               std::string("cycle ") + std::to_string(i) +
                   ": load failed: " + (runtime_get_error() ? runtime_get_error() : ""));
        ASSERT(runtime_cleanup() == RUNTIME_STATUS_SUCCESS,
               std::string("cycle ") + std::to_string(i) + ": cleanup failed");
    }
    PASS("3 full init/load/cleanup cycles completed");

    // ── 7. runtime_get_info reflects correct loaded_models count ─────────────
    std::cout << "\n[7] runtime_get_info() reports correct loaded_models count" << std::endl;
    {
        ASSERT(runtime_init(cfg) == RUNTIME_STATUS_SUCCESS, "init failed");
        const char* info = runtime_get_info();
        ASSERT(info && strstr(info, "\"loaded_models\":0"),
               std::string("expected 0 loaded_models before load, got: ") + (info ? info : "null"));

        ModelConfig mc{};
        mc.file_path = model_path;
        ASSERT(runtime_load_models(1, &mc) == RUNTIME_STATUS_SUCCESS,
               std::string("load failed: ") + (runtime_get_error() ? runtime_get_error() : ""));

        info = runtime_get_info();
        ASSERT(info && strstr(info, "\"loaded_models\":1"),
               std::string("expected 1 loaded_models after load, got: ") + (info ? info : "null"));

        ASSERT(runtime_cleanup() == RUNTIME_STATUS_SUCCESS, "cleanup failed");

        // After cleanup get_info must return null again
        ASSERT(runtime_get_info() == nullptr, "get_info after cleanup must return nullptr");
    }
    PASS("get_info loaded_models count is correct before/after load and after cleanup");

    // ── 8. Double load without cleanup returns ALREADY_INITIALIZED ────────────
    std::cout << "\n[8] Double runtime_load_models without cleanup returns ALREADY_INITIALIZED" << std::endl;
    {
        ASSERT(runtime_init(cfg) == RUNTIME_STATUS_SUCCESS, "init failed");
        ModelConfig mc{};
        mc.file_path = model_path;
        ASSERT(runtime_load_models(1, &mc) == RUNTIME_STATUS_SUCCESS,
               std::string("first load failed: ") + (runtime_get_error() ? runtime_get_error() : ""));
        ASSERT(runtime_load_models(1, &mc) == RUNTIME_STATUS_ALREADY_INITIALIZED,
               "second load without cleanup should return ALREADY_INITIALIZED");
        ASSERT(runtime_cleanup() == RUNTIME_STATUS_SUCCESS, "cleanup failed");
    }
    PASS("double load_models rejected with ALREADY_INITIALIZED");

    // ── 9. Inference round-trip works on cycle 2 ─────────────────────────────
    std::cout << "\n[9] Inference round-trip works on cycle 2 (after full teardown/reinit)" << std::endl;
    {
        ModelConfig mc{};
        mc.file_path = model_path;

        // Cycle 1 — load and immediately clean up without running any inference.
        ASSERT(runtime_init(cfg) == RUNTIME_STATUS_SUCCESS, "cycle 1: init failed");
        ASSERT(runtime_load_models(1, &mc) == RUNTIME_STATUS_SUCCESS,
               std::string("cycle 1: load failed: ") + (runtime_get_error() ? runtime_get_error() : ""));
        ASSERT(runtime_cleanup() == RUNTIME_STATUS_SUCCESS, "cycle 1: cleanup failed");

        // Cycle 2 — full inference round-trip.
        ASSERT(runtime_init(cfg) == RUNTIME_STATUS_SUCCESS, "cycle 2: init failed");
        ASSERT(runtime_load_models(1, &mc) == RUNTIME_STATUS_SUCCESS,
               std::string("cycle 2: load failed: ") + (runtime_get_error() ? runtime_get_error() : ""));

        Tensors* input = make_yolo_input(640);
        ASSERT(input != nullptr, "cycle 2: failed to allocate input tensor");

        ASSERT(runtime_enqueue_input(0, input) == RUNTIME_STATUS_SUCCESS, "cycle 2: enqueue failed");

        int out_mid = -1;
        Tensors* output = nullptr;
        RuntimeStatus st = runtime_retrieve_output(&out_mid, &output, 5000);
        ASSERT(st == RUNTIME_STATUS_SUCCESS,
               std::string("cycle 2: retrieve failed, status=") + std::to_string((int)st));
        ASSERT(output != nullptr, "cycle 2: output is null");
        ASSERT(out_mid == 0, std::string("cycle 2: wrong model_id=") + std::to_string(out_mid));
        free_tensors(output);

        // Queue must be empty after draining the single result.
        Tensors* leftover = nullptr;
        int dummy_mid = -1;
        ASSERT(runtime_retrieve_output(&dummy_mid, &leftover, 0) == RUNTIME_STATUS_NO_OUTPUT_AVAILABLE,
               "cycle 2: output queue should be empty after retrieving the result");

        ASSERT(runtime_cleanup() == RUNTIME_STATUS_SUCCESS, "cycle 2: cleanup failed");
    }
    PASS("inference round-trip succeeds on cycle 2 after full teardown/reinit");

    std::cout << "\n=== All tests passed ===" << std::endl;
    return 0;
}
