#include <thread>

#include <gtest/gtest.h>
#include <libcpp/time/duration.hpp>

TEST(duration, time_passed)
{
    TIME_START();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ASSERT_EQ(TIME_PASSED_MS() > 0 && TIME_PASSED_MS() < 30, true);
    ASSERT_EQ(TIME_PASSED_NS() > 0 && TIME_PASSED_NS() < 30000000, true);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    TIME_START();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    // ASSERT_EQ(TIME_PASSED_MS() > 0 && TIME_PASSED_MS() < 30, true);
    // ASSERT_EQ(TIME_PASSED_NS() > 0 && TIME_PASSED_NS() < 30000000, true);
}
