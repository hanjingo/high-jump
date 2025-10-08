
#include <gtest/gtest.h>
#include <hj/testing/unit_test.hpp>

TEST(unit_test, header_compiles)
{
    SUCCEED();
}

TEST(unit_test, gtest_assertion)
{
    EXPECT_EQ(1 + 1, 2);
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);
}