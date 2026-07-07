#!/usr/bin/env bash
set -euo pipefail
BIN="${CPPJUDGE:-./build/cppjudge}"
SB="${CPPJUDGE_SANDBOX:-auto}"
P="--problem problems/A+B --sandbox-type $SB"
OUT=$("$BIN" judge $P --submission submissions/cpp/wrong_answer.cpp  2>/dev/null || true)
echo "$OUT" | grep -q 'Wrong Answer' || { echo "FAIL [wa]: expected Wrong Answer, got: $OUT"; exit 1; }
echo "PASS [wa]"
