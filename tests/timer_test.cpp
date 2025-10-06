#include <gtest/gtest.h>
#include <hj/time/timer.hpp>
#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>

using namespace std::chrono_literals;

TEST(timer, basic_callback)
{
    std::atomic<int> flag{0};
    auto             t = std::make_shared<hj::timer>(1000); // 1ms
    t->start([&] { flag = 42; });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_EQ(flag, 42);
}

TEST(timer, reset_and_wait)
{
    std::atomic<int> flag{0};
    auto             t = std::make_shared<hj::timer>(10000);
    t->start([&] { flag = 1; });
    t->reset(1, [&] { flag = 2; });
    for(int i = 0; i < 100 && flag != 2; ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    ASSERT_EQ(flag, 2);
}

TEST(timer, cancel)
{
    std::atomic<int> flag{0};
    auto             t = std::make_shared<hj::timer>(10000);
    t->start([&] { flag = 1; });
    t->cancel();
    std::this_thread::sleep_for(2ms);
    ASSERT_EQ(flag, 0);
}

TEST(timer, move_semantics)
{
    std::atomic<int> flag{0};
    auto             t1 = std::make_shared<hj::timer>(1);
    t1->start([&] { flag = 9; });
    auto t2 = std::move(t1);
    for(int i = 0; i < 100 && flag != 9; ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    ASSERT_EQ(flag, 9);
}