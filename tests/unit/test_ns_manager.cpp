#include <gtest/gtest.h>

#include <sched.h>
#include <sys/wait.h>
#include <unistd.h>

#include "cppjudge/ns_manager.h"

using namespace cppjudge::ns;

TEST(NsManager, MountEntryDefaultsReadOnly) {
    MountEntry e{"/usr/bin/g++", "/usr/bin/g++"};
    EXPECT_FALSE(e.writable);
}

TEST(NsManager, MountEntryWritable) {
    MountEntry e{"/tmp/work", "/box", true};
    EXPECT_TRUE(e.writable);
}

TEST(NsManager, AllNsFlagsCoverExpected) {
    EXPECT_TRUE(Manager::ALL_NS_FLAGS & CLONE_NEWNS);
    EXPECT_TRUE(Manager::ALL_NS_FLAGS & CLONE_NEWPID);
    EXPECT_TRUE(Manager::ALL_NS_FLAGS & CLONE_NEWNET);
    EXPECT_TRUE(Manager::ALL_NS_FLAGS & CLONE_NEWIPC);
    EXPECT_TRUE(Manager::ALL_NS_FLAGS & CLONE_NEWUTS);
}

TEST(NsManager, CloneAndExecSpawnsChild) {
    if (geteuid() != 0) GTEST_SKIP() << "clone(NEWPID/NEWNET) needs privilege";
    pid_t child = Manager::clone_and_exec(
        CLONE_NEWPID | CLONE_NEWNET | CLONE_NEWIPC | CLONE_NEWUTS,
        []() -> int { _exit(42); });
    ASSERT_GT(child, 0);
    int status = 0;
    waitpid(child, &status, 0);
    EXPECT_TRUE(WIFEXITED(status));
    EXPECT_EQ(WEXITSTATUS(status), 42);
}

TEST(NsManager, CloneRejectsBadFlags) {
    pid_t child = Manager::clone_and_exec(-1, []() -> int { _exit(0); });
    EXPECT_EQ(child, -1);
}
