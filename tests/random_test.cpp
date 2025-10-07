#include <gtest/gtest.h>
#include <hj/math/random.hpp>

TEST(random, range_int)
{
    hj::random rng;
    for(int i = 0; i < 1000; ++i)
    {
        int ret = rng.range<int>(1, 5);
        ASSERT_GE(ret, 1);
        ASSERT_LE(ret, 5);
    }
}

TEST(random, range_real)
{
    hj::random rng;
    for(int i = 0; i < 1000; ++i)
    {
        double ret = rng.range_real<double>(0.0, 1.0);
        ASSERT_GE(ret, 0.0);
        ASSERT_LE(ret, 1.0);
    }
}

TEST(random, range_bulk)
{
    hj::random rng;
    auto       vec = rng.range_bulk<int>(10, 20, 100);
    ASSERT_EQ(vec.size(), 100);
    for(auto v : vec)
    {
        ASSERT_GE(v, 10);
        ASSERT_LE(v, 20);
    }
}

TEST(random, normal)
{
    hj::random rng;
    for(int i = 0; i < 1000; ++i)
    {
        double ret = rng.normal<double>(100.0, 10.0);
        ASSERT_GT(ret, 50.0);
        ASSERT_LT(ret, 150.0);
    }
}