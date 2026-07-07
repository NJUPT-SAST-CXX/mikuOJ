#include <gtest/gtest.h>

#include "cppjudge/problem.h"

using namespace cppjudge;

TEST(ProblemManager, LoadValid) {
    std::string err;
    auto p = ProblemManager::load("problems/A+B", err);
    ASSERT_NE(p, nullptr) << err;
    EXPECT_EQ(p->title, "A+B Problem");
    EXPECT_EQ(p->limits.cpu_time_ms, 2000u);
    EXPECT_EQ(p->limits.memory_mb, 256u);
    EXPECT_EQ(p->compare_mode, "exact");
    EXPECT_EQ(p->test_cases.size(), 2u);
}

TEST(ProblemManager, WallIsCpuTimesThree) {
    std::string err;
    auto p = ProblemManager::load("problems/A+B", err);
    ASSERT_NE(p, nullptr) << err;
    EXPECT_EQ(p->limits.wall_time_ms, p->limits.cpu_time_ms * 3);
}

TEST(ProblemManager, TestCasesPairedAndSorted) {
    std::string err;
    auto p = ProblemManager::load("problems/A+B", err);
    ASSERT_NE(p, nullptr) << err;
    EXPECT_EQ(p->test_cases[0].index, 1);
    EXPECT_EQ(p->test_cases[1].index, 2);
    EXPECT_NE(p->test_cases[0].input_file.find("1.in"), std::string::npos);
    EXPECT_NE(p->test_cases[0].output_file.find("1.out"), std::string::npos);
}

TEST(ProblemManager, ValidatePasses) {
    std::string err;
    auto p = ProblemManager::load("problems/A+B", err);
    ASSERT_NE(p, nullptr) << err;
    EXPECT_TRUE(ProblemManager::validate(*p, err)) << err;
}

TEST(ProblemManager, NonexistentDirFails) {
    std::string err;
    auto p = ProblemManager::load("problems/DoesNotExist", err);
    EXPECT_EQ(p, nullptr);
    EXPECT_FALSE(err.empty());
}
