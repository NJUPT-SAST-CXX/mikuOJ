#!/usr/bin/env bash
set -euo pipefail
BIN="${CPPJUDGE:-./build/cppjudge}"
SB="${CPPJUDGE_SANDBOX:-auto}"
P="--problem problems/A+B --sandbox-type $SB"
OUT=$("$BIN" judge $P --submission submissions/cpp/endless_loop.cpp --time-limit-ms 500 2>/dev/null || true)
echo "$OUT" | grep -q 'Time Limit Exceeded' || { echo "FAIL [tle]: expected Time Limit Exceeded, got: $OUT"; exit 1; }
echo "PASS [tle]"
