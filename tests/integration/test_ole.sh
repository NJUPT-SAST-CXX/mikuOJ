#!/usr/bin/env bash
set -euo pipefail
BIN="${CPPJUDGE:-./build/cppjudge}"
SB="${CPPJUDGE_SANDBOX:-auto}"
P="--problem problems/A+B --sandbox-type $SB"
OUT=$("$BIN" judge $P --submission submissions/cpp/infinite_output.cpp --output-limit-mb 1 2>/dev/null || true)
echo "$OUT" | grep -q 'Output Limit Exceeded' || { echo "FAIL [ole]: expected Output Limit Exceeded, got: $OUT"; exit 1; }
echo "PASS [ole]"
