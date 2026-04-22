---
name: build-toolchain
description: Knows how to build the OAAX conversion toolchain Docker image
when_to_use: When building, verifying, or troubleshooting the Python conversion toolchain
user-invocable: false
---

## Building the Conversion Toolchain

```bash
bash conversion-toolchain/build-toolchain.sh
```

Output: `conversion-toolchain/artifacts/oaax-cpu-toolchain.tar`

## Prerequisites

- Docker must be running (`docker --version` to verify)
- The script builds a Python 3.8.16 Debian-based image

## What the Build Does

1. Builds Docker image from `conversion-toolchain/Dockerfile`
2. Installs Python dependencies from `requirements.txt` (onnx, onnxruntime, onnxsim, lgg)
3. Installs the `conversion_toolchain` package via `setup.py`
4. Exports image as a tarball

## Testing the Built Image

```bash
docker load < conversion-toolchain/artifacts/oaax-cpu-toolchain.tar
docker run --rm oaax-cpu-toolchain --help
```

## Python Package Structure

- Entry point: `conversion_toolchain/main.py` (CLI)
- Core logic: `conversion_toolchain/utils.py` (onnxsim wrapper, MD5)
- Logging: `conversion_toolchain/logger.py` (JSON step logger)
- Container entrypoint: `conversion-toolchain/scripts/convert.sh`

## Troubleshooting

- If Docker build fails on pip install, check `requirements.txt` for version conflicts
- The image must work offline after build — no runtime internet access
