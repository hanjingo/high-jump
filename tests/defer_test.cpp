#include <gtest/gtest.h>
#include <hj/util/defer.hpp>

static int num = 0;

void defer_test()
{
    DEFER(num += 1; ASSERT_EQ(num, 3););
    DEFER(num += 1; ASSERT_EQ(num, 2););
    DEFER(num += 1; ASSERT_EQ(num, 1););
}

TEST(defer, defer)
{
    defer_test();
}