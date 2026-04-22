# Version 2.0.0
- Introduced OAAX v2 API (`oaax_runtime.h`): replaces the v1 9-function API with a richer interface supporting multi-model loading, per-model config, structured status codes, and a `runtime_get_info()` diagnostic call.
- Removed deprecated v1 header (`runtime_core.hpp`).

# Version 1.1.3 (2025-10-14)
- Adds support for Ubuntu 25, by removing the executable flag from the libraries ELF.
- Includes `msvcp140_1.dll` in the Windows artifacts to fix runtime errors on some systems.

# Version 1.0.0 (2025-04-16)
- Initial release of the OAAX reference implementation.
- Includes the OAAX runtime and conversion toolchain.
- There are two runtimes: One for x86_64 and one for aarch64.
- The conversion toolchain is expected to run on x86_64.
