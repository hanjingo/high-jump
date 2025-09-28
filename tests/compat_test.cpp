#include <gtest/gtest.h>
#include <hj/os/compat.hpp>

#ifdef FORCE_INLINE
FORCE_INLINE int add(int a, int b)
{
    return a + b;
}
#endif

#ifdef NO_INLINE
NO_INLINE int sub(int a, int b)
{
    return a - b;
}
#endif

TEST(compat, force_inline_macro)
{
#ifdef FORCE_INLINE
    ASSERT_EQ(add(1, 2), 3);
#endif
}

TEST(compat, no_inline_macro)
{
#ifdef NO_INLINE
    ASSERT_EQ(sub(3, 1), 2);
#endif
}

TEST(compat, unary_function)
{
    struct MyFunc : std::unary_function<int, int>
    {
        int operator()(int x) const { return x + 1; }
    };
    MyFunc f;
    ASSERT_EQ(f(1), 2);
    int a = static_cast<MyFunc::argument_type>(1);
    int b = static_cast<MyFunc::result_type>(2);
    (void) a;
    (void) b;
    SUCCEED();
}