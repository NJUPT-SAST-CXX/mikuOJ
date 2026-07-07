#pragma once

#include "cppjudge/common.h"

#include <memory>
#include <string>
#include <vector>

namespace cppjudge {

// ============================================================
// Sandbox Core 对外输入：单次沙箱执行配置。
//
// 该结构由 Compiler 或 Judge/CLI 调度层构造，由 Sandbox::execute() 或
// SandboxBackend::execute() 消费。Sandbox Core 只接收已经整理好的运行配置，
// 不负责解析命令行、加载题目，也不负责比较输出。
//
// 注意：
//   1. executable 必须是宿主机上的绝对路径，Linux 后端会把它映射进沙箱。
//   2. argv 不包含 argv[0]；Sandbox Core 会自动把 executable 放到 argv[0]。
//   3. envp 只表示额外环境变量；后端会先构造最小安全环境，再追加 envp。
//   4. extra_mounts 来自 Language Manager，用于挂载编译器、解释器和运行时依赖。
//   5. is_compile 用于区分编译阶段和运行阶段，便于选择不同的 seccomp 策略。
// ============================================================
struct SandboxConfig {
    Limits      limits;
    std::string executable;                    // 要执行的可执行文件（解释器或编译产物，绝对路径）
    std::vector<std::string> argv;             // executable 之后的参数（不含 argv[0]）
    std::vector<std::string> envp;             // 追加到最小安全环境(PATH/HOME/LANG)之上的额外变量(如 GOCACHE)；绝不继承 host environ
    std::string stdin_path;                    // 宿主机路径；为空 → /dev/null
    std::string stdout_path;                   // 宿主机路径；为空 → 丢弃
    std::string stderr_path;                   // 宿主机路径；为空 → 丢弃
    std::string work_dir;                      // 沙箱工作目录（宿主机路径，需可写）
    std::string lang;                          // "cpp"/"python3"/... 决定 seccomp profile
    bool        is_compile = false;            // 编译阶段：放宽 syscall 策略（见 D6）
    std::vector<ns::MountEntry> extra_mounts;  // 语言运行时依赖（仅 Linux 后端使用）
};

// 内部实现沿用 SandboxRequest 命名；对外课程/任务接口使用 SandboxConfig。
using SandboxRequest = SandboxConfig;

// ============================================================
// Sandbox Core 对外输出：单次沙箱执行结果。
//
// Sandbox Core 负责把底层 wait status、signal、cgroup 统计、超时和输出限制等
// 原始信息归一化为 SandboxResult。Judge 层再把它转换或合并到测试点级 RunResult，
// Comparator 只在 verdict 仍为 AC 时继续判断是否 WA。
// ============================================================
struct SandboxResult {
    Verdict     verdict          = Verdict::AC;
    int         exit_code        = 0;
    int         signal_num       = 0;
    uint64_t    time_ms          = 0;   // 用户态 CPU 时间
    uint64_t    wall_time_ms     = 0;   // 墙上时间
    uint64_t    memory_kb        = 0;   // 峰值 RSS
    bool        output_truncated = false;
    std::string error_detail;           // SE 时判题系统内部错误细节
};

// ============================================================
// Sandbox Core 对外门面接口。
//
// 该接口与任务手册中的 cppjudge::Sandbox::execute(const SandboxConfig&) 保持一致。
// 内部实现仍复用 make_sandbox("auto") 和正式安全后端，避免把后端选择、Linux
// namespace/cgroup/seccomp 细节暴露给调用方。
// ============================================================
class Sandbox {
public:
    static SandboxResult execute(const SandboxConfig& config);
};

// ============================================================
// Sandbox Core 后端抽象。
//
// 这是 CLI / Judge / Compiler 看到的统一沙箱接口。具体隔离机制由平台后端实现：
//
//   - LinuxNsSandbox（src/sandbox_linux.cpp，__linux__）
//     负责编排 namespace、mount、cgroup、seccomp、stdio、exec、wait 和结果收集。
//
// mikuOJ 的正式 OJ 路径只保留 Linux 安全沙箱后端；教学或本地直跑后端不属于
// 当前正式判题链路。
// ============================================================
class SandboxBackend {
public:
    virtual ~SandboxBackend() = default;

    // 执行一次编译命令或用户程序，并返回统一结果。
    virtual SandboxResult execute(const SandboxRequest& req) = 0;

    // 返回后端名称，用于日志和诊断输出。
    virtual const char*   name()      const = 0;
};

// Sandbox Core 后端工厂。
//
// 后端类型标识来自 problem.json 或 CLI 合并后的 JudgeConfig::sandbox_type。
//   "auto"                 → Linux 安全后端
//   "linux-ns" / "nsjail"  → Linux 安全后端（仅 __linux__）
//
// 当前实现中，nsjail 名称作为正式安全路径的兼容别名，由 Linux 后端统一承接；
// 未知类型或当前平台不可用时返回 nullptr，并通过 error 给出系统错误原因。
std::unique_ptr<SandboxBackend> make_sandbox(const std::string& type, std::string& error);

} // namespace cppjudge
