
#include <gtest/gtest.h>
#include <hj/time/date_time.hpp>

TEST(date_time, date_time)
{
    hj::date_time dt1;
    hj::date_time dt2(2023, 1, 1);
    hj::date_time dt3(2024, 1, 1);
    ASSERT_TRUE(dt1 != dt2);

    dt1 = dt2;
    ASSERT_TRUE(dt1 == dt2);

    ASSERT_TRUE(dt2 < dt3);
    ASSERT_TRUE(dt2 <= dt3);
    ASSERT_TRUE(dt3 > dt2);
    ASSERT_TRUE(dt3 >= dt2);
}

TEST(date_time, now)
{
    std::time_t tm = std::time(nullptr);
    std::tm     tm_buf;
#if defined(_WIN32)
    localtime_s(&tm_buf, &tm);
#else
    localtime_r(&tm, &tm_buf);
#endif
    ASSERT_EQ(hj::date_time::now(hj::timezone::UTC).time(), tm);

    ASSERT_TRUE(hj::date_time::now(hj::timezone::BEIJING).time()
                > hj::date_time::now(hj::timezone::UTC).time());
}

TEST(date_time, today)
{
    std::time_t tm = std::time(nullptr);
    std::tm     tm_buf;
#if defined(_WIN32)
    localtime_s(&tm_buf, &tm);
#else
    localtime_r(&tm, &tm_buf);
#endif

    hj::date_time today = hj::date_time::today();
    ASSERT_EQ(today.year(), tm_buf.tm_year + 1900);
    ASSERT_EQ(today.month(), static_cast<hj::moon>(tm_buf.tm_mon + 1));
    ASSERT_EQ(today.day(), tm_buf.tm_mday);
    ASSERT_EQ(today.hour(), 0);
    ASSERT_EQ(today.minute(), 0);
    ASSERT_EQ(today.seconds(), 0);
}

TEST(date_time, is_bigger_smaller_equal)
{
    hj::date_time d1(2023, 1, 1, 0, 0, 0);
    hj::date_time d2(2023, 1, 2, 0, 0, 0);
    ASSERT_TRUE(d2.is_bigger(d1));
    ASSERT_TRUE(d1.is_smaller(d2));
    ASSERT_TRUE(d1.is_equal(hj::date_time(2023, 1, 1, 0, 0, 0)));
}

TEST(date_time, day_of_week_and_str)
{
    hj::date_time d(2023, 1, 31, 0, 0, 0);
    ASSERT_EQ(d.day_of_week(), hj::weekday::tuesday);
    ASSERT_EQ(d.day_of_week_str(), "Tue");
}

TEST(date_time, month_and_str)
{
    hj::date_time d(2023, 2, 1, 0, 0, 0);
    ASSERT_EQ(d.month(), hj::moon::february);
    ASSERT_EQ(d.month_str(), "February");
}

TEST(date_time, day_of_month_and_year)
{
    hj::date_time d(2023, 2, 1, 0, 0, 0);
    ASSERT_EQ(d.day_of_month(), 1);
    ASSERT_EQ(d.day_of_year(), 32);
    ASSERT_EQ(d.year(), 2023);
}

TEST(date_time, start_end_of_half_year)
{
    hj::date_time d1(2023, 3, 15);
    hj::date_time d2(2023, 9, 15);
    ASSERT_EQ(d1.start_of_half_year().time(),
              hj::date_time(2023, 1, 1, 0, 0, 0).time());
    ASSERT_EQ(d2.start_of_half_year().time(),
              hj::date_time(2023, 7, 1, 0, 0, 0).time());
    ASSERT_EQ(d1.end_of_half_year().time(),
              hj::date_time(2023, 6, 30, 23, 59, 59).time());
    ASSERT_EQ(d2.end_of_half_year().time(),
              hj::date_time(2023, 12, 31, 23, 59, 59).time());
}

TEST(date_time, next_pre_month_quarter_half_year_year)
{
    hj::date_time d(2023, 2, 1);
    ASSERT_EQ(d.next_month().time(), hj::date_time(2023, 3, 1, 0, 0, 0).time());
    ASSERT_EQ(d.pre_month().time(), hj::date_time(2023, 1, 1, 0, 0, 0).time());
    ASSERT_EQ(d.next_quarter().time(),
              hj::date_time(2023, 4, 1, 0, 0, 0).time());
    ASSERT_EQ(d.pre_quarter().time(),
              hj::date_time(2022, 10, 1, 0, 0, 0).time());
    ASSERT_EQ(d.next_half_year().time(),
              hj::date_time(2023, 7, 1, 0, 0, 0).time());
    ASSERT_EQ(d.pre_half_year().time(),
              hj::date_time(2022, 7, 1, 0, 0, 0).time());
    ASSERT_EQ(d.next_year().time(), hj::date_time(2024, 1, 1, 0, 0, 0).time());
    ASSERT_EQ(d.pre_year().time(), hj::date_time(2022, 1, 1, 0, 0, 0).time());
}

TEST(date_time, timezone_now_today)
{
    auto utc_now   = hj::date_time::now(hj::timezone::UTC);
    auto local_now = hj::date_time::now(hj::timezone::LOCAL);
    auto bj_now    = hj::date_time::now(hj::timezone::BEIJING);
    ASSERT_TRUE(bj_now.time() > utc_now.time());
    auto utc_today   = hj::date_time::today(hj::timezone::UTC);
    auto local_today = hj::date_time::today(hj::timezone::LOCAL);
    auto bj_today    = hj::date_time::today(hj::timezone::BEIJING);
    ASSERT_TRUE(bj_today.time() > utc_today.time());
}

TEST(date_time, format)
{
    hj::date_time tm(2023, 1, 1, 0, 0, 0);
    ASSERT_STREQ(hj::date_time::format(tm, TIME_FMT).c_str(),
                 "2023-01-01 00:00:00");

    ASSERT_STREQ(hj::date_time::format(tm, "%Y%m%d-%H:%M:%S").c_str(),
                 "20230101-00:00:00");
}

TEST(date_time, parse)
{
    ASSERT_EQ(hj::date_time::parse("2023-01-01 00:00:00", TIME_FMT).time(),
              hj::date_time(2023, 1, 1, 0, 0, 0).time());

    hj::date_time dt;
    hj::date_time::parse(dt,
                         std::string("2023-01-01 00:00:00"),
                         std::string(TIME_FMT));
    ASSERT_EQ(dt.time(), hj::date_time(2023, 1, 1, 0, 0, 0).time());
}

TEST(date_time, is_null)
{
    ASSERT_FALSE(hj::date_time::now().is_null());
    ASSERT_TRUE(hj::date_time().is_null());
}

TEST(date_time, is_working_day)
{
    ASSERT_FALSE(hj::date_time(2025, 7, 12).is_working_day());
    ASSERT_TRUE(hj::date_time(2025, 7, 11).is_working_day());
}

TEST(date_time, is_weekend)
{
    ASSERT_TRUE(hj::date_time(2025, 7, 12).is_weekend());
    ASSERT_FALSE(hj::date_time(2025, 7, 11).is_weekend());
}

TEST(date_time, string)
{
    ASSERT_STREQ(hj::date_time(2023, 1, 1, 0, 0, 0).string().c_str(),
                 "2023-01-01 00:00:00");
}

TEST(date_time, date)
{
    hj::date_time dt(2023, 1, 1, 0, 0, 0);
    ASSERT_EQ(dt.date().tm_year + 1900, 2023);
    ASSERT_EQ(dt.date().tm_mon + 1, 1);
    ASSERT_EQ(dt.date().tm_mday, 1);
    ASSERT_EQ(dt.date().tm_hour, 0);
    ASSERT_EQ(dt.date().tm_min, 0);
    ASSERT_EQ(dt.date().tm_sec, 0);
}

TEST(date_time, time)
{
    std::time_t tm = std::time(nullptr);
    std::tm     tm_buf;
#if defined(_WIN32)
    localtime_s(&tm_buf, &tm);
#else
    localtime_r(&tm, &tm_buf);
#endif
    ASSERT_EQ(hj::date_time(tm).time(), tm);
}

TEST(date_time, seconds)
{
    ASSERT_EQ(hj::date_time(2023, 1, 1, 0, 0, 59).seconds(), 59);
}

TEST(date_time, minute)
{
    ASSERT_EQ(hj::date_time(2023, 1, 1, 0, 59, 59).minute(), 59);
}

TEST(date_time, minutes_to)
{
    ASSERT_EQ(hj::date_time(2023, 1, 1, 0, 59, 59)
                  .minutes_to(hj::date_time(2023, 1, 1, 1, 0, 0)),
              0);

    ASSERT_EQ(hj::date_time(2023, 1, 1, 0, 59, 0)
                  .minutes_to(hj::date_time(2023, 1, 1, 1, 0, 0)),
              1);
}

TEST(date_time, hour)
{
    ASSERT_EQ(hj::date_time(2023, 1, 1, 23, 59, 59).hour(), 23);
}

TEST(date_time, hours_to)
{
    ASSERT_EQ(hj::date_time(2023, 1, 1, 23, 59, 59)
                  .hours_to(hj::date_time(2023, 1, 2, 0, 0, 0)),
              0);

    ASSERT_EQ(hj::date_time(2023, 1, 1, 23, 0, 0)
                  .hours_to(hj::date_time(2023, 1, 2, 0, 0, 0)),
              1);
}

TEST(date_time, day)
{
    ASSERT_EQ(hj::date_time(2023, 1, 31, 0, 0, 0).day(), 31);
}

TEST(date_time, days_to)
{
    ASSERT_EQ(hj::date_time(2023, 1, 1, 23, 59, 59)
                  .days_to(hj::date_time(2023, 1, 2, 0, 0, 0)),
              0);

    ASSERT_EQ(hj::date_time(2023, 1, 1, 0, 0, 0)
                  .days_to(hj::date_time(2023, 1, 2, 0, 0, 0)),
              1);
}

TEST(date_time, day_of_week)
{
    ASSERT_TRUE(hj::date_time(2023, 1, 31, 0, 0, 0).day_of_week()
                == hj::weekday::tuesday);
}

TEST(date_time, day_of_week_str)
{
    ASSERT_TRUE(hj::date_time(2023, 1, 31, 0, 0, 0).day_of_week_str() == "Tue");
}

TEST(date_time, day_of_month)
{
    ASSERT_EQ(hj::date_time(2023, 2, 1, 0, 0, 0).day_of_month(), 1);
}

TEST(date_time, day_of_year)
{
    ASSERT_EQ(hj::date_time(2023, 2, 1, 0, 0, 0).day_of_year(), 32);
}

TEST(date_time, month)
{
    ASSERT_EQ(hj::date_time(2023, 1, 31, 0, 0, 0).month(),
              static_cast<hj::moon>(1));
}

TEST(date_time, month_str)
{
    ASSERT_TRUE(hj::date_time(2023, 1, 31, 0, 0, 0).month_str() == "January");
}

TEST(date_time, year)
{
    ASSERT_EQ(hj::date_time(2023, 1, 31, 0, 0, 0).year(), 2023);
}

TEST(date_time, start_of_day)
{
    ASSERT_EQ(hj::date_time(2023, 1, 1, 12, 1, 1).start_of_day().time(),
              hj::date_time(2023, 1, 1, 0, 0, 0).time());
}

TEST(date_time, end_of_day)
{
    ASSERT_EQ(hj::date_time(2023, 1, 1, 12, 1, 1).end_of_day().time(),
              hj::date_time(2023, 1, 1, 23, 59, 59).time());
}

TEST(date_time, start_of_week)
{
    ASSERT_EQ(hj::date_time(2023, 1, 31, 12, 1, 1).start_of_week().time(),
              hj::date_time(2023, 1, 30, 0, 0, 0).time());
}

TEST(date_time, end_of_week)
{
    ASSERT_EQ(hj::date_time(2023, 1, 31, 12, 0, 0).end_of_week().time(),
              hj::date_time(2023, 2, 5, 23, 59, 59).time());
}

TEST(date_time, start_of_month)
{
    ASSERT_EQ(hj::date_time(2023, 1, 31, 12, 1, 1).start_of_month().time(),
              hj::date_time(2023, 1, 1, 0, 0, 0).time());
}

TEST(date_time, end_of_month)
{
    ASSERT_EQ(hj::date_time(2023, 1, 30, 12, 1, 1).end_of_month().time(),
              hj::date_time(2023, 1, 31, 23, 59, 59).time());
}

TEST(date_time, start_of_quarter)
{
    ASSERT_EQ(hj::date_time(2023, 1, 30, 12, 1, 1).start_of_quarter().time(),
              hj::date_time(2023, 1, 1, 0, 0, 0).time());
}

TEST(date_time, end_of_quarter)
{
    ASSERT_EQ(hj::date_time(2023, 1, 30, 12, 1, 1).end_of_quarter().time(),
              hj::date_time(2023, 3, 31, 23, 59, 59).time());
}

TEST(date_time, start_of_half_year)
{
    ASSERT_EQ(hj::date_time(2023, 7, 1, 12, 1, 1).start_of_half_year().time(),
              hj::date_time(2023, 7, 1, 0, 0, 0).time());
}

TEST(date_time, end_of_half_year)
{
    ASSERT_EQ(hj::date_time(2023, 7, 1, 12, 1, 1).end_of_half_year().time(),
              hj::date_time(2023, 12, 31, 23, 59, 59).time());
}

TEST(date_time, start_of_year)
{
    ASSERT_EQ(hj::date_time(2023, 1, 31, 12, 1, 1).start_of_year().time(),
              hj::date_time(2023, 1, 1, 0, 0, 0).time());
}

TEST(date_time, end_of_year)
{
    ASSERT_EQ(hj::date_time(2023, 1, 31, 12, 1, 1).end_of_year().time(),
              hj::date_time(2023, 12, 31, 23, 59, 59).time());
}

TEST(date_time, next_day)
{
    ASSERT_EQ(hj::date_time(2023, 1, 31, 12, 1, 1).next_day().time(),
              hj::date_time(2023, 2, 1, 0, 0, 0).time());

    ASSERT_EQ(hj::date_time(2023, 1, 31, 12, 1, 1).next_day(3).time(),
              hj::date_time(2023, 2, 3, 0, 0, 0).time());
}

TEST(date_time, pre_day)
{
    ASSERT_EQ(hj::date_time(2023, 2, 1, 12, 1, 1).pre_day().time(),
              hj::date_time(2023, 1, 31, 0, 0, 0).time());

    ASSERT_EQ(hj::date_time(2023, 2, 1, 12, 1, 1).pre_day(5).time(),
              hj::date_time(2023, 1, 27, 0, 0, 0).time());
}

TEST(date_time, next_weekday)
{
    ASSERT_EQ(hj::date_time(2025, 7, 12, 0, 0, 0)
                  .next_weekday(hj::weekday::monday)
                  .time(),
              hj::date_time(2025, 7, 14, 0, 0, 0).time());

    ASSERT_EQ(hj::date_time(2025, 7, 12, 0, 0, 0)
                  .next_weekday(hj::weekday::friday)
                  .time(),
              hj::date_time(2025, 7, 18, 0, 0, 0).time());
}

TEST(date_time, pre_weekday)
{
    ASSERT_EQ(hj::date_time(2025, 7, 12, 0, 0, 0)
                  .pre_weekday(hj::weekday::monday)
                  .time(),
              hj::date_time(2025, 7, 7, 0, 0, 0).time());

    ASSERT_EQ(hj::date_time(2025, 7, 12, 0, 0, 0)
                  .pre_weekday(hj::weekday::friday)
                  .time(),
              hj::date_time(2025, 7, 11, 0, 0, 0).time());
}

TEST(date_time, next_working_day)
{
    ASSERT_EQ(hj::date_time(2025, 7, 11).next_working_day().time(),
              hj::date_time(2025, 7, 14, 0, 0, 0).time());

    ASSERT_EQ(hj::date_time(2025, 7, 11).next_working_day(5).time(),
              hj::date_time(2025, 7, 18, 0, 0, 0).time());

    ASSERT_EQ(hj::date_time(2025, 7, 11).next_working_day(6).time(),
              hj::date_time(2025, 7, 21, 0, 0, 0).time());
}

TEST(date_time, pre_working_day)
{
    ASSERT_EQ(hj::date_time(2025, 7, 11).pre_working_day().time(),
              hj::date_time(2025, 7, 10, 0, 0, 0).time());

    ASSERT_EQ(hj::date_time(2025, 7, 11).pre_working_day(5).time(),
              hj::date_time(2025, 7, 4, 0, 0, 0).time());

    ASSERT_EQ(hj::date_time(2025, 7, 11).pre_working_day(10).time(),
              hj::date_time(2025, 6, 27, 0, 0, 0).time());
}

TEST(date_time, next_month)
{
    ASSERT_EQ(hj::date_time(2023, 2, 1, 12, 1, 1).next_month().time(),
              hj::date_time(2023, 3, 1, 0, 0, 0).time());

    ASSERT_EQ(hj::date_time(2023, 2, 1, 12, 1, 1).next_month(3).time(),
              hj::date_time(2023, 5, 1, 0, 0, 0).time());
}

TEST(date_time, pre_month)
{
    ASSERT_EQ(hj::date_time(2023, 2, 1, 12, 1, 1).pre_month().time(),
              hj::date_time(2023, 1, 1, 0, 0, 0).time());

    ASSERT_EQ(hj::date_time(2023, 2, 1, 12, 1, 1).pre_month(3).time(),
              hj::date_time(2022, 11, 1, 0, 0, 0).time());
}

TEST(date_time, next_quarter)
{
    ASSERT_EQ(hj::date_time(2023, 2, 1, 12, 1, 1).next_quarter().time(),
              hj::date_time(2023, 4, 1, 0, 0, 0).time());

    ASSERT_EQ(hj::date_time(2023, 2, 1, 12, 1, 1).next_quarter(2).time(),
              hj::date_time(2023, 7, 1, 0, 0, 0).time());

    ASSERT_EQ(hj::date_time(2023, 2, 15).next_quarter().time(),
              hj::date_time(2023, 4, 1, 0, 0, 0).time());

    ASSERT_EQ(hj::date_time(2023, 2, 15).next_quarter(2).time(),
              hj::date_time(2023, 7, 1, 0, 0, 0).time());

    ASSERT_EQ(hj::date_time(2023, 11, 15).next_quarter().time(),
              hj::date_time(2024, 1, 1, 0, 0, 0).time());
}

TEST(date_time, pre_quarter)
{
    ASSERT_EQ(hj::date_time(2023, 2, 1, 12, 1, 1).pre_quarter().time(),
              hj::date_time(2022, 10, 1, 0, 0, 0).time());

    ASSERT_EQ(hj::date_time(2023, 2, 1, 12, 1, 1).pre_quarter(2).time(),
              hj::date_time(2022, 7, 1, 0, 0, 0).time());

    ASSERT_EQ(hj::date_time(2023, 5, 15).pre_quarter().time(),
              hj::date_time(2023, 1, 1, 0, 0, 0).time());

    ASSERT_EQ(hj::date_time(2023, 2, 15).pre_quarter().time(),
              hj::date_time(2022, 10, 1, 0, 0, 0).time());
}

TEST(date_time, next_half_year)
{
    ASSERT_EQ(hj::date_time(2023, 2, 1, 12, 1, 1).next_half_year().time(),
              hj::date_time(2023, 7, 1, 0, 0, 0).time());

    ASSERT_EQ(hj::date_time(2023, 2, 1, 12, 1, 1).next_half_year(3).time(),
              hj::date_time(2024, 7, 1, 0, 0, 0).time());

    ASSERT_EQ(hj::date_time(2023, 3, 15).next_half_year().time(),
              hj::date_time(2023, 7, 1, 0, 0, 0).time());

    ASSERT_EQ(hj::date_time(2023, 9, 15).next_half_year().time(),
              hj::date_time(2024, 1, 1, 0, 0, 0).time());
}

TEST(date_time, pre_half_year)
{
    ASSERT_EQ(hj::date_time(2023, 9, 15).pre_half_year().time(),
              hj::date_time(2023, 1, 1, 0, 0, 0).time());

    ASSERT_EQ(hj::date_time(2023, 3, 15).pre_half_year().time(),
              hj::date_time(2022, 7, 1, 0, 0, 0).time());
}

TEST(date_time, next_year)
{
    ASSERT_EQ(hj::date_time(2023, 2, 1, 12, 1, 1).next_year().time(),
              hj::date_time(2024, 1, 1, 0, 0, 0).time());

    ASSERT_EQ(hj::date_time(2023, 2, 1, 12, 1, 1).next_year(2).time(),
              hj::date_time(2025, 1, 1, 0, 0, 0).time());

    ASSERT_EQ(hj::date_time(2023, 6, 15, 12, 30, 45).next_year().time(),
              hj::date_time(2024, 1, 1, 0, 0, 0).time());

    ASSERT_EQ(hj::date_time(2023, 6, 15).next_year(3).time(),
              hj::date_time(2026, 1, 1, 0, 0, 0).time());
}

TEST(date_time, pre_year)
{
    ASSERT_EQ(hj::date_time(2023, 2, 1, 12, 1, 1).pre_year().time(),
              hj::date_time(2022, 1, 1, 0, 0, 0).time());

    ASSERT_EQ(hj::date_time(2023, 2, 1, 12, 1, 1).pre_year(2).time(),
              hj::date_time(2021, 1, 1, 0, 0, 0).time());

    ASSERT_EQ(hj::date_time(2023, 6, 15, 12, 30, 45).pre_year().time(),
              hj::date_time(2022, 1, 1, 0, 0, 0).time());

    ASSERT_EQ(hj::date_time(2023, 6, 15).pre_year(2).time(),
              hj::date_time(2021, 1, 1, 0, 0, 0).time());
}