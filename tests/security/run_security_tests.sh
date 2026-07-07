#!/usr/bin/env bash
# 安全测试：验证沙箱隔离（需安全后端 linux-ns，需 root）。
set -uo pipefail
BIN="${CPPJUDGE:-./build/cppjudge}"
SB="${CPPJUDGE_SANDBOX:-linux-ns}"
PASS=0; FAIL=0

check() {
    local name="$1" sub="$2" expect="$3"
    OUT=$("$BIN" judge --problem problems/A+B --sandbox-type "$SB" --submission "$sub" 2>/dev/null || true)
    if echo "$OUT" | grep -q "$expect"; then
        echo "  [$name] PASS ($expect)"; PASS=$((PASS + 1))
    else
        echo "  [$name] FAIL: expected '$expect', got: $OUT"; FAIL=$((FAIL + 1))
    fi
}

echo "=== security tests (backend=$SB) ==="
check "network blocked"    tests/security/cases/bad_syscall.cpp       "Syscall Violation"
check "fork bomb blocked"  tests/security/cases/fork_bomb.cpp         "Syscall Violation"
check "malicious include"  tests/security/cases/malicious_include.cpp "Compile Error"
check "memory bomb -> MLE" tests/security/cases/infinite_memory.cpp   "Memory Limit Exceeded"
# read_passwd：/etc/passwd 不在沙箱内 → 输出为空 → 非 Accepted（无泄漏）
check "no /etc/passwd leak" tests/security/cases/read_passwd.cpp      "Wrong Answer"
echo "=== $PASS passed, $FAIL failed ==="
exit $FAIL
