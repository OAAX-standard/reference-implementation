BASE_DIR="$(
  cd "$(dirname "$0")"
  pwd
)"
cd $BASE_DIR
cd source || exit 1

export PATH=/usr/bin:/usr/local/bin:/snap/bin
export CC=x86_64-w64-mingw32-gcc
export CXX=x86_64-w64-mingw32-g++

# Ensure CC and CXX are available
if ! command -v $CC &>/dev/null; then
  echo "$CC could not be found"
  exit
fi

if ! command -v $CXX &>/dev/null; then
  echo "$CXX could not be found"
  exit
fi

rm -rf build
mkdir build
cd build

cmake .. \
  -DCMAKE_SYSTEM_PROCESSOR=x86_64 \
  -DCPUINFO_TARGET_PROCESSOR=x86 \
  -DCPUINFO_BUILD_BENCHMARKS=OFF \
  -DCPUINFO_RUNTIME_TYPE=static \
  -DCMAKE_SYSTEM_NAME=Windows \
  -DCMAKE_C_COMPILER=$CC \
  -DCMAKE_CXX_COMPILER=$CXX \
  -DCMAKE_RC_COMPILER=x86_64-w64-mingw32-windres \
  -DCMAKE_FIND_ROOT_PATH=/usr/x86_64-w64-mingw32 \
  -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
  -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
  -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
  -DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ONLY \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON

make -j

cd ../..

rm -rf X86_64_WINDOWS
mkdir X86_64_WINDOWS
rm -rf include
mkdir include

cp ./source/build/*.a ./X86_64_WINDOWS
cp ./source/include/*.h ./include
