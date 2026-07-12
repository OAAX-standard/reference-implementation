# Architecture Rules

## C API Stability

The 9 functions in `runtime-library/include/oaax_runtime.h` are public API. **Never change existing signatures.** New arguments must go through the `Config` key-value struct passed to `runtime_init()`.

## Config Key Documentation Sync

Any change to the config keys read via `config_get()` in `runtime_core.cpp` MUST update, in the same commit: the `runtime_init` doc comment in `oaax_runtime.h`, the config table in `runtime-library/README.md`, and root `CLAUDE.md`. Stale key docs cause silent no-op configs for API consumers.

## Threading

Inference runs on replica worker threads — N replicas per model, spawned by `runtime_load_models()` (N from the `perf_mode` CPU budget or the `num_replicas` override; ONNX Runtime inter-op threads are fixed at 1). Input arrives via `moodycamel::ConcurrentQueue`, workers wake via semaphore, output is returned via a second queue. Do not add synchronous inference paths that bypass this model.

## Dependencies

Libraries in `runtime-library/deps/` are pre-compiled binaries. Do not modify them. If a dependency upgrade is needed, replace the entire pre-compiled set for all platforms simultaneously.

## Platform Coverage

Any change to `runtime-library/src/` must be verified (or at minimum be plausible) for all three platforms: Linux X86_64, Linux AARCH64, Windows x86_64. CMakeLists.txt uses platform guards — keep them intact.

## Artifacts

Build outputs always land in `*/artifacts/`. Never commit build artifacts to git.

## Conversion Toolchain

The Docker image is the unit of distribution. Changes to Python code require a full Docker rebuild to verify. The container runs offline — no internet access at inference time.
