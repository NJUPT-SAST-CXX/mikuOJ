#include <gtest/gtest.h>

#include <algorithm>
#include <string>

#include "cppjudge/seccomp_manager.h"

using namespace cppjudge;
using namespace cppjudge::seccomp;

namespace {
bool contains(const std::vector<std::string>& v, const std::string& n) {
    return std::find(v.begin(), v.end(), n) != v.end();
}
}  // namespace

TEST(Seccomp, ProfileForLang) {
    EXPECT_EQ(profile_for_lang("cpp"), SeccompProfile::Strict);
    EXPECT_EQ(profile_for_lang("c"), SeccompProfile::Strict);
    EXPECT_EQ(profile_for_lang("go"), SeccompProfile::Standard);
    EXPECT_EQ(profile_for_lang("rust"), SeccompProfile::Standard);
    EXPECT_EQ(profile_for_lang("python3"), SeccompProfile::Extended);
    EXPECT_EQ(profile_for_lang("java"), SeccompProfile::JVM);
    EXPECT_EQ(profile_for_lang("brainfuck"), SeccompProfile::Strict);  // 未知 → 最安全
}

TEST(Seccomp, AllowlistsNested) {
    const auto& s = Manager::allowlist_for_testing(SeccompProfile::Strict);
    const auto& st = Manager::allowlist_for_testing(SeccompProfile::Standard);
    const auto& e = Manager::allowlist_for_testing(SeccompProfile::Extended);
    const auto& j = Manager::allowlist_for_testing(SeccompProfile::JVM);
    EXPECT_GT(s.size(), 0u);
    EXPECT_GE(st.size(), s.size());
    EXPECT_GE(e.size(), st.size());
    EXPECT_GE(j.size(), e.size());
}

TEST(Seccomp, AllProfilesAllowExecve) {
    for (auto p : {SeccompProfile::Strict, SeccompProfile::Standard,
                   SeccompProfile::Extended, SeccompProfile::JVM}) {
        const auto& l = Manager::allowlist_for_testing(p);
        EXPECT_TRUE(contains(l, "execve"));
        EXPECT_TRUE(contains(l, "execveat"));
    }
}

TEST(Seccomp, NetworkAndPtraceBlocked) {
    // socket/connect/ptrace 不在任何运行时白名单 → 默认拒绝
    for (auto p : {SeccompProfile::Strict, SeccompProfile::JVM}) {
        const auto& l = Manager::allowlist_for_testing(p);
        EXPECT_FALSE(contains(l, "socket"));
        EXPECT_FALSE(contains(l, "connect"));
        EXPECT_FALSE(contains(l, "ptrace"));
    }
}

TEST(Seccomp, ViolationToString) {
    // syscall 0 在两个架构上都存在（x86_64=read, aarch64=io_setup），名称非空即可
    EXPECT_FALSE(Manager::violation_to_string(0).empty());
}
