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

cd source
rm -rf build
mkdir build
cd build

cmake .. -DCMAKE_TOOLCHAIN_FILE=$toolchain \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF

make -j
sudo make

cd ../..
rm -rf X86_64_WINDOWS include
mkdir X86_64_WINDOWS
mkdir include

cp ./source/build/libre2.a ./X86_64_WINDOWS
cp -rf ./source/re2 ./include
