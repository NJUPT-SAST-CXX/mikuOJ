#pragma once

// Linux 专用：Seccomp Manager。
//
// 该模块负责把语言对应的 SeccompProfile 转换为 syscall 白名单，并通过
// libseccomp 安装过滤器。策略采用默认拒绝 + 白名单模式，默认动作是
// SCMP_ACT_KILL_PROCESS。
//
// Sandbox Core 负责决定“在哪个阶段安装 seccomp”。当前正式运行路径中，
// 运行阶段会在子进程 execve 前安装 seccomp；编译阶段仍处于 namespace、
// cgroup、降权和最小文件系统约束下，但当前 Linux 后端没有启用编译期 seccomp。
// 如果后续要启用编译期 seccomp，可复用 install(profile, true) 的编译白名单能力。
//
// 违规子进程收到 SIGSYS，由 Sandbox Core 映射为 Verdict::SV。

#include "cppjudge/language.h"  // SeccompProfile

#include <string>
#include <vector>

namespace cppjudge::seccomp {

class Manager {
public:
    // 安装过滤器。约束：必须在子进程 execve 前、所有 setup syscall 之后调用。
    //
    // is_compile=false → 使用语言运行时白名单。
    // is_compile=true  → 使用编译白名单（预留给编译阶段启用 seccomp 时使用）。
    //
    // ptrace 等逃逸相关 syscall 始终不放行；execve/execveat 始终放行。
    // JVM profile 会放行 socket/connect 以兼容运行时，本项目依赖 CLONE_NEWNET
    // 提供空网络命名空间来阻断真实网络访问。
    static bool install(SeccompProfile profile, bool is_compile);
static SeccompProfile profile_for_lang(const std::string& lang);
    // syscall 号 → 名称（SV 诊断用）。
    static std::string violation_to_string(int syscall_num);

    // 测试钩子：返回运行时白名单的 syscall 名（arch 无关；不含 compile 扩展）。
    static const std::vector<std::string>& allowlist_for_testing(SeccompProfile p);
};

} // namespace cppjudge::seccomp
