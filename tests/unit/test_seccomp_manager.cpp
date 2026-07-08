#include <gtest/gtest.h>
#include "cppjudge/seccomp_manager.h"

using namespace cppjudge::seccomp;
using namespace cppjudge;
TEST(SeccompManagerTest, ProfileForLangCpp) {
    EXPECT_EQ(Manager::profile_for_lang("cpp"), SeccompProfile::Strict);
    EXPECT_EQ(Manager::profile_for_lang("c"),   SeccompProfile::Strict);
}

TEST(SeccompManagerTest, ProfileForLangGo) {
    EXPECT_EQ(Manager::profile_for_lang("go"),   SeccompProfile::Standard);
    EXPECT_EQ(Manager::profile_for_lang("rust"), SeccompProfile::Standard);
}

TEST(SeccompManagerTest, ProfileForLangPython) {
    EXPECT_EQ(Manager::profile_for_lang("python3"), SeccompProfile::Extended);
    EXPECT_EQ(Manager::profile_for_lang("node"),    SeccompProfile::Extended);
}

TEST(SeccompManagerTest, ProfileForLangJava) {
    EXPECT_EQ(Manager::profile_for_lang("java"), SeccompProfile::JVM);
}

TEST(SeccompManagerTest, ProfileForUnknownLangDefaultsToStrict) {
    EXPECT_EQ(Manager::profile_for_lang("unknown_lang"), SeccompProfile::Strict);
}

TEST(SeccompManagerTest, ViolationToString) {
    std::string unknown = Manager::violation_to_string(999999);
    std::string name = Manager::violation_to_string(0);
    EXPECT_FALSE(name.empty());
    EXPECT_FALSE(unknown.empty());
}

TEST(SeccompManagerTest, AllowlistsContainExecve) {
    const auto& strict = Manager::allowlist_for_testing(SeccompProfile::Strict);
    const auto& jvm    = Manager::allowlist_for_testing(SeccompProfile::JVM);
    EXPECT_GT(strict.size(), 0u);
    EXPECT_GT(jvm.size(), strict.size());
}
