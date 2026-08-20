#include <gtest/gtest.h>
#include <hj/str/str_replace.hpp>

// Test new replace_all function
TEST(str, replace_all)
{
    std::string original = "hello world hello universe";
    std::string result   = hj::str::replace_all(original, "hello", "hi");
    ASSERT_EQ(result, "hi world hi universe");

    // Test replace_all_inplace
    std::string inplace_str = "test test test";
    hj::str::replace_all_inplace(inplace_str, "test", "demo");
    ASSERT_EQ(inplace_str, "demo demo demo");

    // Test with empty replacement
    std::string spaces = "a b c d";
    ASSERT_EQ(hj::str::replace_all(spaces, " ", ""), "abcd");
}