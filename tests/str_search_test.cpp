#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <chrono>

#include <hj/str/str_search.hpp>

TEST(str_search, contains)
{
    ASSERT_EQ(hj::str::contains("hello world", "hello"), true);
    ASSERT_EQ(hj::str::contains("hello world", "hello1"), false);

    // Test with string_view compatibility
    std::string      str = "hello world";
    std::string_view sv  = str;
    ASSERT_TRUE(hj::str::contains(sv, "world"));
    ASSERT_FALSE(hj::str::contains(sv, "xyz"));
}

// Test new starts_with function
TEST(str_search, starts_with)
{
    ASSERT_TRUE(hj::str::starts_with("hello world", "hello"));
    ASSERT_FALSE(hj::str::starts_with("hello world", "world"));
    ASSERT_TRUE(hj::str::starts_with("", ""));
    ASSERT_FALSE(hj::str::starts_with("", "hello"));
    ASSERT_TRUE(hj::str::starts_with("hello", ""));

    // Test with string_view
    std::string_view sv = "hello world";
    ASSERT_TRUE(hj::str::starts_with(sv, "hello"));
    ASSERT_FALSE(hj::str::starts_with(sv, "world"));
}

// Test new ends_with function (more comprehensive)
TEST(str_search, ends_with)
{
    ASSERT_TRUE(hj::str::ends_with("hello world", "world"));
    ASSERT_FALSE(hj::str::ends_with("hello world", "worl"));

    ASSERT_TRUE(hj::str::ends_with("hello world", "world"));
    ASSERT_FALSE(hj::str::ends_with("hello world", "hello"));
    ASSERT_TRUE(hj::str::ends_with("", ""));
    ASSERT_FALSE(hj::str::ends_with("", "world"));
    ASSERT_TRUE(hj::str::ends_with("world", ""));

    // Test with string_view
    std::string_view sv = "hello world";
    ASSERT_TRUE(hj::str::ends_with(sv, "world"));
    ASSERT_FALSE(hj::str::ends_with(sv, "hello"));
}

TEST(str_search, regex_search)
{
    // Compare against std::regex_search for consistency
    std::string s = "hello w123orld";
    std::regex  re(R"(\d+)");
    std::smatch m;
    bool        found = std::regex_search(s, m, re);
    ASSERT_TRUE(found);
    auto mine = hj::str::regex_search(s, R"(\d+)");
    ASSERT_EQ(mine, m.str(0));
}

TEST(str_search, search_n)
{
    auto ret = hj::str::regex_search_n("hello w123orld ni456hao", R"(\d+)", 2);
    ASSERT_EQ(ret[0] == std::string("123"), true);
    ASSERT_EQ(ret[1] == std::string("456"), true);
}

TEST(str_search, cequal)
{
    ASSERT_EQ(hj::str::cequal("hello", "hello"), true);
    ASSERT_EQ(hj::str::cequal("hello", "world"), false);

    // Test null pointer safety
    ASSERT_EQ(hj::str::cequal(nullptr, nullptr), true);
    ASSERT_EQ(hj::str::cequal("hello", nullptr), false);
    ASSERT_EQ(hj::str::cequal(nullptr, "world"), false);
}

// Test new case-insensitive comparison
TEST(str_search, equal)
{
    ASSERT_TRUE(hj::str::equal("Hello", "hello"));
    ASSERT_TRUE(hj::str::equal("WORLD", "world"));
    ASSERT_TRUE(hj::str::equal("MixedCase", "mixedcase"));
    ASSERT_FALSE(hj::str::equal("hello", "world"));
    ASSERT_TRUE(hj::str::equal("", ""));
}

// Test regex search functions
TEST(str, regex_search_functions)
{
    // Test basic regex functionality using std::regex as ground truth
    {
        const std::string text = "hello w123orld";
        const std::regex  re(R"(\d+)");
        std::smatch       m;
        bool              found = std::regex_search(text, m, re);
        ASSERT_TRUE(found);
        auto mine = hj::str::regex_search(text, R"(\d+)");
        ASSERT_TRUE(mine == m.str(0));
    }

    // Test search_n (multiple matches) against std::sregex_iterator
    {
        const std::string        text = "hello w123orld ni456hao";
        const std::regex         re(R"(\d+)");
        std::sregex_iterator     it(text.begin(), text.end(), re), end;
        std::vector<std::string> expected;
        for(; it != end; ++it)
            expected.emplace_back(it->str(0));

        auto results = hj::str::regex_search_n(text, R"(\d+)", 6);

        ASSERT_EQ(results.size(), expected.size());
        for(size_t i = 0; i < expected.size(); ++i)
            ASSERT_EQ(results[i], expected[i]);
    }
}


// Test regex_search which returns optional<std::string>
TEST(str, regex_search_first_optional)
{
    using std::regex;
    using std::smatch;
    using std::string;

    // Helper lambda: compare hj::str::regex_search with std::regex_search
    auto compare_with_std = [](const string &s, const string &pattern) {
        auto mine = hj::str::regex_search_first(s, pattern);
        try
        {
            regex  re(pattern);
            smatch m;
            bool   found = std::regex_search(s, m, re);
            if(found)
            {
                ASSERT_TRUE(mine.has_value());
                ASSERT_EQ(*mine, m.str(0));
            } else
            {
                ASSERT_FALSE(mine.has_value());
            }
        }
        catch(const std::regex_error &)
        {
            // std::regex throws for invalid patterns; our function should return nullopt
            ASSERT_FALSE(mine.has_value());
        }
    };

    // Basic numeric match
    compare_with_std("hello w123orld", "\\d+");

    // Capture group: should still return full match (group 0)
    compare_with_std("abcXYZdef", "XYZ");

    // No match
    compare_with_std("abcdef", "\"notfound\"");

    // Empty pattern (std::regex allows empty pattern and matches at pos 0)
    compare_with_std("anything", "");

    // Special characters and escaped sequences
    compare_with_std("a+b(c)*d?", "a\+b\\(c\\)\*d\?");

    // Long complex pattern
    compare_with_std("user: alice@example.com",
                     R"([\w.-]+@[\w.-]+\.[A-Za-z]{2,6})");

    // Invalid pattern must return nullopt (do not throw)
    auto bad = hj::str::regex_search_first("abc", "(");
    ASSERT_FALSE(bad.has_value());
}