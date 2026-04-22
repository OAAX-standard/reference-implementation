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

**Runtime Library** — runs at inference time:
- Loads optimized ONNX via ONNX Runtime C++ API
- Exposes a pure C interface so any language can call it
- Async inference via one dedicated thread + two lock-free queues

## C API Contract (must never break)

```c
int runtime_initialization_with_args(int length, char **keys, void **values);
int runtime_initialization();
int runtime_model_loading(const char *model_path);
int send_input(tensors_struct *input_tensors);
int receive_output(tensors_struct **output_tensors);
int runtime_destruction();
const char *runtime_error_message();
const char *runtime_version();
const char *runtime_name();
```

Supported init args: `log_level`, `log_file`, `number_of_threads`.

## Platform Support

| Platform | Toolchain | Notes |
|----------|-----------|-------|
| Linux X86_64 | GCC 9.5.0 | `-march=haswell` |
| Linux AARCH64 | GCC ARM 9.2 | Cross-compiled |
| Windows x86_64 | MSVC | Separate `.bat` build |

## Versioning

Version string lives in `VERSION` at repo root. Bump it for every release. CI uses it to name S3 artifacts.

## Design Constraints

- Pre-compiled deps in `runtime-library/deps/` must not be modified
- Build artifacts always go to `*/artifacts/` directories
- The runtime is a shared library (`.so`/`.dll`) — no main(), no executable
- Thread safety: callers serialize through `send_input`/`receive_output` queues
