// 尝试 ptrace 附加到父进程 — seccomp 所有 profile 应拒绝
// ptrace 是经典沙箱逃逸手段：ptrace(PTRACE_ATTACH, 1, ...) 附加到 init
// 或父进程，可注入代码或窃取内存。
#include <sys/ptrace.h>
#include <unistd.h>
int main() {
    // 尝试附加到 PID 1 (init)，seccomp 应拦截 ptrace syscall
    ptrace(PTRACE_ATTACH, 1, nullptr, nullptr);
    return 0;
}
