# OAX Runtime Library

This reference implementation of an OAX runtime is a simple example of how an OAX runtime should be implemented.

For the sake of example, we'll provide a simple implementation of an OAX runtime that loads an optimized model and
runs it on CPU using the [ONNX Runtime](https://github.com/microsoft/onnxruntime) library.

## Pre-requisites

This source code should be built on an Ubuntu 22.04 LTS machine with a X86_64 architecture
using [this](https://download.sclbl.net/toolchains/x86_64-unknown-linux-gnu-gcc-9.5.0.tar.gz) cross-compilation
toolchain. The toolchain should be extracted to the `/opt` directory.

## Getting started

The OAX runtime is leveraging the ONNX Runtime library to load and run the model. ORT requires
the [CPU INFOrmation library](https://github.com/pytorch/cpuinfo), and the [RE2 library](https://github.com/google/re2).

All of these dependencies are included in the `deps` directory, and are already cross-compiled for the target
architecture using the above-mentioned toolchain.
However, you can recompile them separately by running the Shell scripts inside each directory.

To build the OAX runtime, run the following command:

```bash
bash build-runtime.sh
```

This will create a `build/` directory containing the compiled shared library: `libRuntimeLibrary.so`.

## Running Inference using the OAX runtime

To run the inference process, you need to have the optimized model file, the shared library built in the previous step,
along with a simple C++ code that loads the shared library and the model and runs it.

You can find diverse examples and applications of using the OAX runtime in the
[examples](https://github.com/oax-standard/examples) repository.
