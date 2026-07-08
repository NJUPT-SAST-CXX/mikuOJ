#pragma once

// Sandbox Core 内部辅助接口（不对 CLI / Judge / Compiler 暴露）。
//
// 这里放置平台后端共用的“小而稳定”的编排工具：
//   - 原始运行观测量 RawOutcome；
//   - 从 RawOutcome + Limits 推导 Verdict；
//   - 构造最小安全环境；
//   - 构造 execve 所需 argv/envp 指针数组；
//   - 统计输出文件大小。
//
// 真正的 namespace、mount、cgroup、seccomp、exec 和 wait 编排仍由
// src/sandbox_linux.cpp 这样的具体后端实现。

#include "cppjudge/common.h"
#include "cppjudge/sandbox.h"

#include <cstdint>
#include <string>
#include <vector>

namespace cppjudge::sandbox_detail {

// 子进程结束后的原始观测量。
//
// 平台后端负责填充这些字段；derive_verdict() 再根据统一优先级转换成
// 对外的 Verdict。这样可以避免 Judge 层直接理解 wait status、signal 或
// cgroup 细节。
struct RawOutcome {
    bool     exited         = false;
    int      exit_code      = 0;
    bool     signaled       = false;
    int      signal_num     = 0;
    bool     wall_timed_out = false;   // 父进程因墙上超时主动 kill
    bool     cpu_timed_out  = false;   // 后端判定 CPU 超时
    bool     output_exceeded = false;  // 父进程因输出超限主动 kill
    bool     oom_killed     = false;   // cgroup / 观测判定 OOM
    uint64_t cpu_time_ms    = 0;       // 用户态 CPU 时间
    uint64_t wall_time_ms   = 0;
    uint64_t memory_kb      = 0;       // 峰值 RSS
    uint64_t output_bytes   = 0;       // stdout 实际字节（0 = 未捕获）
    bool     secure_backend = true;    // SIGSYS → SV 仅安全后端成立
};

// 由原始观测量 + 资源限制推导判决，并给出 output_truncated。
// 判决优先级：TLE > MLE > SV > OLE > RE > AC（AC 之后交给 Comparator 判 WA）。
Verdict derive_verdict(const RawOutcome& o, const Limits& limits,
                       bool& output_truncated);

// 最小安全环境（PATH/HOME/LANG/LC_ALL）。
//
// Sandbox Core 不继承宿主机完整环境，避免把代理、密钥路径、用户环境变量等
// 非必要信息暴露给用户程序。确有必要的额外变量由 SandboxRequest::envp 显式传入。
std::vector<std::string> minimal_env(const std::string& home_dir);

// argv = [executable, req.argv..., nullptr]；storage 持有字符串所有权。
void build_argv(const SandboxRequest& req,
                std::vector<std::string>& storage,
                std::vector<char*>& out_ptrs);

// envp = minimal_env(work_dir) + req.envp + nullptr；storage 持有所有权。
void build_envp(const SandboxRequest& req,
                std::vector<std::string>& storage,
                std::vector<char*>& out_ptrs);

// 文件字节数（不存在返回 0）。
uint64_t file_size(const std::string& path);

} // namespace cppjudge::sandbox_detail

namespace cppjudge {

// 平台后端工厂。
//
// make_sandbox() 只依赖这些工厂函数，不直接暴露具体后端类名。
// 当前正式 OJ 路径在 Linux 上由 make_linux_ns_sandbox() 提供。
#if defined(__linux__)
std::unique_ptr<SandboxBackend> make_linux_ns_sandbox();     // 仅 Linux 安全后端
#endif

} // namespace cppjudge
