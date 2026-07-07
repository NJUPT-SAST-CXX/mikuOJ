#pragma once

namespace cppjudge {

class Doctor {
public:
    // 环境诊断。返回 true 表示所有"硬"检查通过（可判题）。
    // Linux：检查 root / cgroup v2 / 内核 / libseccomp（硬）+ WSL2（建议）。
    // macOS：报告工具链并提示"开发模式、无隔离"，硬检查恒通过。
    static bool check();
};

} // namespace cppjudge
