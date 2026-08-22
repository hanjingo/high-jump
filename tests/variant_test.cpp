#include <gtest/gtest.h>
#include <hj/types/variant.hpp>
#include <string>
#include <type_traits>

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

TEST(variant, copy_semantics)
{
    hj::variant<int, std::string> a = std::string("hello");
    auto                          b = a;
    ASSERT_EQ(hj::get<std::string>(b), "hello");
    ASSERT_EQ(hj::get<std::string>(a), "hello");
}

TEST(variant, move_semantics)
{
    hj::variant<int, std::string> a = std::string("world");
    auto                          b = std::move(a);
    ASSERT_EQ(hj::get<std::string>(b), "world");
}

TEST(variant, assignment)
{
    hj::variant<int, std::string> v = 100;
    v                               = int{42};
    ASSERT_EQ(hj::get<int>(v), 42);

    v = std::string("assigned");
    ASSERT_EQ(hj::get<std::string>(v), "assigned");
}

TEST(variant, self_assignment)
{
    hj::variant<int, std::string> v = std::string("self");
    v                               = static_cast<decltype(v) &>(v);
    ASSERT_EQ(hj::get<std::string>(v), "self");
}

TEST(variant, same_type_duplicates_compile_check)
{
    SUCCEED();
}

#if (__cplusplus >= 201703L) || (defined(_MSC_VER) && _MSC_VER >= 1910)
TEST(variant, bad_variant_access)
{
    hj::variant<int, std::string> v = 123;
    EXPECT_THROW(hj::get<std::string>(v), std::bad_variant_access);
}
#endif

TEST(variant, reference_category)
{
    hj::variant<int, std::string> v = std::string("ref");

    static_assert(
        std::is_same_v<decltype(hj::get<std::string>(v)), std::string &>,
        "get() must return lvalue reference for lvalue variant");

    static_assert(std::is_same_v<decltype(hj::get<std::string>(std::move(v))),
                                 std::string &&>,
                  "get() must return rvalue reference for rvalue variant");

    const auto &cv = v;
    static_assert(
        std::is_same_v<decltype(hj::get<std::string>(cv)), const std::string &>,
        "get() must return const lvalue reference for const lvalue variant");
}

TEST(variant, traits_test)
{
    using MyVar = hj::variant<int, std::string, double>;

    static_assert(hj::variant_size_v<MyVar> == 3, "Size mismatch");

    static_assert(std::is_same_v<hj::variant_alternative_t<0, MyVar>, int>,
                  "Alternative 0 should be int");

    static_assert(
        std::is_same_v<hj::variant_alternative_t<1, MyVar>, std::string>,
        "Alternative 1 should be std::string");

    using FirstType = hj::variant_alternative_t<0, MyVar>;
    FirstType val   = 10;
    ASSERT_EQ(val, 10);
}