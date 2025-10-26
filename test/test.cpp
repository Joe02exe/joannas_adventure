#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <vector>
#include <tuple>

// Simple functions to test
namespace {
int add(int a, int b) { return a + b; }

int divide(int a, int b) {
    if (b == 0) throw std::invalid_argument("division by zero");
    return a / b;
}

bool is_palindrome(const std::string& s) {
    std::string t;
    std::remove_copy_if(s.begin(), s.end(), std::back_inserter(t), [](char c){ return std::isspace(static_cast<unsigned char>(c)); });
    std::transform(t.begin(), t.end(), t.begin(), [](unsigned char c){ return std::tolower(c); });
    std::string r = t;
    std::reverse(r.begin(), r.end());
    return r == t;
}
} // namespace

// Basic tests
TEST(MathTest, Addition) {
    EXPECT_EQ(add(1, 1), 2);
    EXPECT_EQ(add(-3, 3), 0);
}

TEST(MathTest, Division) {
    EXPECT_EQ(divide(10, 2), 5);
    EXPECT_THROW(divide(1, 0), std::invalid_argument);
}

TEST(StringTest, Palindrome) {
    EXPECT_TRUE(is_palindrome("A man a plan a canal Panama"));
    EXPECT_FALSE(is_palindrome("not a palindrome"));
}

// Fixture example
class VectorFixture : public ::testing::Test {
protected:
    void SetUp() override { v = {1, 2, 3}; }
    std::vector<int> v;
};

TEST_F(VectorFixture, HasCorrectSize) {
    EXPECT_EQ(v.size(), 3u);
    v.push_back(4);
    EXPECT_EQ(v.back(), 4);
}

// Parameterized test for add
class AddParamTest : public ::testing::TestWithParam<std::tuple<int,int,int>> {};

TEST_P(AddParamTest, AddsCorrectly) {
    auto [a, b, expected] = GetParam();
    EXPECT_EQ(add(a, b), expected);
}

INSTANTIATE_TEST_SUITE_P(
    BasicCases,
    AddParamTest,
    ::testing::Values(
        std::make_tuple(0, 0, 0),
        std::make_tuple(2, 2, 4),
        std::make_tuple(-1, -2, -3),
        std::make_tuple(100, -50, 50)
    )
);

// main (optional if linking with gtest_main library)
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}