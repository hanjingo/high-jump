#include <gtest/gtest.h>
#include <libcpp/types/optional.hpp>

libcpp::optional<int> hello(int in)
{
    return (in % 2 == 0) ? in : libcpp::optional<int>();
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