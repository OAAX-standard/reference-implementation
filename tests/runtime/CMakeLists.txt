cmake_minimum_required(VERSION 3.10.2)
project(OAAXRuntimeTests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Path to the prebuilt runtime library (libRuntimeLibrary.so)
if(NOT DEFINED RUNTIME_LIB_DIR)
    set(RUNTIME_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../runtime-library/build")
endif()

# Path to the runtime public headers
if(NOT DEFINED RUNTIME_INCLUDE_DIR)
    set(RUNTIME_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../runtime-library/include")
endif()

message(STATUS "Runtime include dir: ${RUNTIME_INCLUDE_DIR}")
message(STATUS "Runtime lib dir:     ${RUNTIME_LIB_DIR}")

add_executable(simple_test simple_test.cpp)
target_include_directories(simple_test PRIVATE ${RUNTIME_INCLUDE_DIR})
target_link_directories(simple_test PRIVATE ${RUNTIME_LIB_DIR})
target_link_libraries(simple_test PRIVATE RuntimeLibrary)
if(NOT MSVC)
    target_link_options(simple_test PRIVATE "-Wl,--allow-shlib-undefined")
endif()
set_target_properties(simple_test PROPERTIES
    BUILD_RPATH "$ORIGIN"
    INSTALL_RPATH "$ORIGIN"
)
