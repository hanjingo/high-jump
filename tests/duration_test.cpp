#include <gtest/gtest.h>
#include <hj/time/duration.hpp>
#include <thread>

TEST(duration, time_passed_basic)
{
    TIME_START();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    auto ms  = TIME_PASSED_MS();
    auto ns  = TIME_PASSED_NS();
    auto sec = TIME_PASSED_SEC();
    ASSERT_GT(ms, 0);
    ASSERT_GT(ns, 0);
    ASSERT_GE(ns, ms * 1000000);
    ASSERT_EQ(sec, 0); // 2ms < 1s
}

TEST(duration, time_start_reset)
{
    TIME_START();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    auto ms1 = TIME_PASSED_MS();
    ASSERT_GT(ms1, 0);
    TIME_START();
    auto ms2 = TIME_PASSED_MS();
    ASSERT_LT(ms2, ms1);
}

TEST(duration, time_passed_duration_type)
{
    TIME_START();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    auto dur = TIME_PASSED();
    auto ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
    ASSERT_GT(ms, 0);
}

TEST(duration, time_passed_macro_multiple)
{
    TIME_START();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    auto ms1 = TIME_PASSED_MS();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    auto ms2 = TIME_PASSED_MS();
    ASSERT_GT(ms2, ms1);
}
