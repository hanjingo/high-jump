
#include <gtest/gtest.h>
#include <hj/types/variant.hpp>
#include <string>

TEST(variant, basic_construct_get)
{
    hj::variant<int, double, std::string> v = 42;
    ASSERT_EQ(hj::get<int>(v), 42);
    v = 3.14;
    ASSERT_DOUBLE_EQ(hj::get<double>(v), 3.14);
    v = std::string("hello");
    ASSERT_EQ(hj::get<std::string>(v), "hello");
}

TEST(variant, const_get)
{
    const hj::variant<int, std::string> v = std::string("abc");
    ASSERT_EQ(hj::get<std::string>(v), "abc");
}

TEST(variant, move_get)
{
    hj::variant<int, std::string> v = std::string("move");
    std::string                   s = hj::get<std::string>(std::move(v));
    ASSERT_EQ(s, "move");
}

TEST(variant, type_switch)
{
    hj::variant<int, double> v = 1;
    ASSERT_EQ(hj::get<int>(v), 1);
    v = 2.5;
    ASSERT_DOUBLE_EQ(hj::get<double>(v), 2.5);
}

#if (__cplusplus >= 201703L) || (defined(_MSC_VER) && _MSC_VER >= 1910)
TEST(variant, bad_variant_access)
{
    hj::variant<int, std::string> v = 123;
    EXPECT_THROW(hj::get<std::string>(v), std::bad_variant_access);
}
#endif