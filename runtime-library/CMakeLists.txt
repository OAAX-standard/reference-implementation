cmake_minimum_required(VERSION 3.10.2)

project(RuntimeLibrary)

# Enable verbose makefile output (set to OFF for less output)
set(CMAKE_VERBOSE_MAKEFILE OFF)

# === Directory Structure ===
# Define key source, include, and dependency directories
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(DEPS_DIR ${CMAKE_CURRENT_LIST_DIR}/deps)
set(TOOLS_C_UTILITIES_DIR ${DEPS_DIR}/tools/c-utilities)
set(TOOLS_C_UTILITIES_INCLUDE_DIR ${DEPS_DIR}/tools/c-utilities/include)
set(RE2_DIR "${DEPS_DIR}/re2")
set(ONNX_RUNTIME_DIR "${DEPS_DIR}/onnxruntime")
set(CPUINFO_DIR "${DEPS_DIR}/cpuinfo")


# === Platform Selection and Toolchain Setup ===
# Select platform and set cross-compilation toolchain variables
if(NOT DEFINED PLATFORM)
  message(FATAL_ERROR "Please specify the target platform: -DPLATFORM=X86_64 or -DPLATFORM=AARCH64")
endif()

# Force the compiler to be set to the one specified in the toolchain file
# This is necessary for cross-compilation to work correctly
INCLUDE(CMakeForceCompiler)

if (PLATFORM STREQUAL "X86_64")
  set(GENERIC_BUILD_TARGET X86_64)
  set(CROSS_ROOT "/opt/x86_64-unknown-linux-gnu-gcc-9.5.0")
  set(COMPILER_PREFIX "x86_64-unknown-linux-gnu-")
  set(SYSROOT "/opt/x86_64-unknown-linux-gnu-gcc-9.5.0/x86_64-unknown-linux-gnu/sysroot")
  # -fno-math-errno: Avoid setting errno after math functions (improves performance).
  # -fopenmp: Enable OpenMP for parallel programming.
  # -std=c99: Use the C99 standard for C code.
  # -march=haswell: Optimize code for Intel Haswell CPUs.
  set(CMAKE_BUILD_FLAGS "-fno-math-errno -fopenmp -march=haswell")
elseif (PLATFORM STREQUAL "AARCH64")
  set(GENERIC_BUILD_TARGET AARCH64)
  # Uncomment below to use a different toolchain version
  # set(CROSS_ROOT "/opt/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu")
  set(CROSS_ROOT "/opt/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu")
  set(COMPILER_PREFIX "aarch64-none-linux-gnu-")
else()
  message(FATAL_ERROR "Please specify the target platform: X86_64 or AARCH64. Found ${PLATFORM}")
endif()

# Print toolchain and sysroot info for debugging
message(STATUS "SDKTARGETSYSROOT=${SDKTARGETSYSROOT}")
message(STATUS "CROSS_ROOT=${CROSS_ROOT}")

# Set the root path for finding libraries and headers on the target
set(CMAKE_FIND_ROOT_PATH ${SDKTARGETSYSROOT})

# Set CMake toolchain variables for cross-compilation
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_AR ${CROSS_ROOT}/bin/${COMPILER_PREFIX}ar)
set(CMAKE_AS ${CROSS_ROOT}/bin/${COMPILER_PREFIX}as)
set(CMAKE_LD ${CROSS_ROOT}/bin/${COMPILER_PREFIX}ld)
set(CMAKE_RANLIB ${CROSS_ROOT}/bin/${COMPILER_PREFIX}ranlib)
set(CMAKE_C_COMPILER ${CROSS_ROOT}/bin/${COMPILER_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${CROSS_ROOT}/bin/${COMPILER_PREFIX}g++)
set(CMAKE_ASM_COMPILER ${CROSS_ROOT}/bin/${COMPILER_PREFIX}gcc)
set(CMAKE_NM ${CROSS_ROOT}/bin/${COMPILER_PREFIX}NM)
# Do not use the root path when searching for programs (executables)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# Only search the root path when searching for libraries
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# Only search the root path when searching for header files
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
# Only search the root path when searching for packages (e.g., via find_package)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Reset default linker flags for shared libraries
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS " ")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS " ")

# Add common warning and optimization flags for C and C++
# -fno-common: Prevents multiple definitions of common symbols (reduces binary size).
# -Wall: Enables most compiler warning messages.
# -Wextra: Enables additional warning messages.
# -Wformat: Enables warnings for format string issues.
# -Wformat-security: Enables warnings for format string security issues.
# -Wno-missing-field-initializers: Suppresses warnings for missing field initializers.
# -Wno-attribute-warning: Suppresses warnings for attribute-related issues.
# -fdata-sections: Places each data item in its own section in the output file.
# -ffunction-sections: Places each function in its own section in the output file.
# -Wl,--gc-sections: Tells the linker to remove unused sections (dead code/data) from the final binary.
# -fno-exceptions: Disables exception handling (reduces binary size).
set(CMAKE_BUILD_FLAGS "${CMAKE_BUILD_FLAGS} -std=c99 -fno-common -Wall -Wextra -Wformat -Wformat-security")
set(CMAKE_BUILD_FLAGS "${CMAKE_BUILD_FLAGS} -Wno-missing-field-initializers -Wno-attribute-warning")
set(CMAKE_BUILD_FLAGS "${CMAKE_BUILD_FLAGS} -fdata-sections -ffunction-sections -Wl,--gc-sections -fno-exceptions")
# -Wall                # Enables most compiler warning messages
# -Wextra             # Enables additional warning messages
# -Wformat            # Enables warnings for format string issues
# -Wformat-security    # Enables warnings for format string security issues
# -fdata-sections     # Places each data item in its own section in the output file
# -ffunction-sections # Places each function in its own section in the output file
# -Wl,--gc-sections   # Tells the linker to remove unused sections (dead code/data) from the final binary
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wformat -Wformat-security -fdata-sections -ffunction-sections -Wl,--gc-sections")
# -D_GNU_SOURCE: Defines the _GNU_SOURCE macro for GNU-specific features.
# -I${CROSS_ROOT}/include: Adds the include directory for the cross-compilation toolchain.
set(CMAKE_C_FLAGS " ${CMAKE_C_FLAGS} ${CMAKE_BUILD_FLAGS}  -D_GNU_SOURCE -I${CROSS_ROOT}/include")
set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} ${CMAKE_BUILD_FLAGS}  -D_GNU_SOURCE -I${CROSS_ROOT}/include")
set(CMAKE_CPP_FLAGS " ${CMAKE_CPP_FLAGS} ${CMAKE_BUILD_FLAGS}  -D_GNU_SOURCE -I${CROSS_ROOT}/include")

# === Generic Build Settings ===
# Set basic C, C++, and linker flags
set(LINKER_FLAGS "${LINKER_FLAGS} ${CMAKE_BUILD_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_BUILD_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_BUILD_FLAGS}")

# Linker flag to error on undefined symbols when building shared libraries
# set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")

# === Source Files ===
# Add all source files for the runtime library
file(GLOB_RECURSE SRC
        ${SRC_DIR}/runtime_utils.c
        ${SRC_DIR}/runtime_core.c
)
######################## main bin build settings #######################################################################

# === Library Target ===
# Create the shared library target for the runtime
add_library(RuntimeLibrary SHARED ${SRC})

# Add optimization and debug flags for Release builds
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
  target_compile_options(RuntimeLibrary PUBLIC
    -Ofast # Enable aggressive optimizations for speed (may break strict standards compliance)
    -s     # Strip all symbol information from the output binary (reduces size)
    -g     # Generate debug information (minimal, for backtraces)
  )
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  target_compile_options(RuntimeLibrary PUBLIC
    -O0    # No optimization (easier debugging)
    -g3    # Generate maximum debug information
  )
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
  target_compile_options(RuntimeLibrary PUBLIC
    -O2    # Optimize for speed, but less aggressively than -Ofast
    -g     # Generate debug information
  )
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
  target_compile_options(RuntimeLibrary PUBLIC
    -Os    # Optimize for minimum binary size
    -g     # Generate debug information
  )
endif()

# === Include Directories ===
# Specify public include paths for consumers
target_include_directories(RuntimeLibrary PUBLIC
  ${INCLUDE_DIR}
  ${TOOLS_C_UTILITIES_INCLUDE_DIR}
  ${RE2_DIR}/include
  ${CPUINFO_DIR}/include
  ${ONNX_RUNTIME_DIR}/${GENERIC_BUILD_TARGET}/include
  ${ONNX_RUNTIME_DIR}/${GENERIC_BUILD_TARGET}/include/onnxruntime/core/session
)

# === Link Directories ===
# Specify library search paths for dependencies
target_link_directories(RuntimeLibrary PUBLIC
  "${RE2_DIR}/${GENERIC_BUILD_TARGET}"
  "${CPUINFO_DIR}/${GENERIC_BUILD_TARGET}"
  "${ONNX_RUNTIME_DIR}/${GENERIC_BUILD_TARGET}"
)

# === Link to c-utilities ===
# Add the c-utilities subdirectory for building and linking
add_subdirectory(${TOOLS_C_UTILITIES_DIR} ${CMAKE_CURRENT_BINARY_DIR}/c_utilities)

# === Link Libraries ===
# Link all required libraries and group them for the linker
# -Wl,--start-group and -Wl,--end-group ensure all symbols are resolved
# List all ONNX, absl, and other dependencies explicitly
# Link pthread, dl, c_utilities, and stdc++ as well
target_link_libraries(RuntimeLibrary PUBLIC
  -Wl,--start-group
  onnxruntime_session onnxruntime_optimizer onnxruntime_providers onnxruntime_util
  onnxruntime_framework onnxruntime_graph onnxruntime_mlas onnxruntime_common onnxruntime_flatbuffers
  absl_base absl_log_severity absl_malloc_internal absl_raw_logging_internal absl_spinlock_wait absl_throw_delegate
  absl_hashtablez_sampler absl_raw_hash_set absl_hash absl_city absl_low_level_hash absl_int128 absl_exponential_biased
  absl_cord absl_cord_internal absl_cordz_functions absl_cordz_handle absl_cordz_info absl_strings absl_strings_internal
  absl_graphcycles_internal absl_synchronization absl_civil_time absl_time absl_time_zone absl_bad_optional_access absl_bad_variant_access
  flatbuffers nsync_cpp onnx onnx_proto protobuf-lite
  cpuinfo re2 pthread dl c_utilities stdc++
  -Wl,--end-group
)