#include <gtest/gtest.h>
#include <libcpp/encoding/utf8.hpp>

TEST (utf8, is_valid)
{
    ASSERT_TRUE (libcpp::utf8::is_valid ("hello"));
    ASSERT_TRUE (libcpp::utf8::is_valid (
      std::string (reinterpret_cast<const char *> (u8 "你好，世界"))));
    ASSERT_TRUE (libcpp::utf8::is_valid ("abc\xE4\xB8\xAD\xE6\x96\x87"));

    ASSERT_FALSE (libcpp::utf8::is_valid ("\xC3\x28"));
    ASSERT_FALSE (libcpp::utf8::is_valid ("\xE2\x28\xA1"));
    ASSERT_FALSE (libcpp::utf8::is_valid ("\xF0\x28\x8C\x28"));
    ASSERT_FALSE (libcpp::utf8::is_valid ("\x80"));
}

TEST (utf8, decode)
{
    std::wstring hello = L"hello";
    ASSERT_TRUE (libcpp::utf8::decode (hello) == "hello");

    std::wstringstream wss;
    wss << hello;
    std::ostringstream oss;
    libcpp::utf8::decode (oss, wss);
    ASSERT_EQ (oss.str (), "hello");

    std::wstring cn = L"中文";
    std::wstringstream wss2;
    wss2 << cn;
    std::ostringstream oss2;
    libcpp::utf8::decode (oss2, wss2);
    ASSERT_EQ (oss2.str (), libcpp::utf8::decode (cn));

    std::wstring wstr = L"test";
    unsigned char buf[256];
    unsigned char *result = libcpp::utf8::decode (buf, wstr.c_str ());
    ASSERT_TRUE (result != nullptr);
    std::string str (reinterpret_cast<char *> (buf));
    ASSERT_EQ (str, "test");
}

TEST (utf8, encode)
{
    std::wstring hello = L"hello";
    auto utf8_hello = libcpp::utf8::decode (hello);
    ASSERT_TRUE (libcpp::utf8::encode (utf8_hello) == hello);

    std::string utf8_cn = libcpp::utf8::decode (L"中文");
    std::istringstream iss (utf8_cn);
    std::wostringstream wos;
    libcpp::utf8::encode (wos, iss);
    ASSERT_EQ (wos.str (), L"中文");

    std::string str = "test";
    wchar_t buf[256];
    wchar_t *result = libcpp::utf8::encode (
      buf, reinterpret_cast<const unsigned char *> (str.c_str ()));
    ASSERT_TRUE (result != nullptr);
    std::wstring wstr (reinterpret_cast<wchar_t *> (buf));
    ASSERT_EQ (wstr, L"test");
}