#include <gtest/gtest.h>
#include <libcpp/types/reflect.hpp>

TEST(reflect, name)
{
    ASSERT_EQ(libcpp::reflect::name(int(1)), "int");
}
