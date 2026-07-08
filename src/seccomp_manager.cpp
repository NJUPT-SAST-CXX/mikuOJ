#include "cppjudge/seccomp_manager.h"
#include <seccomp.h>
#include <sys/syscall.h>
#include <algorithm>
#include <vector>
#include <cstdlib>

namespace cppjudge::seccomp {

static const std::vector<int> STRICT_ALLOWLIST = {
    
    SYS_read, SYS_write, SYS_open, SYS_openat, SYS_close,
    SYS_lseek, SYS_readlink, SYS_fstat, SYS_newfstatat,
    
    SYS_mmap, SYS_munmap, SYS_mprotect, SYS_brk,
   
    SYS_execve, SYS_execveat,
    SYS_exit, SYS_exit_group,
   
    SYS_getpid, SYS_gettid, SYS_getuid, SYS_getgid,
    SYS_clock_gettime, SYS_gettimeofday,
    SYS_nanosleep,
    SYS_arch_prctl,
    SYS_set_tid_address,
    SYS_set_robust_list,
    SYS_rseq,
    SYS_prlimit64,
};

static const std::vector<int> STANDARD_ALLOWLIST = []() {
    std::vector<int> v = STRICT_ALLOWLIST;
    v.insert(v.end(), {
        SYS_futex, SYS_sched_yield, SYS_sched_getaffinity,
        SYS_sigaltstack, SYS_rt_sigaction, SYS_rt_sigprocmask,
        SYS_rt_sigreturn,
        SYS_madvise,
        SYS_clone, SYS_clone3,
        SYS_getrandom,
        SYS_getcwd,
        SYS_stat, SYS_lstat,
        SYS_uname,
    });
    return v;
}();

static const std::vector<int> EXTENDED_ALLOWLIST = []() {
    std::vector<int> v = STANDARD_ALLOWLIST;
    v.insert(v.end(), {
        SYS_poll, SYS_ppoll, SYS_epoll_create1, SYS_epoll_ctl,
        SYS_epoll_pwait2, SYS_epoll_wait,
        SYS_eventfd2,
        SYS_pipe2,
        SYS_socketpair,
        SYS_getdents64,
        SYS_statx,
        SYS_access, SYS_faccessat, SYS_faccessat2,
        SYS_getxattr, SYS_lgetxattr,
        SYS_ioctl,
        SYS_fcntl,
        SYS_dup, SYS_dup2, SYS_dup3,
        SYS_sendfile,
        SYS_copy_file_range,
        SYS_rename, SYS_renameat2,
        SYS_mkdir, SYS_mkdirat,
        SYS_unlink, SYS_unlinkat,
        SYS_rmdir,
        SYS_readahead,
        SYS_membarrier,
        SYS_sysinfo,
    });
    return v;
}();

static const std::vector<int> JVM_ALLOWLIST = []() {
    std::vector<int> v = EXTENDED_ALLOWLIST;
    v.insert(v.end(), {
        SYS_tgkill, SYS_tkill,
        SYS_sched_setaffinity,
        SYS_sched_getparam, SYS_sched_getscheduler,
        SYS_get_robust_list,
        SYS_mincore,
        SYS_msync,
        SYS_mremap,
        SYS_shmget, SYS_shmat, SYS_shmdt, SYS_shmctl,
        SYS_getrusage,
        SYS_times,
        SYS_pread64, SYS_pwrite64,
        SYS_recvfrom, SYS_recvmsg,
    });
    return v;
}();

const std::vector<int>& Manager::get_allowlist(SeccompProfile profile) {
    switch (profile) {
        case SeccompProfile::Strict:   return STRICT_ALLOWLIST;
        case SeccompProfile::Standard: return STANDARD_ALLOWLIST;
        case SeccompProfile::Extended: return EXTENDED_ALLOWLIST;
        case SeccompProfile::JVM:      return JVM_ALLOWLIST;
    }
    return STRICT_ALLOWLIST;
}

const std::vector<int>& Manager::allowlist_for_testing(SeccompProfile p) {
    return get_allowlist(p);
}

SeccompProfile Manager::profile_for_lang(const std::string& lang) {
    if (lang == "c" || lang == "cpp" || lang == "c++") {
        return SeccompProfile::Strict;
    }
    if (lang == "go" || lang == "rust") {
        return SeccompProfile::Standard;
    }
    if (lang == "python3" || lang == "python" ||
        lang == "node" || lang == "nodejs" || lang == "javascript" ||
        lang == "ruby" || lang == "php" || lang == "perl") {
        return SeccompProfile::Extended;
    }
    if (lang == "java" || lang == "kotlin" || lang == "scala") {
        return SeccompProfile::JVM;
    }
    return SeccompProfile::Strict; 
}

bool Manager::install(SeccompProfile profile , bool is_compile) {
    (void)is_compile;
    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_KILL_PROCESS);
    if (ctx == nullptr) {
        return false;
    }

    const auto& allowlist = get_allowlist(profile);
    for (int syscall : allowlist) {
        if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, syscall, 0) != 0) {
            
        }
    }

    int rc = seccomp_load(ctx);
    seccomp_release(ctx);

    return rc == 0;
}

std::string Manager::violation_to_string(int syscall_num) {
    char* name = seccomp_syscall_resolve_num_arch(
        SCMP_ARCH_NATIVE, syscall_num);
    if (name != nullptr) {
        std::string result(name);
        free(name);
        return result;
    }
    return "unknown(" + std::to_string(syscall_num) + ")";
}

}