#include <gtest/gtest.h>
#include <libcpp/math/random.hpp>

TEST(random, range)
{
    int ret = 0;
    for (int i = 1000; i < 100; i++)
    {
        ret = libcpp::random::range<1, 5>();
        ASSERT_EQ((ret >= 1 && ret <= 5), true);

        ret = libcpp::random::range<int>(1, 5);
        ASSERT_EQ((ret >= 1 && ret <= 5), true);
    }
}