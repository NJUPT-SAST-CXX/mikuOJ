#pragma once

// Linux 专用：seccomp-bpf syscall 过滤。默认拒绝 + 白名单，SCMP_ACT_KILL_PROCESS。
// 违规子进程收到 SIGSYS，由 Sandbox 映射为 Verdict::SV（修 D4）。

#include "cppjudge/language.h"  // SeccompProfile

#include <string>
#include <vector>

namespace cppjudge::seccomp {

// 由语言名选运行时 profile（未知 → 最严格 Strict）。
SeccompProfile profile_for_lang(const std::string& lang);

class Manager {
public:
    // 安装过滤器。约束：必须是子进程 execve 前的绝对最后一步。
    //   is_compile=true → 使用宽松编译白名单（放行 fork/clone/wait4/pipe 等），
    //                     否则编译器子进程会被 SIGSYS 杀（修 D6）。
    // 所有白名单都禁止 socket/connect/ptrace 等；始终放行 execve/execveat。
    static bool install(SeccompProfile profile, bool is_compile);

    // syscall 号 → 名称（SV 诊断用）。
    static std::string violation_to_string(int syscall_num);

    // 测试钩子：返回运行时白名单的 syscall 名（arch 无关；不含 compile 扩展）。
    static const std::vector<std::string>& allowlist_for_testing(SeccompProfile p);
};

} // namespace cppjudge::seccomp
