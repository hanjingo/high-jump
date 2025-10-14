#include <gtest/gtest.h>
#include <hj/time/timer.hpp>
#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>

using namespace std::chrono_literals;

TEST(timer, basic_callback)
{
    static std::atomic<int> tm_flag1{0};
    auto                    t = std::make_shared<hj::timer>(1000); // 1ms
    t->start([] { tm_flag1 = 42; });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_EQ(tm_flag1, 42);
}

TEST(timer, reset_and_wait)
{
    static std::atomic<int> tm_flag2{0};
    auto                    t = std::make_shared<hj::timer>(10000);
    t->start([] { tm_flag2 = 1; });
    t->reset(1, [] { tm_flag2 = 2; });
    for(int i = 0; i < 100 && tm_flag2 != 2; ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    ASSERT_EQ(tm_flag2, 2);
}

TEST(timer, cancel)
{
    static std::atomic<int> tm_flag3{0};
    auto                    t = std::make_shared<hj::timer>(10000);
    t->start([] { tm_flag3 = 1; });
    t->cancel();
    std::this_thread::sleep_for(2ms);
    ASSERT_EQ(tm_flag3, 0);
}

TEST(timer, move_semantics)
{
    static std::atomic<int> tm_flag4{0};
    auto                    t1 = std::make_shared<hj::timer>(1);
    t1->start([] { tm_flag4 = 9; });
    auto t2 = std::move(t1);
    for(int i = 0; i < 100 && tm_flag4 != 9; ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    ASSERT_EQ(tm_flag4, 9);
}