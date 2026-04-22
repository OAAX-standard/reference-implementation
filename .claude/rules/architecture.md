# Architecture Rules

## C API Stability

The 9 functions in `runtime-library/include/runtime_core.hpp` are public API. **Never change existing signatures.** New arguments must go through `runtime_initialization_with_args` key-value pairs.

## Threading

All inference runs on a single dedicated thread spawned at initialization. Input arrives via `moodycamel::ConcurrentQueue`, output is returned via a second queue. Do not add synchronous inference paths that bypass this model.

## Dependencies

Libraries in `runtime-library/deps/` are pre-compiled binaries. Do not modify them. If a dependency upgrade is needed, replace the entire pre-compiled set for all platforms simultaneously.

## Platform Coverage

Any change to `runtime-library/src/` must be verified (or at minimum be plausible) for all three platforms: Linux X86_64, Linux AARCH64, Windows x86_64. CMakeLists.txt uses platform guards — keep them intact.

## Artifacts

Build outputs always land in `*/artifacts/`. Never commit build artifacts to git.

## Conversion Toolchain

The Docker image is the unit of distribution. Changes to Python code require a full Docker rebuild to verify. The container runs offline — no internet access at inference time.
