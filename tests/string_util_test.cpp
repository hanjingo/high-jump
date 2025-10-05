#include <gtest/gtest.h>
#include <hj/util/string_util.hpp>

TEST(string_util, contains)
{
    ASSERT_EQ(hj::string::contains("hello world", "hello"), true);
    ASSERT_EQ(hj::string::contains("hello world", "hello1"), false);

    // Test with string_view compatibility
    std::string      str = "hello world";
    std::string_view sv  = str;
    ASSERT_TRUE(hj::string::contains(sv, "world"));
    ASSERT_FALSE(hj::string::contains(sv, "xyz"));
}

TEST(string_util, end_with)
{
    ASSERT_TRUE(hj::string::ends_with("hello world", "world"));
    ASSERT_FALSE(hj::string::ends_with("hello world", "worl"));
}

// Test new starts_with function
TEST(string_util, starts_with)
{
    ASSERT_TRUE(hj::string::starts_with("hello world", "hello"));
    ASSERT_FALSE(hj::string::starts_with("hello world", "world"));
    ASSERT_TRUE(hj::string::starts_with("", ""));
    ASSERT_FALSE(hj::string::starts_with("", "hello"));
    ASSERT_TRUE(hj::string::starts_with("hello", ""));

    // Test with string_view
    std::string_view sv = "hello world";
    ASSERT_TRUE(hj::string::starts_with(sv, "hello"));
    ASSERT_FALSE(hj::string::starts_with(sv, "world"));
}

// Test new ends_with function (more comprehensive)
TEST(string_util, ends_with)
{
    ASSERT_TRUE(hj::string::ends_with("hello world", "world"));
    ASSERT_FALSE(hj::string::ends_with("hello world", "hello"));
    ASSERT_TRUE(hj::string::ends_with("", ""));
    ASSERT_FALSE(hj::string::ends_with("", "world"));
    ASSERT_TRUE(hj::string::ends_with("world", ""));

    // Test with string_view
    std::string_view sv = "hello world";
    ASSERT_TRUE(hj::string::ends_with(sv, "world"));
    ASSERT_FALSE(hj::string::ends_with(sv, "hello"));
}

TEST(string_util, search)
{
    ASSERT_EQ(hj::string::search("hello w123orld", R"(\d+)")
                  == std::string("123"),
              true);
}

TEST(string_util, search_n)
{
    auto ret = hj::string::search_n("hello w123orld ni456hao", R"(\d+)");
    ASSERT_EQ(ret[0] == std::string("123"), true);
    ASSERT_EQ(ret[1] == std::string("456"), true);
}

TEST(string_util, split)
{
    auto arr1 = hj::string::split("abc;123;++", ";");
    ASSERT_EQ(arr1.size(), 3);

    auto arr2 = hj::string::split("broadcast,database,quote,sentinel", ",");
    ASSERT_EQ(arr2[0] == "broadcast", true);
    ASSERT_EQ(arr2[1] == "database", true);
    ASSERT_EQ(arr2[2] == "quote", true);
    ASSERT_EQ(arr2[3] == "sentinel", true);

    // Test edge cases
    auto empty_result = hj::string::split("", ",");
    ASSERT_EQ(empty_result.size(), 1);
    ASSERT_EQ(empty_result[0], "");

    auto single_result = hj::string::split("hello", ",");
    ASSERT_EQ(single_result.size(), 1);
    ASSERT_EQ(single_result[0], "hello");
}

// Test new replace_all function
TEST(string_util, replace_all)
{
    std::string original = "hello world hello universe";
    std::string result   = hj::string::replace_all(original, "hello", "hi");
    ASSERT_EQ(result, "hi world hi universe");

    // Test replace_all_inplace
    std::string inplace_str = "test test test";
    hj::string::replace_all_inplace(inplace_str, "test", "demo");
    ASSERT_EQ(inplace_str, "demo demo demo");

    // Test with empty replacement
    std::string spaces = "a b c d";
    ASSERT_EQ(hj::string::replace_all(spaces, " ", ""), "abcd");
}

TEST(string_util, replace)
{
    std::string str = "hello  world";
    hj::string::replace(str, " ", "");
    ASSERT_EQ(str == std::string("helloworld"), true);
}

TEST(string_util, equal)
{
    ASSERT_EQ(hj::string::equal("hello", "hello"), true);
    ASSERT_EQ(hj::string::equal("hello", "world"), false);

    // Test null pointer safety
    ASSERT_EQ(hj::string::equal(nullptr, nullptr), true);
    ASSERT_EQ(hj::string::equal("hello", nullptr), false);
    ASSERT_EQ(hj::string::equal(nullptr, "world"), false);
}

// Test new case-insensitive comparison
TEST(string_util, iequal)
{
    ASSERT_TRUE(hj::string::iequal("Hello", "hello"));
    ASSERT_TRUE(hj::string::iequal("WORLD", "world"));
    ASSERT_TRUE(hj::string::iequal("MixedCase", "mixedcase"));
    ASSERT_FALSE(hj::string::iequal("hello", "world"));
    ASSERT_TRUE(hj::string::iequal("", ""));
}

// Test string trimming functions
TEST(string_util, trim_functions)
{
    // Test trim_left
    ASSERT_EQ(hj::string::trim_left("   hello world"), "hello world");
    ASSERT_EQ(hj::string::trim_left("hello world   "), "hello world   ");
    ASSERT_EQ(hj::string::trim_left("   "), "");

    // Test trim_right
    ASSERT_EQ(hj::string::trim_right("   hello world"), "   hello world");
    ASSERT_EQ(hj::string::trim_right("hello world   "), "hello world");
    ASSERT_EQ(hj::string::trim_right("   "), "");

    // Test trim (both sides)
    ASSERT_EQ(hj::string::trim("   hello world   "), "hello world");
    ASSERT_EQ(hj::string::trim("\t\n hello \r\f"), "hello");

    // Test trim_inplace
    std::string inplace_str = "  hello world  ";
    hj::string::trim_inplace(inplace_str);
    ASSERT_EQ(inplace_str, "hello world");
}

// Test case conversion functions
TEST(string_util, case_conversion)
{
    ASSERT_EQ(hj::string::to_upper("hello world"), "HELLO WORLD");
    ASSERT_EQ(hj::string::to_upper("Hello123"), "HELLO123");
    ASSERT_EQ(hj::string::to_upper(""), "");

    ASSERT_EQ(hj::string::to_lower("HELLO WORLD"), "hello world");
    ASSERT_EQ(hj::string::to_lower("Hello123"), "hello123");
    ASSERT_EQ(hj::string::to_lower(""), "");
}

// Test regex search functions
TEST(string_util, regex_search_functions)
{
    // Test basic regex functionality using legacy search function
    std::string result = hj::string::search("hello w123orld", R"(\d+)");
    ASSERT_EQ(result, "123");

    // Test search_n (multiple matches)
    auto results = hj::string::search_n("hello w123orld ni456hao", R"(\d+)");
    ASSERT_EQ(results.size(), 2);
    if(results.size() >= 2)
    {
        ASSERT_EQ(results[0], "123");
        ASSERT_EQ(results[1], "456");
    }
}

// Test Unicode conversion functions
TEST(string_util, unicode_conversion)
{
    // Test safe conversion functions (non-optional versions)
    std::string ascii_str = "hello world";

    std::wstring safe_wide = hj::string::to_wstring_safe(ascii_str);
    ASSERT_FALSE(safe_wide.empty());

    std::string safe_str = hj::string::from_wstring_safe(safe_wide);
    ASSERT_EQ(safe_str, ascii_str);

    // Test empty string conversion
    std::wstring empty_wide = hj::string::to_wstring_safe("");
    ASSERT_TRUE(empty_wide.empty());

    std::string empty_str = hj::string::from_wstring_safe(L"");
    ASSERT_TRUE(empty_str.empty());
}

TEST(string_util, from_wchar)
{
#if defined(_WIN32)
    auto wstr = hj::string::to_wchar(std::string("abc"));
    ASSERT_STREQ(hj::string::from_wchar(wstr.c_str()).c_str(), "abc");
#endif
}

TEST(string_util, from_wstring)
{
#if defined(_WIN32)
    auto wstr = hj::string::to_wstring(std::string("hello"));
    ASSERT_STREQ(hj::string::from_wstring(wstr).c_str(), "hello");
#endif
}

TEST(string_util, from_ptr_addr)
{
    std::string *ptr = new std::string("hello");

    // not hex
    std::string str = hj::string::from_ptr_addr(ptr, false);
    ASSERT_EQ(static_cast<uintptr_t>(std::stoull(str, nullptr, 10)),
              reinterpret_cast<uintptr_t>(ptr));

    // for hex
    std::string str_hex = hj::string::from_ptr_addr(ptr, true);
    ASSERT_EQ(static_cast<uintptr_t>(std::stoull(str_hex, nullptr, 16)),
              reinterpret_cast<uintptr_t>(ptr));

    delete ptr;
}