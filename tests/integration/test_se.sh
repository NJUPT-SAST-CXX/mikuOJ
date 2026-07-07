#!/usr/bin/env bash
set -uo pipefail
BIN="${CPPJUDGE:-./build/cppjudge}"
SB="${CPPJUDGE_SANDBOX:-auto}"
OUT=$("$BIN" judge --sandbox-type "$SB" --problem problems/NOPE --submission submissions/cpp/solution.cpp 2>/dev/null || true)
echo "$OUT" | grep -q "System Error" || { echo "FAIL [se]: expected System Error, got: $OUT"; exit 1; }
echo "PASS [se]"
