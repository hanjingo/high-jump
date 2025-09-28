#include <gtest/gtest.h>
#include <hj/types/any.hpp>

TEST(any, any_cast)
{
    int i = 123;

    ASSERT_EQ(hj::any_cast<int>(hj::any(i)), 123);

    ASSERT_EQ(hj::any_cast<int *>(hj::any(&i)) == &i, true);
}