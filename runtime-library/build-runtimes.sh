set -e

cd "$(dirname "$0")"

BUILD_DIR="$(pwd)/build"
ARTIFACTS_DIR="$(pwd)/artifacts"
ROOT_DIR="$(pwd)/.."
mkdir -p $BUILD_DIR
rm -rf $ARTIFACTS_DIR
mkdir -p $ARTIFACTS_DIR

VERSION_FILE="$ROOT_DIR/VERSION"
RUNTIME_VERSION="$(cat $VERSION_FILE)"

echo "Building for runtime version: $RUNTIME_VERSION"

# read platform from command line argument
if [ "$#" -ne 1 ]; then
  PLATFORMS=("X86_64" "AARCH64")
  echo "No platform specified. Building for both X86_64 and AARCH64."
  echo "You can specify a platform by running: ./build-runtime.sh <X86_64|AARCH64>"
  sleep 1
else
  if [ "$1" != "X86_64" ] && [ "$1" != "AARCH64" ]; then
    echo "Invalid platform specified. Use X86_64 or AARCH64."
    exit 1
  fi
  echo "Platform specified: $1"
  PLATFORMS=("$1")
fi

cd ${BUILD_DIR}

for platform in "${PLATFORMS[@]}"; do
  echo ">>>> Building for platform: $platform"
  # Clean up old build files
  rm -rf *
  # Generate the build files
  cmake .. -DPLATFORM="$platform" -DCMAKE_BUILD_TYPE=Release -DRUNTIME_VERSION="$RUNTIME_VERSION"
  # Make the build
  make -j
  echo "Build complete. The following shared libraries were created:"
  ls ./*.so
  # Copy the shared libraries to the artifacts directory
  echo "Copying shared libraries to artifacts directory..."
  mkdir -p "${ARTIFACTS_DIR}/$platform/"
  cp ./*.so "${ARTIFACTS_DIR}/$platform/"
  # Bundle the shared libraries into a tarball
  tar czf "${ARTIFACTS_DIR}/runtime-library-${platform}.tar.gz" -C "${ARTIFACTS_DIR}/$platform" ./*.so
  echo "Shared libraries for $platform have been copied to ${ARTIFACTS_DIR}/$platform/"
done
