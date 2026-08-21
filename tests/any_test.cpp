#include <gtest/gtest.h>
#include <hj/types/any.hpp>

namespace any_test
{
struct Foo
{
    int value;

    bool operator==(const Foo &rhs) const { return value == rhs.value; }
};

struct Tracker
{
    static int alive;

    Tracker() { ++alive; }

    Tracker(const Tracker &) { ++alive; }

    ~Tracker() { --alive; }
};

int Tracker::alive = 0;
}

TEST(any, lifecycle_and_operations)
{
    hj::any a;
    EXPECT_FALSE(a.has_value());
#if (__cplusplus >= 201703L) || (defined(_MSC_VER) && _MSC_VER >= 1910)
    EXPECT_EQ(a.type(), typeid(void));
#endif

    // ② Reset test
    hj::any a_reset = 123;
    EXPECT_TRUE(a_reset.has_value());
    a_reset.reset();
    EXPECT_FALSE(a_reset.has_value());

    // ③ Assignment test
    hj::any a_assign;
    a_assign = 123;
    EXPECT_EQ(hj::any_cast<int>(a_assign), 123);

    a_assign = std::string("hello");
    EXPECT_EQ(hj::any_cast<std::string>(a_assign), "hello");

    // ④ Copy test
    hj::any a_src  = std::string("hello");
    hj::any a_copy = a_src;
    EXPECT_EQ(hj::any_cast<std::string>(a_src), "hello");
    EXPECT_EQ(hj::any_cast<std::string>(a_copy), "hello");

    // ⑤ Move any container test
    hj::any a_to_move = std::string("hello");
    hj::any a_moved   = std::move(a_to_move);
    EXPECT_EQ(hj::any_cast<std::string>(a_moved), "hello");

    hj::any a_cast_move = std::string("hello");
    auto    value       = hj::any_cast<std::string>(std::move(a_cast_move));
    EXPECT_EQ(value, "hello");
}

TEST(any, any_cast_variants)
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

    EXPECT_THROW(hj::any_cast<double>(a1), hj::bad_any_cast);
}

TEST(any, reference_semantics_and_modification)
{
    int     i = 123;
    hj::any a1(i);

    auto &ref_value = hj::any_cast<int &>(a1);
    ref_value       = 456;
    EXPECT_EQ(hj::any_cast<int>(a1), 456);

    const hj::any &ca1       = a1;
    const auto    &const_ref = hj::any_cast<const int &>(ca1);
    EXPECT_EQ(const_ref, 456);
}

TEST(any, pointer_cast_nullptr_behavior)
{
    hj::any        a  = 123;
    const hj::any &ca = a;

    EXPECT_EQ(hj::any_cast<double>(&a), nullptr);

    EXPECT_EQ(hj::any_cast<double>(&ca), nullptr);

    EXPECT_NE(hj::any_cast<int>(&a), nullptr);

    EXPECT_EQ(hj::any_cast<int>(static_cast<hj::any *>(nullptr)), nullptr);
    EXPECT_EQ(hj::any_cast<int>(static_cast<const hj::any *>(nullptr)),
              nullptr);
}

TEST(any, type_system_boundaries)
{
    hj::any a = 123;

    EXPECT_THROW(hj::any_cast<double>(a), hj::bad_any_cast);
}

TEST(any, custom_type_support)
{
    hj::any a   = any_test::Foo{123};
    auto    foo = hj::any_cast<any_test::Foo>(a);
    EXPECT_EQ(foo.value, 123);

    any_test::Foo &foo_ref = hj::any_cast<any_test::Foo &>(a);
    foo_ref.value          = 456;
    EXPECT_EQ(hj::any_cast<any_test::Foo>(a).value, 456);

    const hj::any       &ca        = a;
    const any_test::Foo &c_foo_ref = hj::any_cast<const any_test::Foo &>(ca);
    EXPECT_EQ(c_foo_ref.value, 456);
}

TEST(any, lifecycle_tracker)
{
    EXPECT_EQ(any_test::Tracker::alive, 0);

    {
        hj::any a = any_test::Tracker{};
        EXPECT_GT(any_test::Tracker::alive, 0);

        a.reset();
        EXPECT_EQ(any_test::Tracker::alive, 0);
    }

    EXPECT_EQ(any_test::Tracker::alive, 0);
}