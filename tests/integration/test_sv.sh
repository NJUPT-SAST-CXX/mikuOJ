#!/usr/bin/env bash
set -euo pipefail
BIN="${CPPJUDGE:-./build/cppjudge}"
SB="${CPPJUDGE_SANDBOX:-auto}"
P="--problem problems/A+B --sandbox-type $SB"
OUT=$("$BIN" judge $P --submission tests/security/cases/bad_syscall.cpp  2>/dev/null || true)
echo "$OUT" | grep -q 'Syscall Violation' || { echo "FAIL [sv]: expected Syscall Violation, got: $OUT"; exit 1; }
echo "PASS [sv]"
