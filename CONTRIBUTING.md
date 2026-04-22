# Contributing to OAAX reference implementation

Thank you for your interest in contributing to OAAX's reference implementation! We welcome contributions from the community to improve the reference implementation of the OAAX runtime and conversion toolchain.

Before you start contributing, please take a moment to read through [OAAX general CONTRIBUTING document](https://github.com/OAAX-standard/OAAX/blob/main/CONTRIBUTING.md). It sets out the guidelines for contributing to OAAX projects, including code style, commit message format, and other important information.

## Table of contents
- [How to contribute](#how-to-contribute)
- [Development environment](#development-environment)

## How to contribute

The OAAX reference implementation is built to be straightforward to understand, build, and use. If you'd like to suggest improvements to the documentation or implementation, report a bug, or contribute a new feature, please follow these steps:
1. **Open an issue** in the repository to discuss your idea or improvement.
2. **Fork the repository** and create a new branch for your changes.
3. **Make your changes** and commit them with a clear message.
4. **Push your changes** to your forked repository.
5. **Submit a pull request** to the main repository for review.


## Development environment

The OAAX reference implementation has been tested for building on x86_64 architecture machines running Ubuntu 20.04 or later.

### Runtime

The runtime library is built using CMake. Install the required cross-compilation toolchains once per machine:

```bash
sudo bash scripts/setup-env.sh
```

The public API is defined in `runtime-library/include/oaax_runtime.h`. Never change existing function signatures; new configuration goes through the `Config` key-value struct.

### Conversion toolchain

The conversion toolchain is built using Docker and requires Docker to be installed on your machine. You can find instructions for installing Docker [here](https://docs.docker.com/get-docker/).

## Commit conventions

All commits must include a sign-off and co-author line:

```
Signed-off-by: Your Name <your@email.com>
```

Commit message format: `<type>: <short description>` where type is one of `feat`, `fix`, `refactor`, `docs`, `ci`, `chore`.
