#!/bin/bash
set -e

cd "$(dirname "$0")"

TOOLCHAINS_DIR=$(realpath ../../toolchains)
toolchain=$TOOLCHAINS_DIR/mingw-w64-x86_64-posix.cmake

if [ ! -f $toolchain ]; then
  echo "Toolchain file not found: $toolchain"
  exit 1
fi

export PATH=/usr/bin:/usr/local/bin:/snap/bin

cd source || exit 1

rm -rf build
mkdir build
cd build

cmake .. -DCMAKE_TOOLCHAIN_FILE=$toolchain \
  -DCPUINFO_TARGET_PROCESSOR=x86 \
  -DCPUINFO_BUILD_BENCHMARKS=OFF \
  -DCPUINFO_RUNTIME_TYPE=static

make -j

cd ../..

rm -rf X86_64_WINDOWS
mkdir X86_64_WINDOWS
rm -rf include
mkdir include

cp ./source/build/*.a ./X86_64_WINDOWS
cp ./source/include/*.h ./include
