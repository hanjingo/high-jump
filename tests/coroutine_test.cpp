#include <gtest/gtest.h>
#include <hj/sync/coroutine.hpp>

#ifndef __SANITIZE_ADDRESS__

// void(*)(void)
void f1(hj::coroutine<void>::push_type &out)
{
    ASSERT_EQ(true, true);
}

// void(*)(std::string)
void f2(hj::coroutine<std::string>::pull_type &in)
{
    ASSERT_STREQ(in.get().c_str(), "hj");
    in();

    ASSERT_STREQ(in.get().c_str(), "c++");
    in();

    ASSERT_STREQ(in.get().c_str(), "");
}

// std::string(*)(void)
void f3(hj::coroutine<std::string>::push_type &out)
{
    out("hello");
    out("world");
}

// NOTE: NOT SUPPORT std::string(*)(std::string)

TEST(coroutine, coroutine)
{
    // co1
    hj::coroutine<void>::pull_type co1(f1);

    hj::coroutine<void>::pull_type co1_lambda(
        [](hj::coroutine<void>::push_type &out) {
            ASSERT_EQ(true, true);
            // std::cout << "lambda" << std::endl;
        });

    COROUTINE(ASSERT_EQ(true, true);
              // std::cout << "COROUTINE" << std::endl;
    );

    // co2
    hj::coroutine<std::string>::push_type co2(f2);
    co2("hj");
    co2("c++");

    hj::coroutine<std::string>::push_type co2_lambda(
        hj::stack_alloc(1 * 1024 * 1024),
        [](hj::coroutine<std::string>::pull_type &in) {
            ASSERT_STREQ(in.get().c_str(), "hj");
            in();

            ASSERT_STREQ(in.get().c_str(), "c++");
            in();

            ASSERT_STREQ(in.get().c_str(), ""); // get() -> std::string()
        });
    co2_lambda("hj");
    co2_lambda("c++");


    hj::coroutine<std::string>::pull_type co3(f3);
    ASSERT_STREQ(co3.get().c_str(), "hello");
    co3(); // ["hello", "world"] -> ["world"]
    ASSERT_STREQ(co3.get().c_str(), "world");
    co3();                               // ["world"] -> []
    ASSERT_STREQ(co3.get().c_str(), ""); // get() -> std::string()

    hj::coroutine<std::string>::pull_type co3_lambda(
        [](hj::coroutine<std::string>::push_type &out) {
            out("hello");
            out("world");
        });
    ASSERT_STREQ(co3_lambda.get().c_str(), "hello");
    co3_lambda(); // ["hello", "world"] -> ["world"]
    ASSERT_STREQ(co3_lambda.get().c_str(), "world");
    co3_lambda();                               // ["world"] -> []
    ASSERT_STREQ(co3_lambda.get().c_str(), ""); // get() -> std::string()

    hj::coroutine<int>::pull_type co4_lambda(
        [](hj::coroutine<int>::push_type &out) {
            out(1);
            out(2);
            out(3);
        });
    ASSERT_EQ(co4_lambda.get(), 1);
    co4_lambda(); // [1, 2, 3] -> [2, 3]
    ASSERT_EQ(co4_lambda.get(), 2);
    co4_lambda(); // [2, 3] -> [3]
    ASSERT_EQ(co4_lambda.get(), 3);
    co4_lambda(); // [3] -> [3]
    ASSERT_EQ(co4_lambda.get(), 3);
    ASSERT_EQ(co4_lambda.get(), 3);
}

#endif