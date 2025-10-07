#include <gtest/gtest.h>
#include <hj/types/result.hpp>
#include <string>

enum error1 : int
{
    ok,
    fail,
    unknow
};

struct my_error
{
};

TEST(result, basic_value)
{
    auto fn  = []() -> hj::result<int> { return 42; };
    int  ret = fn().value();
    ASSERT_EQ(ret, 42);
}

TEST(result, error_enum)
{
    auto   fn  = []() -> hj::result<error1> { return error1::fail; };
    error1 ret = fn().value();
    ASSERT_EQ(ret, error1::fail);
}

TEST(result, error_handling)
{
    auto fn = []() -> hj::result<int> {
        return boost::leaf::new_error(my_error{});
    };
    auto r = fn();
    ASSERT_FALSE(r);
}

TEST(result, move)
{
    hj::result<std::string> r1 = std::string("abc");
    hj::result<std::string> r2 = std::move(r1);
    ASSERT_EQ(r2.value(), "abc");
}

TEST(result, void_type)
{
    auto             fn = []() -> hj::result<void> { return {}; };
    hj::result<void> r  = fn();
    ASSERT_TRUE(r);
}

TEST(result, error_type_void)
{
    auto fn = []() -> hj::result<void> {
        return boost::leaf::new_error(my_error{});
    };
    hj::result<void> r = fn();
    ASSERT_FALSE(r);
}