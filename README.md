# Reference Implementation

[![Build OAAX conversion toolchain](https://github.com/OAAX-standard/reference-implementation/actions/workflows/build-toolchain.yml/badge.svg?branch=main)](https://github.com/OAAX-standard/reference-implementation/actions/workflows/build-toolchain.yml)
[![Build OAAX runtime](https://github.com/OAAX-standard/reference-implementation/actions/workflows/build-runtime.yml/badge.svg?branch=main)](https://github.com/OAAX-standard/reference-implementation/actions/workflows/build-runtime.yml)

This repository contains the source code for building the reference implementation of the OAAX runtime and conversion toolchain.

> If you're new to OAAX, you can find more information about the OAAX standard in
> the [OAAX repository](https://github.com/oaax-standard/OAAX)

It builds on top of the illustrative implementation provided in
the [OAAX standard repository](https://github.com/oaax-standard/OAAX/Illustrative%20example). Please check it out first to get a high-level understanding on how an OAAX runtime and conversion toolchain must be implemented.

## Repository structure

The repository is structured as follows:

- [Conversion toolchain](conversion-toolchain): Contains the source code for building the OAAX conversion toolchain.
- [Runtime library](runtime-library): Contains the source code for building the OAAX runtime.

Each folder contains a README file that provides more details about the different parts of the implementation.

## Building the reference implementation

You can build the conversion toolchain and the runtime separately by calling the (Shell) build scripts in each folder.
That will create a `artifacts/` directory in each folder containing the compiled binaries: a compressed Docker image and shared libraries (one for X86_64 and one for AARCH64), respectively.

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

See `.claude/CLAUDE.md` for the full testing guide including known environment setup issues.

## Contributing

If you're interested in contributing to the OAAX reference implementation, please check out the [CONTRIBUTING.md](CONTRIBUTING.md) file for more information on how to get started.

## License

This project is licensed under the Apache License 2.0. See the [LICENSE](LICENSE) file for more details.
