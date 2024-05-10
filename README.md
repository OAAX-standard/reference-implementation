# Reference Implementation

This repository contains the source code for building the reference implementation of the OAX runtime and conversion
toolchain.

> If you're new to OAX, you can find more information about the OAX standard in
> the [OAX repository](https://github.com/oax-standard/OAX)

It builds on top of the illustrative implementation provided in
the [OAX standard repository](https://github.com/oax-standard/OAX/Illustrative%20example). Please check it out first to
get a high-level understanding on how the OAX runtime and conversion toolchain should be implemented.

## Repository structure

The repository is structured as follows:

- [Toolchain](conversion-toolchain): Contains the source code for building the OAX conversion toolchain.
- [Runtime](runtime-library): Contains the source code for building the OAX runtime.

Each folder contains a README file that provides more details about the different parts of the implementation.

## Building the reference implementation

You can build the conversion toolchain and the runtime separately by calling the (Shell) build scripts in each folder.
That will create a `build/` directory in each folder containing the compiled binaries: a compressed Docker image and a
shared library, respectively.

## Pre-built OAX artifacts

If you're interested in using the OAX toolchain and runtime without building them, you can find them in the
[contributions](https://github.com/oax-standard/contributions) repository.   
Additionally, you can find a diverse set of examples and applications of using the OAX runtime in the 
[examples](https://github.com/oax-standard/examples) repository.