#pragma once

// Linux 专用：mount / pid / net / ipc / uts namespace 编排。

#include "cppjudge/common.h"

#include <sched.h>       // CLONE_*
#include <sys/types.h>   // pid_t

#include <functional>
#include <string>
#include <vector>

namespace cppjudge::ns {

struct SetupResult {
    bool        ok = true;
    std::string error;
};

class Manager {
public:
    // 在唯一私有 new_root 上构建最小 tmpfs 根 + bind 白名单 + pivot_root。
    // 必须在子进程内（clone 后）以 root 调用。entries 已含 work_dir→"/box"(可写)。
    static SetupResult setup_rootfs(const std::vector<MountEntry>& entries,
                                    const std::string& new_root);

    // 在 new_root/dev 下建最小安全设备节点（null/zero/urandom/std*）。
    static SetupResult bind_minimal_devices(const std::string& new_root);

    // clone(2) 封装：父侧返回子 PID（失败 -1）。所有路径回收子栈（修 D16）。
    static pid_t clone_and_exec(int flags, const std::function<int()>& child_main);

    // 丢弃权限：setgroups/setresgid/setresuid → nobody（修 D3）。mounts 后、seccomp 前调用。
    static bool drop_privileges();

    static constexpr int ALL_NS_FLAGS =
        CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWNET | CLONE_NEWIPC | CLONE_NEWUTS;

private:
    static bool bind_mount_one(const std::string& source,
                               const std::string& target,
                               bool writable);
    static bool make_readonly(const std::string& target);
};

} // namespace cppjudge::ns
