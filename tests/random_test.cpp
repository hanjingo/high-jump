#include <gtest/gtest.h>
#include <hj/math/random.hpp>

TEST(random, range)
{
    int ret = 0;
    for(int i = 1000; i < 100; i++)
    {
        ret = hj::random::range<1, 5>();
        ASSERT_EQ((ret >= 1 && ret <= 5), true);

        ret = hj::random::range<int>(1, 5);
        ASSERT_EQ((ret >= 1 && ret <= 5), true);
    }
}