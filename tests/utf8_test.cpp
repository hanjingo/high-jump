#include <gtest/gtest.h>
#include <libcpp/encoding/utf8.hpp>

TEST(utf8, from_unicode)
{
    std::wstring hello = L"hello";
    ASSERT_EQ(libcpp::utf8::from_unicode(hello) == "hello", true);
}

TEST(utf8, to_unicode)
{
    std::wstring hello = L"hello";
    auto utf8_hello = libcpp::utf8::from_unicode(hello);
    ASSERT_EQ(libcpp::utf8::to_unicode(utf8_hello) == hello, true);
}