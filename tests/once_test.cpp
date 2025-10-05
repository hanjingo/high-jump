#include <gtest/gtest.h>
#include <hj/util/once.hpp>

void test_once_function(int *counter)
{
    ONCE(*counter = 1;);
}

TEST(once, simple_execution)
{
    static int counter = 0;

    counter = 0;

    test_once_function(&counter);
    EXPECT_EQ(counter, 1);

    test_once_function(&counter);
    EXPECT_EQ(counter, 1);

    test_once_function(&counter);
    EXPECT_EQ(counter, 1);
}

TEST(once, once_in_loop)
{
    static int value = 0;

    value = 0;

    for(int i = 0; i < 5; ++i)
    {
        ONCE(value = 42;);
    }

    EXPECT_EQ(value, 42);
}

TEST(once, complex_expression)
{
    static int a = 0, b = 0;

    a = b = 0;

    ONCE(a = 10; b = 20;);
    EXPECT_EQ(a, 10);
    EXPECT_EQ(b, 20);

    for(int i = 0; i < 3; ++i)
    {
        ONCE(a = 100; b = 200;);
    }

    EXPECT_EQ(a, 100);
    EXPECT_EQ(b, 200);
}
