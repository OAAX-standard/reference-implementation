# Version 2.0.0 (2026-10-30)
- Introduced OAAX v2 API (`oaax_runtime.h`): replaces the v1 9-function API with a richer interface supporting multi-model loading, per-model config, structured status codes, and a `runtime_get_info()` diagnostic call.
- Removed deprecated v1 header (`runtime_core.hpp`).
- Fixed 19 security issues (2 critical, 6 high, 7 medium, 4 low): double free and leak in the worker loop, `exit()` inside the shared library, `--onnx-path` validation, the toolchain container running as root, unpinned CI actions and Python dependencies.
- Added a test framework: pytest suites (conversion, Docker end to end, YOLO), C++ runtime tests and an OAAX vs. ONNX Runtime benchmark.
- Test-time use of Ultralytics (AGPL-3.0, optional, not distributed) is now listed in `THIRD_PARTY_NOTICES.txt`.
- Release process: artifacts are uploaded to the bucket only when a `vX.Y.Z` tag is pushed, after all tests pass, and a published version is never overwritten.

# Version 1.1.3 (2025-10-14)
- Adds support for Ubuntu 25, by removing the executable flag from the libraries ELF.
- Includes `msvcp140_1.dll` in the Windows artifacts to fix runtime errors on some systems.

# Version 1.0.0 (2025-04-16)
- Initial release of the OAAX reference implementation.
- Includes the OAAX runtime and conversion toolchain.
- There are two runtimes: One for x86_64 and one for aarch64.
- The conversion toolchain is expected to run on x86_64.
