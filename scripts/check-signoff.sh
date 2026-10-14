#!/bin/bash
# commit-msg hook: the DCO check on PRs requires a Signed-off-by trailer that
# exactly matches the commit author. Catch mismatches before they reach CI.
# See .claude/rules/git-signoff.md.
set -euo pipefail

msg_file=$1
name=${GIT_AUTHOR_NAME:-$(git config user.name)}
email=${GIT_AUTHOR_EMAIL:-$(git config user.email)}
expected="Signed-off-by: ${name} <${email}>"

if grep -qF "$expected" "$msg_file"; then
  exit 0
fi

echo "DCO: sign-off missing or doesn't match the commit author." >&2
echo "  expected: $expected" >&2
found=$(grep '^Signed-off-by:' "$msg_file" || true)
if [ -n "$found" ]; then
  echo "  found:    $found" >&2
fi
echo "Commit with 'git commit -s' using your own git identity." >&2
exit 1
