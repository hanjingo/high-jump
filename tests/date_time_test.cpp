#include <gtest/gtest.h>
#include <libcpp/time/date_time.hpp>

TEST(date_time, date_time)
{
    libcpp::date_time dt1();

    libcpp::date_time dt2(2023, 1, 1);
    ASSERT_EQ(dt2.is_null(), false);
}

TEST(date_time, now)
{
    std::time_t tm = std::time(nullptr);
    std::localtime(&tm);
    ASSERT_EQ(libcpp::date_time::now().time(), tm);
}

TEST(date_time, today)
{
    std::time_t tm = std::time(nullptr);
    std::localtime(&tm);
}

TEST(date_time, format)
{
    libcpp::date_time tm(2023, 1, 1, 0, 0, 0);
    ASSERT_STREQ(
        libcpp::date_time::format(tm, TIME_FMT).c_str(), 
        "2023-01-01 00:00:00"
    );

    ASSERT_STREQ(
        libcpp::date_time::format(tm, "%Y%m%d-%H:%M:%S").c_str(), 
        "20230101-00:00:00"
    );
}

TEST(date_time, parse)
{
    ASSERT_EQ(
        libcpp::date_time::parse("2023-01-01 00:00:00", TIME_FMT).time(), libcpp::date_time(2023, 1, 1, 0, 0, 0).time()
    );

    libcpp::date_time dt;
    libcpp::date_time::parse(dt, std::string("2023-01-01 00:00:00"), std::string(TIME_FMT));
    ASSERT_EQ(dt.time(), libcpp::date_time(2023, 1, 1, 0, 0, 0).time());
}

TEST(date_time, is_null)
{
    ASSERT_EQ(libcpp::date_time::now().is_null(), false);
    ASSERT_EQ(libcpp::date_time().is_null(), true);
}

TEST(date_time, string)
{
    ASSERT_STREQ(
        libcpp::date_time(2023, 1, 1, 0, 0, 0).string().c_str(), 
        "2023-01-01 00:00:00"
    );
}

TEST(date_time, date)
{
}

TEST(date_time, time)
{
    std::time_t tm = std::time(nullptr);
    std::localtime(&tm);
    ASSERT_EQ(libcpp::date_time(tm).time(), tm);
}

TEST(date_time, seconds)
{
    ASSERT_EQ(libcpp::date_time(2023, 1, 1, 0, 0, 59).seconds(), 59);
}

TEST(date_time, minute)
{
    ASSERT_EQ(libcpp::date_time(2023, 1, 1, 0, 59, 59).minute(), 59);
}

TEST(date_time, minutes_to)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 1, 1, 0, 59, 59).minutes_to(
            libcpp::date_time(2023, 1, 1, 1, 0, 0)), 
        0
    );

    ASSERT_EQ(
        libcpp::date_time(2023, 1, 1, 0, 59, 0).minutes_to(
            libcpp::date_time(2023, 1, 1, 1, 0, 0)), 
        1
    );
}

TEST(date_time, hour)
{
    ASSERT_EQ(libcpp::date_time(2023, 1, 1, 23, 59, 59).hour(), 23);
}

TEST(date_time, hours_to)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 1, 1, 23, 59, 59).hours_to(
            libcpp::date_time(2023, 1, 2, 0, 0, 0)), 
        0
    );

    ASSERT_EQ(
        libcpp::date_time(2023, 1, 1, 23, 0, 0).hours_to(
            libcpp::date_time(2023, 1, 2, 0, 0, 0)), 
        1
    );
}

TEST(date_time, day)
{
    ASSERT_EQ(libcpp::date_time(2023, 1, 31, 0, 0, 0).day(), 31);
}

TEST(date_time, days_to)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 1, 1, 23, 59, 59).days_to(
            libcpp::date_time(2023, 1, 2, 0, 0, 0)), 
        0
    );

    ASSERT_EQ(
        libcpp::date_time(2023, 1, 1, 0, 0, 0).days_to(
            libcpp::date_time(2023, 1, 2, 0, 0, 0)), 
        1
    );
}

TEST(date_time, day_of_week)
{
    ASSERT_EQ(libcpp::date_time(2023, 1, 31, 0, 0, 0).day_of_week(), libcpp::tuesday);
}

TEST(date_time, day_of_month)
{
    ASSERT_EQ(libcpp::date_time(2023, 2, 1, 0, 0, 0).day_of_month(), 1);
}

TEST(date_time, day_of_year)
{
    ASSERT_EQ(libcpp::date_time(2023, 2, 1, 0, 0, 0).day_of_year(), 32);
}

TEST(date_time, month)
{
    ASSERT_EQ(libcpp::date_time(2023, 1, 31, 0, 0, 0).month(), 1);
}

TEST(date_time, year)
{
    ASSERT_EQ(libcpp::date_time(2023, 1, 31, 0, 0, 0).year(), 2023);
}

TEST(date_time, start_of_day)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 1, 1, 12, 1, 1).start_of_day().time(), 
        libcpp::date_time(2023, 1, 1, 0, 0, 0).time()
    );
}

TEST(date_time, end_of_day)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 1, 1, 12, 1, 1).end_of_day().time(), 
        libcpp::date_time(2023, 1, 1, 23, 59, 59).time()
    );
}

TEST(date_time, start_of_week)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 1, 31, 12, 1, 1).start_of_week().time(), 
        libcpp::date_time(2023, 1, 30, 0, 0, 0).time()
    );
}

TEST(date_time, end_of_week)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 1, 31, 12, 0, 0).end_of_week().time(), 
        libcpp::date_time(2023, 2, 5, 23, 59, 59).time()
    );
}

TEST(date_time, start_of_month)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 1, 31, 12, 1, 1).start_of_month().time(), 
        libcpp::date_time(2023, 1, 1, 0, 0, 0).time()
    );
}

TEST(date_time, end_of_month)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 1, 30, 12, 1, 1).end_of_month().time(), 
        libcpp::date_time(2023, 1, 31, 23, 59, 59).time()
    );
}

TEST(date_time, start_of_quarter)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 1, 30, 12, 1, 1).start_of_quarter().time(), 
        libcpp::date_time(2023, 1, 1, 0, 0, 0).time()
    );
}

TEST(date_time, end_of_quarter)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 1, 30, 12, 1, 1).end_of_quarter().time(), 
        libcpp::date_time(2023, 3, 31, 23, 59, 59).time()
    );
}

TEST(date_time, start_of_half_year)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 7, 1, 12, 1, 1).start_of_half_year().time(), 
        libcpp::date_time(2023, 7, 1, 0, 0, 0).time()
    );
}

TEST(date_time, end_of_half_year)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 7, 1, 12, 1, 1).end_of_half_year().time(), 
        libcpp::date_time(2023, 12, 31, 23, 59, 59).time()
    );
}

TEST(date_time, start_of_year)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 1, 31, 12, 1, 1).start_of_year().time(), 
        libcpp::date_time(2023, 1, 1, 0, 0, 0).time()
    );
}

TEST(date_time, end_of_year)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 1, 31, 12, 1, 1).end_of_year().time(), 
        libcpp::date_time(2023, 12, 31, 23, 59, 59).time()
    );
}

TEST(date_time, next_day)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 1, 31, 12, 1, 1).next_day().time(), 
        libcpp::date_time(2023, 2, 1, 0, 0, 0).time()
    );
}

TEST(date_time, next_day_n)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 1, 31, 12, 1, 1).next_day_n(3).time(), 
        libcpp::date_time(2023, 2, 3, 0, 0, 0).time()
    );
}

TEST(date_time, pre_day)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 2, 1, 12, 1, 1).pre_day().time(), 
        libcpp::date_time(2023, 1, 31, 0, 0, 0).time()
    );
}

TEST(date_time, pre_day_n)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 2, 1, 12, 1, 1).pre_day_n(5).time(), 
        libcpp::date_time(2023, 1, 27, 0, 0, 0).time()
    );
}

TEST(date_time, next_month)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 2, 1, 12, 1, 1).next_month().time(), 
        libcpp::date_time(2023, 3, 1, 0, 0, 0).time()
    );
}

TEST(date_time, next_month_n)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 2, 1, 12, 1, 1).next_month_n(3).time(), 
        libcpp::date_time(2023, 5, 1, 0, 0, 0).time()
    );
}

TEST(date_time, pre_month)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 2, 1, 12, 1, 1).pre_month().time(), 
        libcpp::date_time(2023, 1, 1, 0, 0, 0).time()
    );
}

TEST(date_time, pre_month_n)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 2, 1, 12, 1, 1).pre_month_n(3).time(), 
        libcpp::date_time(2022, 11, 1, 0, 0, 0).time()
    );
}

TEST(date_time, next_quarter)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 2, 1, 12, 1, 1).next_quarter().time(), 
        libcpp::date_time(2023, 4, 1, 0, 0, 0).time()
    );
}

TEST(date_time, next_quarter_n)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 2, 1, 12, 1, 1).next_quarter_n(2).time(), 
        libcpp::date_time(2023, 7, 1, 0, 0, 0).time()
    );
}

TEST(date_time, pre_quarter)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 2, 1, 12, 1, 1).pre_quarter().time(), 
        libcpp::date_time(2022, 10, 1, 0, 0, 0).time()
    );
}

TEST(date_time, pre_quarter_n)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 2, 1, 12, 1, 1).pre_quarter_n(2).time(), 
        libcpp::date_time(2022, 7, 1, 0, 0, 0).time()
    );
}

TEST(date_time, next_half_year)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 2, 1, 12, 1, 1).next_half_year().time(), 
        libcpp::date_time(2023, 7, 1, 0, 0, 0).time()
    );
}

TEST(date_time, next_halfe_year_n)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 2, 1, 12, 1, 1).next_half_year_n(3).time(), 
        libcpp::date_time(2024, 7, 1, 0, 0, 0).time()
    );
}

TEST(date_time, next_year)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 2, 1, 12, 1, 1).next_year().time(), 
        libcpp::date_time(2024, 1, 1, 0, 0, 0).time()
    );
}

TEST(date_time, next_year_n)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 2, 1, 12, 1, 1).next_year_n(2).time(), 
        libcpp::date_time(2025, 1, 1, 0, 0, 0).time()
    );
}

TEST(date_time, pre_year)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 2, 1, 12, 1, 1).pre_year().time(), 
        libcpp::date_time(2022, 1, 1, 0, 0, 0).time()
    );
}

TEST(date_time, pre_year_n)
{
    ASSERT_EQ(
        libcpp::date_time(2023, 2, 1, 12, 1, 1).pre_year_n(2).time(), 
        libcpp::date_time(2021, 1, 1, 0, 0, 0).time()
    );
}