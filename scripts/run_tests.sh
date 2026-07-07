#!/usr/bin/env bash
# 一键回归：配置 + 构建 + 单元测试 + 集成测试。
set -euo pipefail
cd "$(dirname "$0")/.."

JOBS="$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j"$JOBS"

echo "=== unit tests ==="
ctest --test-dir build -L unit --output-on-failure

echo "=== integration tests ==="
ctest --test-dir build -L integration --output-on-failure
