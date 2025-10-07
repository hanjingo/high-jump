#include <gtest/gtest.h>
#include <hj/types/optional.hpp>

hj::optional<int> hello(int in)
{
    return (in % 2 == 0) ? in : hj::optional<int>();
}

TEST(optional, value)
{
    ASSERT_EQ(hello(2).value(), 2);
}

TEST(optional, value_or)
{
    ASSERT_EQ(hello(3).value_or(123), 123);
    ASSERT_EQ(hello(2).value_or(123), 2);
}

TEST(optional, empty)
{
    hj::optional<int> o;
    ASSERT_FALSE(o.has_value());
    ASSERT_EQ(o.value_or(99), 99);
}

TEST(optional, emplace)
{
    hj::optional<std::string> o;
    o.emplace("abc");
    ASSERT_TRUE(o.has_value());
    ASSERT_EQ(o.value(), "abc");
}

TEST(optional, bool_operator)
{
    hj::optional<int> o1;
    hj::optional<int> o2(5);
    ASSERT_FALSE(o1);
    ASSERT_TRUE(o2);
}

TEST(optional, copy_move)
{
    hj::optional<int> o1(7);
    hj::optional<int> o2 = o1;
    hj::optional<int> o3 = std::move(o1);
    ASSERT_EQ(o2.value(), 7);
    ASSERT_EQ(o3.value(), 7);
}

TEST(optional, exception)
{
    hj::optional<int> o;
#if (__cplusplus >= 201703L) || (defined(_MSC_VER) && _MSC_VER >= 1910)
    EXPECT_THROW(o.value(), std::bad_optional_access);
#else
    EXPECT_THROW(o.value(), boost::bad_optional_access);
#endif
}