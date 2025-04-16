# OAAX Runtime Library

This reference implementation of an OAAX runtime is a simple example of how an OAAX runtime should be implemented.

For the sake of example, we'll provide a simple implementation of an OAAX runtime that loads an optimized model and runs it on CPU using the [ONNX Runtime](https://github.com/microsoft/onnxruntime) library.

## Pre-requisites

Before you start, make sure you have set up your environment using the following script:

```bash
bash scripts/setup-env.sh
```

This will install the required dependencies and set up the environment for building the OAAX runtime.
The script will also set up the cross-compilation toolchain for the target architecture (X86_64 or AARCH64).

## Getting started

The OAAX runtime is leveraging the ONNX Runtime library to load and run the model. ORT requires
the [CPU INFOrmation library](https://github.com/pytorch/cpuinfo), and the [RE2 library](https://github.com/google/re2).

All of these dependencies are included in the `deps` directory, and are already cross-compiled for the target
architecture using the above-mentioned toolchain.
However, you can recompile them separately by running the Shell scripts inside each directory.

To build the OAAX runtime, run the following command:

```bash
bash build-runtimes.sh <X86_64|AARCH64>
```

This will create a `artifacts/` directory containing the compiled shared library: `libRuntimeLibrary.so`.

## Running Inference using the OAAX runtime

To run the inference process, you need to have the optimized model file, the shared library built in the previous step, along with a simple C++ code that loads the shared library and the model and runs it.

You can find diverse examples and applications of using the OAAX runtime in the
[examples](https://github.com/oaax-standard/examples) repository.
