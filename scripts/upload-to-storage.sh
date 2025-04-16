#!/bin/bash

set -e # Exit immediately if a command exits with a non-zero status.

# Ensure s3cmd is installed
if ! command -v s3cmd &>/dev/null; then
    echo "Error: s3cmd is not installed. Please install it and try again."
    exit 1
fi

# Ensure at least two arguments are provided: version and at least one file
if [ "$#" -lt 2 ]; then
    echo "Usage: $0 <version> <file1> [<file2> ... <fileN>]"
    exit 1
fi

# Extract the version argument
version=$1
shift # Shift arguments to access the file list

# Define the object key path for the storage
OBJECT_KEY="oaax/runtimes/$version/CPU/"

# Check if s3cmd is configured properly
if ! s3cmd --help &>/dev/null; then
    echo "Error: s3cmd is not configured. Please configure it and try again."
    exit 1
fi

# Upload files to the object storage using s3cmd
echo "Uploading files to s3://$OBJECT_KEY..."
s3cmd put "$@" "s3://$OBJECT_KEY"

echo "Upload completed successfully."
