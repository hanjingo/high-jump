#define COMPILE_TIME_FMT "%d %d %d %d %d %d"
#define COMPILE_TIME_LEN 22

#include <string>

#include <gtest/gtest.h>
#include <libcpp/os/env.h>

TEST (env, compile_time)
{
    ASSERT_EQ (COMPILE_YEAR > 0, true);
    ASSERT_EQ (COMPILE_MONTH > 0, true);
    ASSERT_EQ (COMPILE_DAY > 0, true);
    ASSERT_EQ (COMPILE_HOUR >= 0, true);
    ASSERT_EQ (COMPILE_MINUTE >= 0, true);
    ASSERT_EQ (COMPILE_SECOND >= 0, true);

    std::string str;
    str.assign (COMPILE_TIME, COMPILE_TIME_LEN);
    ASSERT_EQ (str.size () == COMPILE_TIME_LEN, true);
    ASSERT_EQ (!str.empty (), true);
}