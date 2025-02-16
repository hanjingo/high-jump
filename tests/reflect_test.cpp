#include <gtest/gtest.h>
#include <libcpp/types/reflect.hpp>

TEST(reflect, type_name)
{
    ASSERT_EQ(libcpp::reflect::type_name(int(1)), "int");
}
