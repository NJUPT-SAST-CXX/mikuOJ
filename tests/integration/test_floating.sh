#!/usr/bin/env bash
# 浮点比较：临时题目 + 输出 pi 的程序，容差内应 AC
set -euo pipefail
BIN="${CPPJUDGE:-./build/cppjudge}"
SB="${CPPJUDGE_SANDBOX:-auto}"
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
mkdir -p "$TMP/input" "$TMP/output"
cat > "$TMP/problem.json" <<'J'
{"title":"Float","time_limit_ms":2000,"memory_limit_mb":128,"compare_mode":"floating","float_abs_eps":1e-6,"float_rel_eps":1e-6}
J
echo "0" > "$TMP/input/1.in"
echo "3.14159265" > "$TMP/output/1.out"
cat > "$TMP/sub.cpp" <<'C'
#include <cstdio>
int main() { printf("3.141592653589793\n"); return 0; }
C
OUT=$("$BIN" judge --problem "$TMP" --sandbox-type "$SB" --submission "$TMP/sub.cpp" 2>/dev/null || true)
echo "$OUT" | grep -q 'Accepted' || { echo "FAIL [floating]: $OUT"; exit 1; }
echo "PASS [floating]"
