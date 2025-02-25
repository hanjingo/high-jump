#include <gtest/gtest.h>
#include <libcpp/strings/string.hpp>

TEST(string, split)
{
    auto arr1 = libcpp::string::split("abc;123;++", ";");
    ASSERT_EQ(arr1.size(), 3);
}

TEST(string, equal)
{
    ASSERT_EQ(libcpp::string::equal("hello", "hello"), true);
}

TEST(string, from_wchar)
{
    // ASSERT_STREQ(libcpp::string::from_wchar(libcpp::string::to_wchar(std::string("abc"))).c_str(), "abc");
}

TEST(string, from_wstring)
{
    ASSERT_STREQ(libcpp::string::from_wstring(libcpp::string::to_wstring(std::string("hello"))).c_str(), "hello");
}

TEST(string, fmt)
{
    ASSERT_EQ(libcpp::string::fmt("{}-{}", "hello", "world") == std::string("hello-world"), true);
    ASSERT_EQ(libcpp::string::fmt("{1}-{0}", "hello", "world") == std::string("world-hello"), true);
}