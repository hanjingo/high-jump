
#include <gtest/gtest.h>
#include <hj/types/string_view.hpp>
#include <string>

TEST(string_view, basic_construct)
{
    const char     *cstr = "hello world";
    hj::string_view sv(cstr);
    ASSERT_EQ(sv.size(), 11);
    ASSERT_EQ(sv[0], 'h');
    ASSERT_EQ(sv.substr(0, 5), hj::string_view("hello"));
}

TEST(string_view, from_std_string)
{
    std::string     s = "abcdef";
    hj::string_view sv(s);
    ASSERT_EQ(sv.size(), 6);
    ASSERT_EQ(sv.substr(2, 3), hj::string_view("cde"));
}

TEST(string_view, compare)
{
    hj::string_view sv1("abc");
    hj::string_view sv2("abc");
    hj::string_view sv3("def");
    ASSERT_TRUE(sv1 == sv2);
    ASSERT_FALSE(sv1 == sv3);
    ASSERT_TRUE(sv1 < sv3);
}

TEST(string_view, empty_view)
{
    hj::string_view sv;
    ASSERT_TRUE(sv.empty());
    ASSERT_EQ(sv.size(), 0);
}

TEST(string_view, find_and_slice)
{
    hj::string_view sv("abcdefg");
    ASSERT_EQ(sv.find("cd"), 2u);
    hj::string_view sub = sv.substr(2, 2);
    ASSERT_EQ(sub, hj::string_view("cd"));
}