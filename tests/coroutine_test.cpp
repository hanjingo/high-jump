#include <gtest/gtest.h>
#include <libcpp/sync/coroutine.hpp>

void f1(libcpp::coroutine<std::string>::push_type& out)
{
    out("hello");
    out("world");
    out("c++");
}

TEST(coroutine, coroutine)
{
    libcpp::coroutine<std::string>::pull_type co1(f1);
    ASSERT_STREQ(co1.get().c_str(), "hello");
    co1();

    ASSERT_STREQ(co1.get().c_str(), "world");
    co1();

    ASSERT_STREQ(co1.get().c_str(), "c++");
    co1();

    libcpp::coroutine<int>::pull_type co2([](libcpp::coroutine<int>::push_type & out) {
        out(1);
        out(2);
        out(3);
    });
    ASSERT_EQ(co2.get(), 1);
    co2();

    ASSERT_EQ(co2.get(), 2);
    co2();

    ASSERT_EQ(co2.get(), 3);
    co2();

    libcpp::coroutine<int>::push_type co3([](libcpp::coroutine<int>::pull_type & in) {
        ASSERT_EQ(in.get(), 4);
        in();
        ASSERT_EQ(in.get(), 5);
        in();
    });
    co3(4);
    co3(5);
}