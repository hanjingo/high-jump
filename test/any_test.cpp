#include <gtest/gtest.h>
#include <libcpp/types/any.hpp>

TEST(any, any_cast)
{
    int i = 123;

    ASSERT_EQ(libcpp::any_cast<int>(libcpp::any(i)), 123);

    ASSERT_EQ(libcpp::any_cast<int*>(libcpp::any(&i)) == &i, true);
}