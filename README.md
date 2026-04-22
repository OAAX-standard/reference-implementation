# Reference Implementation

[![CI](https://github.com/OAAX-standard/reference-implementation/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/OAAX-standard/reference-implementation/actions/workflows/ci.yml)

This repository contains the source code for building the reference implementation of the OAAX runtime and conversion toolchain.

> If you're new to OAAX, you can find more information about the OAAX standard in
> the [OAAX repository](https://github.com/oaax-standard/OAAX)

It builds on top of the illustrative implementation provided in
the [OAAX standard repository](https://github.com/oaax-standard/OAAX/Illustrative%20example). Please check it out first to get a high-level understanding on how an OAAX runtime and conversion toolchain must be implemented.

## How the two components fit together

```
ONNX model
    │
    ▼
[Conversion Toolchain]  — Docker image; runs onnxsim to produce an optimized ONNX + logs.json
    │
    ▼
Optimized ONNX
    │
    ▼
[Runtime Library]  — C shared library; loads the optimized model and runs inference via the C API
    │
    ▼
Inference results
```

The toolchain and runtime are independent — the toolchain output is a plain `.onnx` file that any ONNX Runtime-compatible consumer can use.

## Repository structure

- [Conversion toolchain](conversion-toolchain): Docker-based model optimizer.
- [Runtime library](runtime-library): C shared library with a 9-function C API (`oaax_runtime.h`).

## Building the reference implementation

### 1. Set up the environment (once, requires root)

```bash
sudo bash scripts/setup-env.sh
```

This installs cross-compilation toolchains to `/opt/` and CMake 3.31.7. Required only for Linux builds.

### 2. Build the runtime library (Linux)

```bash
bash runtime-library/build-runtimes.sh X86_64    # or AARCH64, or omit for both
```

Output: `runtime-library/artifacts/runtime-library-{ARCH}.tar.gz`

### 3. Build the runtime library (Windows)

```bat
cd runtime-library
.\build-runtime.bat
```

### 4. Build the conversion toolchain (requires Docker)

```bash
bash conversion-toolchain/build-toolchain.sh
```

Output: `conversion-toolchain/artifacts/oaax-cpu-toolchain.tar`

## Pre-built OAAX artifacts

If you're interested in using the OAAX toolchain and runtime without building them, you can find them in the
[contributions](https://github.com/oaax-standard/contributions) repository.   
Additionally, you can find a diverse set of examples and applications of using the OAAX runtime in the 
[examples](https://github.com/oaax-standard/examples) repository.

## Testing

After building both components, run the integration test suite:

```bash
# Set up the Python environment (once)
git submodule update --init --recursive
uv sync --extra integration
uv pip install -e conversion-toolchain/

# Python tests
uv run python tests/stage1.py          # conversion + YOLO model matrix
uv run python tests/stage2.py --csv results.csv  # runtime benchmarks

# C++ tests
bash runtime-library/build-runtimes.sh X86_64
bash tests/runtime/build-tests.sh
```

See [CONTRIBUTING.md](CONTRIBUTING.md) for environment setup details and known issues.

## Contributing

If you're interested in contributing to the OAAX reference implementation, please check out the [CONTRIBUTING.md](CONTRIBUTING.md) file for more information on how to get started.

## License

This project is licensed under the Apache License 2.0. See the [LICENSE](LICENSE) file for more details.
