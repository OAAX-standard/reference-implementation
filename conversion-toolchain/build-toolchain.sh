set -e

cd "$(dirname "$0")" || exit 1

rm -rf build 2&> /dev/null || true
mkdir build

# Build the toolchain as a Docker image
docker build -t conversion-toolchain:latest .

# Save the Docker image as a tarball
docker save conversion-toolchain:latest -o ./build/conversion-toolchain-latest.tar

# You can run the conversion toolchain using the following command:
# docker run -v /path/to/onnx-dir:/onnx conversion-toolchain:latest /onnx/model.onnx /onnx
