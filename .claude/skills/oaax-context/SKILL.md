---
name: oaax-context
description: Provides architectural and standard context for the OAAX reference implementation
when_to_use: When reasoning about design decisions, API changes, or user-facing behavior
user-invocable: false
---

## What OAAX Is

OAAX (Open Architecture for AI Exchange) is a standard for portable AI model deployment. This repository is the **reference implementation** — it defines what conforming toolchains and runtimes must do.

## Two-Component Architecture

```
ONNX Model → [Conversion Toolchain] → Optimized ONNX → [Runtime Library] → Inference
```

**Conversion Toolchain** — runs once offline:
- Input: arbitrary ONNX model
- Process: simplify via onnxsim (operator fusion, constant folding, dead code elimination)
- Output: optimized `.onnx` + `logs.json`
- Distributed as a Docker image (Python 3.11); runs offline, no internet at conversion time

**Runtime Library** — runs at inference time:
- Loads optimized ONNX via ONNX Runtime C++ API
- Exposes a pure C interface so any language can call it
- Async inference: per-model replica worker threads + two lock-free queues, signaled by semaphores

## C API Contract (v2 — must never break)

The 9 functions in `runtime-library/include/oaax_runtime.h`:

```c
RuntimeStatus runtime_init(Config config);
RuntimeStatus runtime_load_models(int num_models, const ModelConfig *model_configs);
RuntimeStatus runtime_enqueue_input(int model_id, Tensors *input_tensors);
RuntimeStatus runtime_retrieve_output(int *model_id, Tensors **output_tensors, int timeout_ms);
RuntimeStatus runtime_cleanup(void);
const char *runtime_get_error(void);
const char *runtime_get_version(void);
const char *runtime_get_name(void);
const char *runtime_get_info(void);
```

`Config` is a `{length, keys[], values[]}` key-value struct. New capabilities are added
as new Config keys, never as new function signatures.

Supported `runtime_init` config keys: `log_level` (0–6), `log_file` (path),
`perf_mode` (`eco` | `power`), `num_intra_threads` (override, 0 = heuristic),
`num_replicas` (override, 0 = heuristic).

## Threading Model

- `runtime_load_models()` spawns N replica worker threads per model (N derived from
  the `perf_mode` CPU budget, or forced via `num_replicas`). Inter-op threads are
  fixed at 1; throughput scales via replicas.
- `runtime_enqueue_input()` pushes to a lock-free input queue; workers wake via
  semaphore, run the ONNX Runtime session, and push to the output queue.
- `runtime_retrieve_output()` blocks up to `timeout_ms` (`0` = non-blocking poll).

## Platform Support

| Platform | Toolchain | Notes |
|----------|-----------|-------|
| Linux X86_64 | GCC 9.5.0 | `-march=haswell` |
| Linux AARCH64 | GCC ARM 9.2 | Cross-compiled |
| Windows x86_64 | MSVC | Separate `.bat` build |

Backend: ONNX Runtime 1.21.1 (pre-built, in `runtime-library/deps/`).

## Versioning

Version string lives in `VERSION` at repo root. Bump it for every release. CI uses it to name S3 artifacts and tag the toolchain Docker image.

## Design Constraints

- Pre-compiled deps in `runtime-library/deps/` must not be modified
- Build artifacts always go to `*/artifacts/` directories
- The runtime is a shared library (`.so`/`.dll`) — no main(), no executable
- All inference goes through the queue system — no synchronous inference paths
