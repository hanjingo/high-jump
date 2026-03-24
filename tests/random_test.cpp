#include <gtest/gtest.h>
#include <type_traits>
#include <hj/math/random.hpp>

namespace
{
// Detection for free-function NTTP overload: hj::random::range<Min,Max>()
template <int Min, int Max, typename = void>
struct has_range_nttp : std::false_type
{
};

template <int Min, int Max>
struct has_range_nttp<Min,
                      Max,
                      std::void_t<decltype(hj::random::range<Min, Max>())>>
    : std::true_type
{
};

} // namespace

TEST(random, range_int)
{
    hj::random::engine rng;
    for(int i = 0; i < 1000; ++i)
    {
        int ret = rng.range<int>(1, 5);
        ASSERT_GE(ret, 1);
        ASSERT_LE(ret, 5);
    }
}

TEST(random, range_real)
{
    hj::random::engine rng;
    for(int i = 0; i < 1000; ++i)
    {
        double ret = rng.range_real<double>(0.0, 1.0);
        ASSERT_GE(ret, 0.0);
        ASSERT_LE(ret, 1.0);
    }
}

TEST(random, range_bulk)
{
    hj::random::engine rng;
    auto               vec = rng.range_bulk<int>(10, 20, 100);
    ASSERT_EQ(vec.size(), 100);
    for(auto v : vec)
    {
        ASSERT_GE(v, 10);
        ASSERT_LE(v, 20);
    }
}

TEST(random, normal)
{
    hj::random::engine rng;
    for(int i = 0; i < 1000; ++i)
    {
        double ret = rng.normal<double>(100.0, 10.0);
        // Use ±7 standard deviations to avoid rare outlier failures
        ASSERT_GT(ret, 30.0);
        ASSERT_LT(ret, 170.0);
    }
}

TEST(random, range_nttp)
{
    // Test the compile-time (non-type template parameter) overload if it exists.
    // Fall back to the runtime overload when the NTTP free-function isn't available.
    for(int i = 0; i < 1000; ++i)
    {
        if constexpr(has_range_nttp<1, 5>::value)
        {
            int ret = hj::random::range<1, 5>();
            ASSERT_GE(ret, 1);
            ASSERT_LE(ret, 5);
        } else
        {
            int ret = hj::random::engine::instance().range<int>(1, 5);
            ASSERT_GE(ret, 1);
            ASSERT_LE(ret, 5);
        }
    }
}