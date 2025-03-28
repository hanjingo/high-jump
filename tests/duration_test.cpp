#include <gtest/gtest.h>
#include <libcpp/time/duration.hpp>
#include <thread>

TEST(duration, time_passed)
{
    TIME_START();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ASSERT_EQ(TIME_PASSED_MS() > 0 && TIME_PASSED_MS() < 20, true);
    ASSERT_EQ(TIME_PASSED_NS() > 0 && TIME_PASSED_NS() < 20000000, true);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    TIME_START();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ASSERT_EQ(TIME_PASSED_MS() > 0 && TIME_PASSED_MS() < 20, true);
    ASSERT_EQ(TIME_PASSED_NS() > 0 && TIME_PASSED_NS() < 20000000, true);

    auto start = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::milliseconds(500);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::this_thread::sleep_for(dur - std::chrono::milliseconds(100));
}
