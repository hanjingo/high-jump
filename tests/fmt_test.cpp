#include <gtest/gtest.h>
#include <hj/encoding/fmt.hpp>

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

TEST(fmt, format_basic)
{
    EXPECT_EQ(hj::format("{}-{}", "hello", "world"), "hello-world");

    EXPECT_EQ(hj::format("{1}-{0}", "hello", "world"), "world-hello");

    EXPECT_EQ(hj::format("{} {} {} {}", 42, 3.14, 'A', true), "42 3.14 A true");

    EXPECT_EQ(hj::format("{:#04x}", 15), "0x0f");
    EXPECT_EQ(hj::format("{:.2f}", 3.14159), "3.14");
}

TEST(fmt, format_containers)
{
    std::vector<int> vec = {1, 2, 3};
    EXPECT_EQ(hj::format("{}", vec), "[1, 2, 3]");

    std::pair<std::string, int> pair = {"key", 100};
    EXPECT_EQ(hj::format("{}", pair), "(\"key\", 100)");

    std::map<std::string, std::string> map = {{"a", "1"}};
    EXPECT_EQ(hj::format("{}", map), "{\"a\": \"1\"}");

    std::set<int> set = {10, 20};
    EXPECT_EQ(hj::format("{}", set), "{10, 20}");
}

TEST(fmt, vformat_dynamic)
{
    std::string pattern = "Dynamic: {1} -> {0}";
    EXPECT_EQ(hj::vformat(pattern, "val", "key"), "Dynamic: key -> val");

    std::string_view pattern_view = "View: {}";
    EXPECT_EQ(hj::vformat(pattern_view, 123), "View: 123");

    std::vector<std::string> str_vec = {"apple", "banana"};
    EXPECT_EQ(hj::vformat(pattern_view, str_vec),
              "View: [\"apple\", \"banana\"]");
}

TEST(fmt, format_edge_cases)
{
    EXPECT_EQ(hj::format("hello world"), "hello world");
    EXPECT_EQ(hj::vformat("hello world"), "hello world");

    EXPECT_EQ(hj::format(""), "");
    EXPECT_EQ(hj::vformat(""), "");
}