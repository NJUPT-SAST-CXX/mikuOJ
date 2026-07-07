#!/usr/bin/env bash
set -euo pipefail
BIN="${CPPJUDGE:-./build/cppjudge}"
SB="${CPPJUDGE_SANDBOX:-auto}"
P="--problem problems/A+B --sandbox-type $SB"
OUT=$("$BIN" judge $P --submission submissions/cpp/return_1.cpp  2>/dev/null || true)
echo "$OUT" | grep -q 'Runtime Error' || { echo "FAIL [re]: expected Runtime Error, got: $OUT"; exit 1; }
echo "PASS [re]"
