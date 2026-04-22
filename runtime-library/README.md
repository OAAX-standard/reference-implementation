# OAAX Runtime Library

A reference implementation of an OAAX runtime that loads and runs optimized ONNX models on CPU using [ONNX Runtime](https://github.com/microsoft/onnxruntime).

## Pre-requisites

Set up the cross-compilation toolchains and CMake (run once, requires root):

```bash
sudo bash scripts/setup-env.sh
```

## Building

```bash
bash build-runtimes.sh X86_64    # Linux x86_64
bash build-runtimes.sh AARCH64   # Linux ARM64
bash build-runtimes.sh           # both platforms
```

Output: `artifacts/runtime-library-{ARCH}.tar.gz` containing `libRuntimeLibrary.so`.

Dependencies (ONNX Runtime 1.21.1, RE2, cpuinfo, spdlog) are pre-compiled in `deps/` and do not need to be rebuilt.

## API

The public API is declared in `include/oaax_runtime.h`. The expected call sequence is:

```
runtime_init()  →  runtime_load_models()  →  runtime_enqueue_input() / runtime_retrieve_output()  →  runtime_cleanup()
```

### Functions

| Function | Description |
|---|---|
| `runtime_init(Config config)` | Initialize the runtime. Must be called before anything else. |
| `runtime_load_models(int n, ModelConfig* configs)` | Load one or more ONNX models and start their worker threads. |
| `runtime_enqueue_input(int model_id, Tensors* input)` | Submit an input tensor batch to the specified model's queue. |
| `runtime_retrieve_output(int* model_id, Tensors** output, int timeout_ms)` | Dequeue an inference result. Use `timeout_ms < 0` to block indefinitely, `0` for non-blocking. Returns `RUNTIME_STATUS_NO_OUTPUT_AVAILABLE` on timeout. |
| `runtime_cleanup()` | Stop all worker threads and free all resources. Safe to call even if not fully initialized. |
| `runtime_get_error()` | Returns the last error string, or `NULL` if none. Always call after a non-zero status. |
| `runtime_get_version()` | Returns the runtime version string. |
| `runtime_get_name()` | Returns the runtime name string. |
| `runtime_get_info()` | Returns a JSON string with runtime diagnostics (loaded models, requests in flight, backend version). |

### Init args (`Config` key-value pairs)

| Key | Type | Default | Description |
|---|---|---|---|
| `log_level` | int (0–6) | `2` (info) | spdlog log level: 0=trace, 1=debug, 2=info, 3=warn, 4=error, 5=critical, 6=off |
| `log_file` | string | `runtime.log` | Path to the log file |
| `num_threads` | int (1–16) | `4` | ONNX Runtime intra-op thread count (can also be set per-model in `ModelConfig.config`) |

### Error handling pattern

```c
RuntimeStatus s = runtime_init(config);
if (s != RUNTIME_STATUS_SUCCESS) {
    const char* err = runtime_get_error();
    fprintf(stderr, "init failed: %s\n", err ? err : "(no details)");
}
```

## Running Inference

See the [examples](https://github.com/oaax-standard/examples) repository for complete usage samples.
