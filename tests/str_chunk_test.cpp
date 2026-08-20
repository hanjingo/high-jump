#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <chrono>

#include <hj/str/str_chunk.hpp>

TEST(str_chunk, split)
{
    auto arr1 = hj::str::split("abc;123;++", ";");
    ASSERT_EQ(arr1.size(), 3);

    auto arr2 = hj::str::split("broadcast,database,quote,sentinel", ",");
    ASSERT_EQ(arr2[0] == "broadcast", true);
    ASSERT_EQ(arr2[1] == "database", true);
    ASSERT_EQ(arr2[2] == "quote", true);
    ASSERT_EQ(arr2[3] == "sentinel", true);

    // Test edge cases
    auto empty_result = hj::str::split("", ",");
    ASSERT_EQ(empty_result.size(), 1);
    ASSERT_EQ(empty_result[0], "");

    auto single_result = hj::str::split("hello", ",");
    ASSERT_EQ(single_result.size(), 1);
    ASSERT_EQ(single_result[0], "hello");

    // Test special characters
    std::string      str = "TinyStories-656K-Q3_K_M;Qwen3-Embedding-0.6B-Q8_0";
    std::string_view tag{";", 1};
    auto             items = hj::str::split(str, tag);
    ASSERT_TRUE(items.size() == 2);
    ASSERT_TRUE(items[0] == "TinyStories-656K-Q3_K_M");
    ASSERT_TRUE(items[1] == "Qwen3-Embedding-0.6B-Q8_0");
}

// Test string trimming functions
TEST(str_chunk, trim_functions)
{
    // Test trim_left
    ASSERT_EQ(hj::str::trim_left("   hello world"), "hello world");
    ASSERT_EQ(hj::str::trim_left("hello world   "), "hello world   ");
    ASSERT_EQ(hj::str::trim_left("   "), "");

    // Test trim_right
    ASSERT_EQ(hj::str::trim_right("   hello world"), "   hello world");
    ASSERT_EQ(hj::str::trim_right("hello world   "), "hello world");
    ASSERT_EQ(hj::str::trim_right("   "), "");

    // Test trim (both sides)
    ASSERT_EQ(hj::str::trim("   hello world   "), "hello world");
    ASSERT_EQ(hj::str::trim("\t\n hello \r\f"), "hello");

    // Test trim_inplace
    std::string inplace_str = "  hello world  ";
    hj::str::trim_inplace(inplace_str);
    ASSERT_EQ(inplace_str, "hello world");
}

// New tests: verify trim functions with custom 'target' parameter
TEST(str_chunk, trim_functions_with_target)
{
    // Single-character target
    std::string s1    = "---hel--lo---";
    auto        left  = hj::str::trim_left(s1, "-");
    auto        right = hj::str::trim_right(s1, "-");
    auto        both  = hj::str::trim(s1, "-");

    ASSERT_EQ(left, "hel--lo---");
    ASSERT_EQ(right, "---hel--lo");
    ASSERT_EQ(both, "hel--lo");

    // Multi-character target
    std::string s2 = "xyzxyhelloxyzxy";
    // trim by the exact sequence "xyz"
    auto left2  = hj::str::trim_left(s2, "xyz");
    auto right2 = hj::str::trim_right(s2, "xyz");
    auto both2  = hj::str::trim(s2, "xyz");

    // Note: trim functions treat 'target' as a set of characters to trim,
    // so trimming with "xyz" removes any leading/trailing 'x' or 'y' or 'z'.
    // For the string above, leading "xyzxy" will be trimmed to "helloxyzxy" on left trim,
    // and trailing will be trimmed similarly.
    ASSERT_EQ(left2, "helloxyzxy");
    ASSERT_EQ(right2, "xyzxyhello");
    ASSERT_EQ(both2, "hello");

    // Test trim_inplace with custom target
    std::string inplace = "--==good==--";
    hj::str::trim_inplace(inplace, "=-");
    ASSERT_EQ(inplace, "good");
}

TEST(str_chunk, regex_cache_lifecycle)
{
    const int iterations = 100;
    for(int i = 0; i < iterations; ++i)
    {
        std::string pattern = "pattern_" + std::to_string(i);
        auto        result  = hj::str::regex_split("a,b,c", pattern);
        ASSERT_EQ(result.size(), 1);
    }
}

TEST(str_chunk, regex_cache_concurrent_safety)
{
    auto task = []() {
        for(int i = 0; i < 50; ++i)
        {
            std::string pattern = "test_" + std::to_string(i % 10);
            auto        result  = hj::str::regex_split("data1,data2", pattern);
            ASSERT_FALSE(result.empty());
        }
    };

    std::vector<std::thread> threads;
    for(int i = 0; i < 8; ++i)
    {
        threads.emplace_back(task);
    }

    for(auto &t : threads)
    {
        t.join();
    }
}

TEST(chunk, regex_split_invalid_pattern)
{
    EXPECT_THROW({ hj::str::regex_split("a,b,c", "["); }, std::regex_error);
}