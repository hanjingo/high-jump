#include <gtest/gtest.h>
#include <hj/encoding/unicode.hpp>
#include <string>

#include <locale.h>

struct UnicodeLocaleInit
{
    UnicodeLocaleInit() { setlocale(LC_ALL, "en_US.UTF-8"); }
};
static UnicodeLocaleInit _unicode_locale_init;

TEST(unicode, from_utf8_to_utf8_basic)
{
    std::string  utf8_en = "hello";
    std::wstring w_en    = hj::unicode::from_utf8(utf8_en);
    EXPECT_EQ(hj::unicode::to_utf8(w_en), utf8_en);

    std::string  utf8_cn = u8"你好，世界";
    std::wstring w_cn    = hj::unicode::from_utf8(utf8_cn);
    EXPECT_EQ(hj::unicode::to_utf8(w_cn), utf8_cn);

    std::string  empty   = "";
    std::wstring w_empty = hj::unicode::from_utf8(empty);
    EXPECT_EQ(w_empty, L"");
    EXPECT_EQ(hj::unicode::to_utf8(w_empty), "");
}

TEST(unicode, invalid_utf8)
{
    std::string bad = "\xFF\xFF";
    try
    {
        auto w = hj::unicode::from_utf8(bad);
        (void) w;
        FAIL() << "Should throw on invalid utf8";
    }
    catch(const std::exception &)
    {
        SUCCEED();
    }
}