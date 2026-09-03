#include <gtest/gtest.h>
#include <hj/os/compat.hpp>
#include <type_traits>

HJ_HOT void  test_hot_function_decl();
HJ_COLD void test_cold_function_decl();

HJ_FORCE_INLINE int test_force_inline(int a, int b)
{
    return a + b;
}

HJ_NO_INLINE int test_no_inline(int a, int b)
{
    return a - b;
}

HJ_HOT int test_hot_function(int x)
{
    if(HJ_LIKELY(x > 0))
    {
        return x * 2;
    }
    return 0;
}

HJ_COLD void test_cold_function()
{
}

void test_hot_function_decl()
{
}
void test_cold_function_decl()
{
}

HJ_DEPRECATED("test deprecation") inline void test_deprecated_fn()
{
}

struct HJ_DEPRECATED("test deprecated struct") TestDeprecatedStruct
{
    HJ_DEPRECATED("test deprecated member") void member_fn() {}
};


inline void trigger_deprecated_calls()
{
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996) // C4996: WAS DECLARED DEPRECATED
#endif

    test_deprecated_fn();

    TestDeprecatedStruct s;
    s.member_fn();

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif
}

TEST(compat, deprecated_attribute_trigger)
{
    trigger_deprecated_calls();
    SUCCEED();
}

TEST(compat, branch_prediction_preserves_value)
{
    EXPECT_TRUE(HJ_LIKELY(true));
    EXPECT_FALSE(HJ_LIKELY(false));

    EXPECT_TRUE(HJ_UNLIKELY(true));
    EXPECT_FALSE(HJ_UNLIKELY(false));

    int  val    = 42;
    int *ptr    = &val;
    int *null_p = nullptr;

    EXPECT_TRUE(HJ_LIKELY(val));
    EXPECT_FALSE(HJ_LIKELY(0));

    EXPECT_TRUE(HJ_LIKELY(ptr));
    EXPECT_FALSE(HJ_UNLIKELY(null_p));

    EXPECT_TRUE(HJ_LIKELY(val > 10 && ptr != nullptr));
    EXPECT_FALSE(HJ_UNLIKELY(val < 0 || null_p != nullptr));

    int counter = 0;
    if(HJ_LIKELY(++counter > 0))
    {
        EXPECT_EQ(counter, 1);
    }
    if(HJ_UNLIKELY(++counter < 0))
    {
        FAIL() << "Branch should not be taken";
    }
    EXPECT_EQ(counter, 2);
}

TEST(compat, hot_cold_attributes_callable)
{
    EXPECT_EQ(test_hot_function(5), 10);
    EXPECT_EQ(test_hot_function(-1), 0);

    test_cold_function();
    test_hot_function_decl();
    test_cold_function_decl();

    SUCCEED();
}