#include <gtest/gtest.h>

#include "cppjudge/comparator.h"

using namespace cppjudge;

// ---- exact ----
TEST(ComparatorExact, Match) {
    EXPECT_TRUE(Comparator::compare_exact("hello\nworld\n", "hello\nworld\n").is_match);
}
TEST(ComparatorExact, Mismatch) {
    auto r = Comparator::compare_exact("hello\nworld\n", "hello\nWelt\n");
    EXPECT_FALSE(r.is_match);
    EXPECT_EQ(r.mismatch_line, 2);
}
TEST(ComparatorExact, TrimsTrailingSpaces) {
    EXPECT_TRUE(Comparator::compare_exact("hello   \nworld\n", "hello\nworld\n").is_match);
}
TEST(ComparatorExact, TrimsTrailingNewlines) {
    EXPECT_TRUE(Comparator::compare_exact("hello\nworld\n\n\n", "hello\nworld").is_match);
}
TEST(ComparatorExact, CarriageReturnTrimmed) {
    EXPECT_TRUE(Comparator::compare_exact("a\r\nb\r\n", "a\nb\n").is_match);
}
TEST(ComparatorExact, FewerLines) {
    EXPECT_FALSE(Comparator::compare_exact("hello\n", "hello\nworld\n").is_match);
}
TEST(ComparatorExact, MoreLines) {
    EXPECT_FALSE(Comparator::compare_exact("a\nb\n", "a\n").is_match);
}
TEST(ComparatorExact, IgnoreEmptyLines) {
    EXPECT_TRUE(Comparator::compare_exact("a\n\nb\n", "a\nb\n", true, true, true).is_match);
}

// ---- floating ----
TEST(ComparatorFloat, ExactMatch) {
    EXPECT_TRUE(Comparator::compare_floating("3.141592653", "3.141592653").is_match);
}
TEST(ComparatorFloat, WithinAbsEps) {
    // diff = 1e-10 <= abs_eps(1e-9) → match（修正手册自相矛盾的用例）
    auto r = Comparator::compare_floating("1.0000000001", "1.0", 1e-9, 0.0);
    EXPECT_TRUE(r.is_match);
}
TEST(ComparatorFloat, OutsideEps) {
    auto r = Comparator::compare_floating("1.01", "1.0", 1e-9, 1e-6);
    EXPECT_FALSE(r.is_match);
}
TEST(ComparatorFloat, WithinRelEps) {
    EXPECT_TRUE(Comparator::compare_floating("1000.001", "1000.000", 1e-9, 1e-3).is_match);
}
TEST(ComparatorFloat, NaNMatches) {
    EXPECT_TRUE(Comparator::compare_floating("nan 42", "nan 42").is_match);
}
TEST(ComparatorFloat, InfMismatch) {
    EXPECT_FALSE(Comparator::compare_floating("inf", "-inf").is_match);
}
TEST(ComparatorFloat, InfMatch) {
    EXPECT_TRUE(Comparator::compare_floating("inf", "inf").is_match);
}
TEST(ComparatorFloat, ZeroVsNegativeZero) {
    EXPECT_TRUE(Comparator::compare_floating("0.0", "-0.0").is_match);
}
TEST(ComparatorFloat, TokenCountMismatch) {
    EXPECT_FALSE(Comparator::compare_floating("1 2 3", "1 2").is_match);
}
TEST(ComparatorFloat, StringTokensMatch) {
    EXPECT_TRUE(Comparator::compare_floating("hello world", "hello world").is_match);
}
TEST(ComparatorFloat, StringTokenMismatch) {
    EXPECT_FALSE(Comparator::compare_floating("hello world", "hello Welt").is_match);
}
TEST(ComparatorFloat, TypeMismatch) {
    EXPECT_FALSE(Comparator::compare_floating("abc", "1.0").is_match);
}
TEST(ComparatorFloat, DecimalPointLocaleIndependent) {
    // 确保 '.' 始终作小数点（修 D17 的 locale 依赖）
    EXPECT_TRUE(Comparator::compare_floating("3.5 2.5", "3.5 2.5").is_match);
}
// ---- 回归：NAN/INF 大小写变体（修 parse_double 缺失分支）----
TEST(ComparatorFloat, NaNUppercaseMatches) {
    EXPECT_TRUE(Comparator::compare_floating("NAN", "NAN").is_match);
}
TEST(ComparatorFloat, NaNMixedCaseMatches) {
    // NaN 与 NAN 都应被视为 NaN，且 NaN==NaN 匹配
    EXPECT_TRUE(Comparator::compare_floating("NaN", "NAN").is_match);
}
TEST(ComparatorFloat, InfUppercaseMatches) {
    EXPECT_TRUE(Comparator::compare_floating("INF", "INF").is_match);
}
TEST(ComparatorFloat, InfUppercaseMismatch) {
    EXPECT_FALSE(Comparator::compare_floating("INF", "-INF").is_match);
}
// ---- 回归：相对误差分母修正（ev 极小时不应退化为绝对误差）----
TEST(ComparatorFloat, RelEpsSmallExpected) {
    // ev=1e-10, uv=1e-7：实际相对误差 ≈ 999，不应匹配
    // 旧代码 denom=max(1.0,1e-10)=1.0 → rel=1e-7 < 1e-6 → 误判匹配
    auto r = Comparator::compare_floating("1e-7", "1e-10", 1e-12, 1e-6);
    EXPECT_FALSE(r.is_match);
}
TEST(ComparatorFloat, RelEpsSmallExpectedMatch) {
    // ev=1e-10, uv=1.0001e-10：相对误差 ≈ 1e-4，rel_eps=1e-3 → 应匹配
    auto r = Comparator::compare_floating("1.0001e-10", "1e-10", 1e-20, 1e-3);
    EXPECT_TRUE(r.is_match);
}
TEST(ComparatorFloat, BothNearZeroAbsEps) {
    // 两者均极小（< 1e-300），退回绝对误差：差距 < abs_eps → 匹配
    auto r = Comparator::compare_floating("0.0", "0.0", 1e-9, 1e-6);
    EXPECT_TRUE(r.is_match);
}
