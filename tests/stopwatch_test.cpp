#include <gtest/gtest.h>
#include <hj/time/stopwatch.hpp>
#include <thread>
#include <atomic>

TEST(stopwatch, time_passed_basic)
{
    hj::stopwatch sw;
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    auto ms  = sw.elapsed_ms();
    auto ns  = sw.elapsed_ns();
    auto sec = sw.elapsed_secs();
    ASSERT_GT(ms, 0);
    ASSERT_GT(ns, 0);
    ASSERT_GE(ns, ms * 1000000);
    ASSERT_EQ(sec, 0); // 2ms < 1s

    HJ_WATCH(
        // macro test
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        auto ms  = HJ_WATCH_PASSED_MS();
        auto ns  = HJ_WATCH_PASSED_NS();
        auto sec = HJ_WATCH_PASSED_SECS();
        ASSERT_GT(ms, 0);
        ASSERT_GT(ns, 0);
        ASSERT_GE(ns, ms * 1000000);
        ASSERT_EQ(sec, 0); // 2ms < 1s
    )
}

TEST(stopwatch, time_start_reset)
{
    hj::stopwatch sw;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    auto ms1 = sw.elapsed_ms();
    ASSERT_GT(ms1, 0);
    sw.reset();
    auto ms2 = sw.elapsed_ms();
    ASSERT_LT(ms2, ms1);
}

TEST(stopwatch, time_passed_duration_type)
{
    hj::stopwatch sw;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    auto dur = sw.elapsed();
    auto ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
    ASSERT_GT(ms, 0);
}

TEST(stopwatch, time_passed_macro_multiple)
{
    hj::stopwatch sw;
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    auto ms1 = sw.elapsed_ms();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    auto ms2 = sw.elapsed_ms();
    ASSERT_GT(ms2, ms1);
}

TEST(stopwatch, monotonic_behavior)
{
    hj::stopwatch sw;
    auto          t1 = sw.elapsed_ns();
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    auto t2 = sw.elapsed_ns();
    ASSERT_GE(t2, t1);
}

TEST(stopwatch, multithread_independence)
{
    std::atomic<long long> ms_a{0};
    std::atomic<long long> ms_b{0};

    std::thread t1([&]() {
        hj::stopwatch sw;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        ms_a = sw.elapsed_ms();
    });

    std::thread t2([&]() {
        hj::stopwatch sw;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        ms_b = sw.elapsed_ms();
    });

    t1.join();
    t2.join();

    ASSERT_GE(ms_a, 5);
    ASSERT_GE(ms_b, 15);
}

TEST(stopwatch, multiple_instances_coexist)
{
    hj::stopwatch sw_a;
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    hj::stopwatch sw_b;
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    auto elapsed_a = sw_a.elapsed_ms();
    auto elapsed_b = sw_b.elapsed_ms();

    ASSERT_GT(elapsed_a, elapsed_b);
}