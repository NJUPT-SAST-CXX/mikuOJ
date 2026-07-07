#pragma once

// Linux 专用：Cgroup Manager。
//
// 该模块负责 cgroup v2 的底层文件系统操作：创建叶子 cgroup、写入资源限制、
// attach 子进程、读取资源统计并销毁 cgroup。
//
// Sandbox Core 负责决定“何时创建、何时 attach、何时采样、何时清理”；
// Cgroup Manager 只提供这些动作的最小封装，不直接参与判题结果聚合。

#include <sys/types.h>

#include <cstdint>
#include <string>

namespace cppjudge::cgroup {

struct Limits {
    uint64_t cpu_time_us  = 0;   // 参考值；真正的 CPU-TLE 由 Sandbox Core 采样 + kill 实现
    uint64_t memory_bytes = 0;
    uint64_t max_pids     = 0;
};

struct Stats {
    uint64_t cpu_usage_us   = 0;
    uint64_t memory_kb      = 0;
    uint64_t memory_peak_kb = 0;
    bool     oom_killed     = false;
};

class Manager {
public:
    static bool is_cgroup_v2_available();

    // 创建叶子 cgroup：自动建 /sys/fs/cgroup/cppjudge 父层级并在 subtree_control
    // 委派 memory/pids/cpu 控制器。失败返回 is_valid()==false。
    static Manager create(const std::string& sandbox_id);

    bool               is_valid() const { return valid_; }
    const std::string& path() const { return path_; }

    // 写入资源限制。当前实现写入 memory.max、memory.swap.max=0、pids.max。
    // CPU 时间限制由 Sandbox Core 在 wait 循环中读取 cpu.stat 并主动 kill。
    bool               apply(const Limits& limits);

    // 把子进程加入 cgroup。Sandbox Core 必须在放行 execve 前调用它，保证限制生效。
    bool               attach(pid_t pid);

    // 读取 cpu.stat、memory.current / memory.peak、memory.events 等统计信息。
    Stats              collect() const;

    // kill cgroup 子树并删除叶子目录。调用后 Manager 失效。
    void               destroy();

private:
    bool        write_control(const std::string& file, const std::string& value) const;
    std::string read_control(const std::string& file) const;

    std::string path_;
    bool        valid_ = false;
};

} // namespace cppjudge::cgroup
