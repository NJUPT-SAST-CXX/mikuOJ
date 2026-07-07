#!/usr/bin/env bash
set -euo pipefail
BIN="${CPPJUDGE:-./build/cppjudge}"
SB="${CPPJUDGE_SANDBOX:-auto}"
P="--problem problems/A+B --sandbox-type $SB"
OUT=$("$BIN" judge $P --submission submissions/cpp/solution.cpp  2>/dev/null || true)
echo "$OUT" | grep -q '"final_verdict":"Accepted"' || { echo "FAIL [ac]: expected "final_verdict":"Accepted", got: $OUT"; exit 1; }
echo "PASS [ac]"
