#include <gtest/gtest.h>
#include <libcpp/testing/error.hpp>

TEST(error, error)
{
    ASSERT_EQ(ERROR(1) == ERROR(1), true);
    ASSERT_EQ(ERROR(1) != ERROR(1), false);
    ASSERT_EQ(ERROR("hello") == ERROR("hello"), true);
}

TEST(error, create)
{
    ASSERT_EQ(libcpp::error_factory::create(1).code(), 1);
}

TEST(error, code)
{
    ASSERT_EQ(libcpp::error_factory::create(1).code(), 1);
}