#!/usr/bin/env bash
set -euo pipefail
BIN="${CPPJUDGE:-./build/cppjudge}"
SB="${CPPJUDGE_SANDBOX:-auto}"
P="--problem problems/A+B --sandbox-type $SB"
OUT=$("$BIN" judge $P --submission submissions/cpp/broken.cpp  2>/dev/null || true)
echo "$OUT" | grep -q 'Compile Error' || { echo "FAIL [ce]: expected Compile Error, got: $OUT"; exit 1; }
echo "PASS [ce]"
