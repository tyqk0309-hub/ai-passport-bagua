#!/usr/bin/env bash
# Run bagua + baseline host tests without the full validate gate (Windows-friendly).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
CC_BIN="${CC:-cc}"
TMP="${TMPDIR:-/tmp}/bagua-host-tests.$$"
mkdir -p "$TMP"
echo "using CC=$CC_BIN"
"$CC_BIN" -std=c11 -Wall -Wextra -Werror -Imain \
    tests/test_bagua_core.c main/bagua_core.c main/bagua_data.c \
    -o "$TMP/test_bagua_core"
"$TMP/test_bagua_core"
"$CC_BIN" -std=c11 -Wall -Wextra -Werror -Imain \
    tests/test_key_guard.c -o "$TMP/test_key_guard"
"$TMP/test_key_guard"
echo "Host tests: PASS"
