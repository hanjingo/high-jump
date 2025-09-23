#include <gtest/gtest.h>
#include <hj/types/optional.hpp>

hj::optional<int> hello(int in)
{
    return (in % 2 == 0) ? in : hj::optional<int>();
}

TEST(optional, value)
{
    ASSERT_EQ(hello(2).value(), 2);
}

TEST(optional, value_or)
{
    ASSERT_EQ(hello(3).value_or(123), 123);
    ASSERT_EQ(hello(2).value_or(123), 2);
}