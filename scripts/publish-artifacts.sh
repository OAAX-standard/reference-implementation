#!/usr/bin/env bash
# Upload the CPU runtimes and the conversion toolchain of one release to the OAAX bucket.
#
#   scripts/publish-artifacts.sh <version> <artifacts-dir>        # CI: on a v* tag, S3_* secrets in the environment
#   DRY_RUN=1 scripts/publish-artifacts.sh <version> <artifacts-dir>   # only check the files and print the keys
#
# <artifacts-dir> holds the CI artifacts, one directory per artifact name (what actions/download-artifact creates).
# Keys under <version>/ are written once: if any of them exists nothing is uploaded, because a published
# version is immutable (oaax-manager pins its sha256). latest/ is the pointer to the newest release.
set -euo pipefail

version=${1:?usage: publish-artifacts.sh <version> <artifacts-dir>}
dir=${2:?usage: publish-artifacts.sh <version> <artifacts-dir>}

# local file -> key suffix after "<runtimes|conversion-toolchain>/<version>/"
declare -A files=(
  ["$dir/runtime-linux-x86_64/runtime-library-X86_64.tar.gz"]="runtimes/@/CPU/x86_64/Ubuntu/library.tar.gz"
  ["$dir/runtime-linux-aarch64/runtime-library-AARCH64.tar.gz"]="runtimes/@/CPU/aarch64/Ubuntu/library.tar.gz"
  ["$dir/runtime-windows-x86_64/runtime-library-X86_64-Windows.tar.gz"]="runtimes/@/CPU/x86_64/Windows/library.tar.gz"
  ["$dir/toolchain-image/oaax-cpu-toolchain.tar"]="conversion-toolchain/@/CPU/oaax-cpu-toolchain.tar"
)

for f in "${!files[@]}"; do
  [ -s "$f" ] || { echo "missing or empty artifact: $f" >&2; exit 1; }
done

if [ "${DRY_RUN:-}" = 1 ]; then
  for f in "${!files[@]}"; do
    echo "would upload $f -> s3://oaax/${files[$f]//@/$version} (and .../latest/)"
  done
  exit 0
fi

s3() {
  s3cmd --access_key="$S3_ACCESS_KEY" --secret_key="$S3_SECRET_KEY" \
    --host="$S3_ENDPOINT_URL" --host-bucket="%(bucket)s.$S3_ENDPOINT_URL" --bucket-location=us-east-1 --ssl "$@"
}

# s3cmd info: 0 = exists, 12 = not found, anything else = a real error (credentials, network)
for f in "${!files[@]}"; do
  url="s3://oaax/${files[$f]//@/$version}"
  rc=0
  s3 info "$url" >/dev/null 2>&1 || rc=$?
  case $rc in
    0) echo "$url already exists: versions are immutable, bump VERSION for a new release" >&2; exit 1 ;;
    12) ;;
    *) echo "could not check $url (s3cmd exit $rc)" >&2; exit 1 ;;
  esac
done

for f in "${!files[@]}"; do
  s3 put "$f" "s3://oaax/${files[$f]//@/$version}"
  s3 put "$f" "s3://oaax/${files[$f]//@/latest}"
done
