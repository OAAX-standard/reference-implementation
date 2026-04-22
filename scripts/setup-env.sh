#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# Change the working directory to the directory of the script
cd "$(dirname "$0")"

# Install ubuntu packages
echo ">>> Installing system packages..."
export DEBIAN_FRONTEND=noninteractive
apt-get install -y wget curl build-essential

# Define an array of URLs for downloading toolchains
toolchain_urls=(
    "https://oaax.nbg1.your-objectstorage.com/toolchains/x86_64-unknown-linux-gnu-gcc-9.5.0.tar.gz"           # nosec: public toolchain storage
    "https://oaax.nbg1.your-objectstorage.com/toolchains/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu.tar.gz" # nosec: public toolchain storage
)

# Function to extract the filename from a given URL
function get_filename_from_url() {
    local url=$1
    echo "${url##*/}" # Extract the part of the URL after the last '/'
}

# Iterate over all toolchain URLs to download and extract them
for url in "${toolchain_urls[@]}"; do
    filename=$(get_filename_from_url "$url") # Get the filename from the URL
    echo wget -nv -c "$url"                 # Print the wget command for logging
    wget -nv -c "$url"                      # Download the file with minimal output and resume capability
    # Extract the downloaded file to /opt, trying both gzip and non-gzip formats
    tar xzf "$filename" -C /opt 2>/dev/null || tar xf "$filename" -C /opt
    # Remove the downloaded file after extraction
    rm -rf "$filename" || true
    # Log the successful extraction of the file
    echo ">>>>>>>>>>> extracted: $filename"
done

# Install cmake
echo ">>> Installing CMake 3.31.7..."
host_platform=$(uname -m)
wget "https://cmake.org/files/v3.31/cmake-3.31.7-linux-${host_platform}.sh" \
    -q -O /tmp/cmake-install.sh &&
    chmod u+x /tmp/cmake-install.sh &&
    mkdir -p /opt/cmake-3.31.7 &&
    /tmp/cmake-install.sh --skip-license --prefix=/opt/cmake-3.31.7 &&
    rm /tmp/cmake-install.sh &&
    ln -fs /opt/cmake-3.31.7/bin/* /usr/local/bin

echo ""
echo ">>> Environment setup complete."
echo "    Toolchains installed to: /opt/"
echo "    CMake installed to:      /opt/cmake-3.31.7 (symlinked to /usr/local/bin)"
