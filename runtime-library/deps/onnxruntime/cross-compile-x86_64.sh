#!/bin/bash

set -e

BASE_DIR="$(cd "$(dirname "$0")"; pwd)";
cd $BASE_DIR

ort_version="1.17.3"
ort_folder_name="onnxruntime-${ort_version}"
cd $ort_folder_name

rm -rf build

export PATH=/snap/bin:/opt/x86_64-unknown-linux-gnu-gcc-9.5.0/bin:/usr/bin:/usr/local/bin
export CPATH="$BASE_DIR/../nlohmann/single_include"
export CC=/opt/x86_64-unknown-linux-gnu-gcc-9.5.0/bin/x86_64-unknown-linux-gnu-gcc
export CXX=/opt/x86_64-unknown-linux-gnu-gcc-9.5.0/bin/x86_64-unknown-linux-gnu-g++

# --x86, --arm, --arm64, --use_mimalloc, --use_dnnl, --dnnl_gpu_runtime, --use_full_protobuf, --use_acl, --use_armnn
# --minimal_build, --disable_ml_ops, --disable_contrib_ops
./build.sh --config Release --parallel --x86 --allow_running_as_root --cmake_extra_defines \
  CMAKE_POSITION_INDEPENDENT_CODE=ON \
  onnxruntime_USE_NSYNC=OFF  \
  onnxruntime_CROSS_COMPILING=ON \
  onnxruntime_BUILD_UNIT_TESTS=OFF \
  onnxruntime_ENABLE_CPUINFO=OFF

cd ..

rm -rf X86_64 || true
mkdir X86_64  || true

cp -rf ./$ort_folder_name/include ./X86_64  || true
RELEASE="./$ort_folder_name/build/Linux/Release"  || true
cp $RELEASE/*.a ./X86_64 2>>/dev/null  || true
cp $RELEASE/*.so ./X86_64 2>>/dev/null  || true

shopt -s globstar  || true
cp -rf $RELEASE/_deps/*-build/**/*.a ./X86_64 2>>/dev/null  || true
cp -rf $RELEASE/_deps/*-build/**/*.so ./X86_64 2>>/dev/null  || true

printf "\nDone :) \n"
