#include <gtest/gtest.h>
#include <libcpp/encoding/utf8.hpp>

TEST(utf8, is_valid)
{
    ASSERT_TRUE(libcpp::utf8::is_valid("hello"));
    ASSERT_TRUE(libcpp::utf8::is_valid(u8"你好，世界"));
    ASSERT_TRUE(libcpp::utf8::is_valid("abc\xE4\xB8\xAD\xE6\x96\x87"));

    ASSERT_FALSE(libcpp::utf8::is_valid("\xC3\x28"));
    ASSERT_FALSE(libcpp::utf8::is_valid("\xE2\x28\xA1"));
    ASSERT_FALSE(libcpp::utf8::is_valid("\xF0\x28\x8C\x28"));
    ASSERT_FALSE(libcpp::utf8::is_valid("\x80"));
}

TEST(utf8, from)
{
    std::wstring hello = L"hello";
    ASSERT_TRUE(libcpp::utf8::from(hello) == "hello");

    std::wstringstream wss;
    wss << hello;
    std::ostringstream oss;
    libcpp::utf8::from(oss, wss);
    ASSERT_EQ(oss.str(), "hello");

    std::wstring cn = L"中文";
    std::wstringstream wss2;
    wss2 << cn;
    std::ostringstream oss2;
    libcpp::utf8::from(oss2, wss2);
    ASSERT_EQ(oss2.str(), libcpp::utf8::from(cn));
}

TEST(utf8, to)
{
    std::wstring hello = L"hello";
    auto utf8_hello = libcpp::utf8::from(hello);
    ASSERT_TRUE(libcpp::utf8::to(utf8_hello) == hello);

    std::string utf8_cn = libcpp::utf8::from(L"中文");
    std::istringstream iss(utf8_cn);
    std::wostringstream wos;
    libcpp::utf8::to(wos, iss);
    ASSERT_EQ(wos.str(), L"中文");
}