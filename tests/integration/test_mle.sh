#!/usr/bin/env bash
set -euo pipefail
BIN="${CPPJUDGE:-./build/cppjudge}"
SB="${CPPJUDGE_SANDBOX:-auto}"
P="--problem problems/A+B --sandbox-type $SB"
OUT=$("$BIN" judge $P --submission submissions/cpp/memory_hog.cpp --memory-limit-mb 64 2>/dev/null || true)
echo "$OUT" | grep -q 'Memory Limit Exceeded' || { echo "FAIL [mle]: expected Memory Limit Exceeded, got: $OUT"; exit 1; }
echo "PASS [mle]"
