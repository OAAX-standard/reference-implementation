export PATH=/snap/bin:/opt/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu/bin:/usr/bin:/usr/local/bin
export CC=/opt/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-gcc
export CXX=/opt/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-g++

cd source
rm -rf build
mkdir build
cd build

cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON

make -j
sudo make

cd ../..
rm -rf AARCH64 include 
mkdir AARCH64
mkdir include

cp ./source/build/libre2.a ./AARCH64
cp -rf ./source/re2 ./include
