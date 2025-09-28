#include <gtest/gtest.h>
#include <hj/types/result.hpp>

enum error1 : int
{
    ok,
    fail,
    unknow
};

TEST(result, result)
{
    auto fn = []() -> hj::result<error1> { return error1::fail; };

    error1 ret = fn().value();
    ASSERT_EQ(ret == error1::fail, true);
}