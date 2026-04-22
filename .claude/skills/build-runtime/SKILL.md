---
name: build-runtime
description: Knows how to build the OAAX runtime library for all supported platforms
when_to_use: When building, verifying, or troubleshooting the C++ runtime library
user-invocable: false
---

## Building the Runtime Library

### Linux (X86_64 and AARCH64)

```bash
bash runtime-library/build-runtimes.sh            # both platforms
bash runtime-library/build-runtimes.sh X86_64     # single platform
bash runtime-library/build-runtimes.sh AARCH64
```

Outputs: `runtime-library/artifacts/runtime-library-{ARCH}.tar.gz`

### Windows

```bat
cd runtime-library
.\build-runtime.bat
```

## Prerequisites

Toolchains must be installed in `/opt/`:
- X86_64: `/opt/x86_64-unknown-linux-gnu-gcc-9.5.0/`
- AARCH64: `/opt/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/`

Run `sudo bash scripts/setup-env.sh` if toolchains are missing.

## Key Build Flags

- X86_64: `-march=haswell -fopenmp -Ofast -s`
- AARCH64: `-fopenmp -Ofast -s`
- Both use `-fno-math-errno`

## Troubleshooting

- If CMake can't find the toolchain, verify `/opt/` paths match `runtime-library/CMakeLists.txt`
- If ONNX Runtime symbols are missing, check `runtime-library/deps/onnxruntime/` has the right arch subfolder
- Build artifacts go to `runtime-library/artifacts/` — never to `build/` directly
