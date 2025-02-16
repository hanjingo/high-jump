#include <gtest/gtest.h>
#include <libcpp/strings/string_util.hpp>

TEST(string_util, split)
{
    auto arr1 = libcpp::string_util::split("abc;123;++", ";");
    ASSERT_EQ(arr1.size(), 3);
}

TEST(string_util, equal)
{
    ASSERT_EQ(libcpp::string_util::equal("hello", "hello"), true);
}

TEST(string_util, from_wchar)
{
    // ASSERT_STREQ(libcpp::string_util::from_wchar(libcpp::string_util::to_wchar(std::string("abc"))).c_str(), "abc");
}

TEST(string_util, from_wstring)
{
    ASSERT_STREQ(libcpp::string_util::from_wstring(libcpp::string_util::to_wstring(std::string("hello"))).c_str(), "hello");
}