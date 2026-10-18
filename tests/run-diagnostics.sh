#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")/.."
test_directory=$(mktemp -d)
trap 'rm -rf "$test_directory"' EXIT
cc -std=gnu11 -Wall -Wextra -Werror -Isource \
  "-DDIAGNOSTICS_DIRECTORY=\"$test_directory/logs\"" \
  "-DDIAGNOSTICS_MARKER=\"$test_directory/enabled\"" \
  -DDIAGNOSTICS_LIMIT=256 \
  tests/diagnostics_test.c source/diagnostics.c -o "$test_directory/test"
"$test_directory/test"
