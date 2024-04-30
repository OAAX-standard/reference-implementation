set -e

cd "$(dirname "$0")"

# check if this file exists: /opt/x86_64-unknown-linux-gnu-gcc-9.5.0/bin/x86_64-unknown-linux-gnu-gcc
if [ ! -f /opt/x86_64-unknown-linux-gnu-gcc-9.5.0/bin/x86_64-unknown-linux-gnu-gcc ]; then
  echo "Cross-compiler not found."
  echo "Please download it from 'https://download.sclbl.net/toolchains/x86_64-unknown-linux-gnu-gcc-9.5.0.tar.gz',"
  echo "and extract it to '/opt' using: 'sudo tar -xvf x86_64-unknown-linux-gnu-gcc-9.5.0.tar.gz -C /opt'"
  exit 1
fi

rm -rf build 2 &>/dev/null || true
mkdir -p build
cd build

cmake ..
make -j
echo "Build complete. The following shared libraries were created:"
ls ./*.so
