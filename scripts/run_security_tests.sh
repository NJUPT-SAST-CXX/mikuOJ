#!/usr/bin/env bash
# 安全测试（需 root + Linux 安全后端）。
set -euo pipefail
cd "$(dirname "$0")/.."
CPPJUDGE="${CPPJUDGE:-./build/cppjudge}" ./tests/security/run_security_tests.sh
