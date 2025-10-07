#include <gtest/gtest.h>
#include <hj/types/any.hpp>

TEST(any, any_cast)
{
    int     i = 123;
    hj::any a1(i);
    hj::any a2(&i);
    hj::any a3(std::string("hello"));

    ASSERT_EQ(hj::any_cast<int>(a1), 123);
    ASSERT_EQ(hj::any_cast<int *>(a2), &i);
    const hj::any &ca1 = a1;
    ASSERT_EQ(hj::any_cast<int>(ca1), 123);
    ASSERT_EQ(*hj::any_cast<int>(&a1), 123);
    ASSERT_EQ(*hj::any_cast<int *>(&a2), &i);
    ASSERT_EQ(*hj::any_cast<int>(&ca1), 123);
    ASSERT_EQ(hj::any_cast<std::string>(a3), "hello");

#if (__cplusplus >= 201703L) || (defined(_MSC_VER) && _MSC_VER >= 1910)
    EXPECT_THROW(hj::any_cast<double>(a1), std::bad_any_cast);
#else
    EXPECT_THROW(hj::any_cast<double>(a1), boost::bad_any_cast);
#endif
}