BASE_DIR="$(
  cd "$(dirname "$0")"
  pwd
)"
cd $BASE_DIR
cd source || exit 1

export PATH=/snap/bin:/opt/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu/bin:/usr/bin:/usr/local/bin
export CC=/opt/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-gcc
export CXX=/opt/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-g++

rm -rf build
mkdir build
cd build

cmake .. -DCMAKE_SYSTEM_PROCESSOR=arm64 \
-DCPUINFO_TARGET_PROCESSOR=arm64 \
-DCPUINFO_BUILD_BENCHMARKS=OFF \
-DCPUINFO_RUNTIME_TYPE=static \
-DCMAKE_SYSTEM_NAME=Linux \
-DCMAKE_SYSROOT=/opt/aarch64-linux-musl-cross \
-DCMAKE_C_COMPILER=/opt/aarch64-linux-musl-cross/bin/aarch64-linux-musl-gcc \
-DCMAKE_CXX_COMPILER=/opt/aarch64-linux-musl-cross/bin/aarch64-linux-musl-g++ \
-DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
-DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
-DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
-DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ONLY \
-DCMAKE_POSITION_INDEPENDENT_CODE=ON

make -j

cd ../..

rm -rf AARCH64
mkdir AARCH64
rm -rf include
mkdir include

cp ./source/build/*.a ./AARCH64
cp ./source/include/*.h ./include
