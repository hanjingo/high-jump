#include <gtest/gtest.h>

#include <chrono>
#include <ctime>
#include <string>

#include <hj/time/date_time.hpp>

namespace
{

hj::date_time make_utc(int year,
                       int month,
                       int day,
                       int hour   = 0,
                       int minute = 0,
                       int second = 0,
                       int ms     = 0)
{
    return hj::date_time(static_cast<unsigned short>(year),
                         static_cast<unsigned short>(month),
                         static_cast<unsigned short>(day),
                         hour,
                         minute,
                         second,
                         ms);
}

} // namespace

TEST(date_time, rule_of_zero_and_comparison)
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

TEST(date_time, const_correctness)
{
    const hj::date_time dt(2023, 1, 31, 23, 59, 59);

    ASSERT_EQ(dt.year(), 2023);
    ASSERT_EQ(dt.month(), hj::month::january);
    ASSERT_EQ(dt.day(), 31);
    ASSERT_EQ(dt.hour(), 23);
    ASSERT_EQ(dt.minute(), 59);
    ASSERT_EQ(dt.seconds(), 59);
    ASSERT_EQ(dt.day_of_week(), hj::weekday::tuesday);
    ASSERT_EQ(dt.string(), "2023-01-31 23:59:59");
}

TEST(date_time, month_name)
{
    const hj::date_time dt(2023, 2, 1);

    ASSERT_EQ(dt.month(), hj::month::february);
    ASSERT_EQ(dt.month_str(), "February");
}

TEST(date_time, now_utc_preserves_epoch)
{
    const auto now = hj::date_time::now(hj::timezone::UTC);

    const auto system_now = std::time(nullptr);
    const auto actual     = now.time();

    ASSERT_LE(std::llabs(static_cast<long long>(actual)
                         - static_cast<long long>(system_now)),
              2);
    ASSERT_EQ(now.timezone_offset().total_seconds(), 0);
}

TEST(date_time, fixed_timezone_preserves_instant)
{
    const auto utc = make_utc(2026, 8, 22, 2, 0, 0);
    const auto bj  = utc.to_timezone(hj::timezone::BEIJING);
    const auto ny  = utc.to_timezone(hj::timezone::NEW_YORK);

    ASSERT_EQ(utc.sec_since_epoch(), bj.sec_since_epoch());
    ASSERT_EQ(utc.sec_since_epoch(), ny.sec_since_epoch());

    ASSERT_EQ(bj.year(), 2026);
    ASSERT_EQ(bj.month(), hj::month::august);
    ASSERT_EQ(bj.day(), 22);
    ASSERT_EQ(bj.hour(), 10);

    ASSERT_EQ(ny.year(), 2026);
    ASSERT_EQ(ny.month(), hj::month::august);
    ASSERT_EQ(ny.day(), 21);
    ASSERT_EQ(ny.hour(), 21);
}

TEST(date_time, fixed_timezone_time_conversion)
{
    const auto utc = make_utc(2026, 8, 22, 2, 0, 0);
    const auto bj  = utc.to_timezone(hj::timezone::BEIJING);

    ASSERT_EQ(utc.time(), bj.time());
    ASSERT_EQ(utc.ms_since_epoch(), bj.ms_since_epoch());
    ASSERT_EQ(bj.timezone_offset().total_seconds(), 8 * 3600);
}

TEST(date_time, fixed_timezone_comparison_uses_instant)
{
    const auto utc = make_utc(2026, 8, 22, 2, 0, 0);
    const auto bj  = utc.to_timezone(hj::timezone::BEIJING);

    ASSERT_EQ(utc, bj);
    ASSERT_FALSE(utc < bj);
    ASSERT_FALSE(utc > bj);
}

TEST(date_time, today_fixed_timezone)
{
    const auto utc_midnight = make_utc(2026, 8, 22, 0, 0, 0);
    const auto bj           = utc_midnight.to_timezone(hj::timezone::BEIJING);

    const auto bj_today = bj.start_of_day();

    ASSERT_EQ(bj_today.year(), 2026);
    ASSERT_EQ(bj_today.month(), hj::month::august);
    ASSERT_EQ(bj_today.day(), 22);
    ASSERT_EQ(bj_today.hour(), 0);
    ASSERT_EQ(bj_today.minute(), 0);
    ASSERT_EQ(bj_today.seconds(), 0);
    ASSERT_EQ(bj_today.timezone_offset().total_seconds(), 8 * 3600);
}

TEST(date_time, today_utc)
{
    const auto today = hj::date_time::today(hj::timezone::UTC);

    ASSERT_EQ(today.hour(), 0);
    ASSERT_EQ(today.minute(), 0);
    ASSERT_EQ(today.seconds(), 0);
    ASSERT_EQ(today.timezone_offset().total_seconds(), 0);
}

TEST(date_time, format)
{
    const hj::date_time dt(2023, 1, 1, 0, 0, 0);

    ASSERT_EQ(hj::date_time::format(dt, hj::date_time::time_fmt),
              "2023-01-01 00:00:00");

    ASSERT_EQ(hj::date_time::format(dt, "%Y%m%d-%H:%M:%S"),
              "20230101-00:00:00");
}

TEST(date_time, format_to)
{
    const hj::date_time dt(2023, 1, 1);

    char buffer[64]{};
    ASSERT_EQ(hj::date_time::format(dt, buffer, sizeof(buffer), "%Y-%m-%d"),
              10U);
    ASSERT_STREQ(buffer, "2023-01-01");
}

TEST(date_time, format_failure)
{
    const hj::date_time dt(2023, 1, 1);

    char buffer[4]{'x', 'x', 'x', '\0'};
    ASSERT_EQ(hj::date_time::format(dt, buffer, sizeof(buffer), "%Y-%m-%d"),
              0U);
    ASSERT_EQ(buffer[0], '\0');
}

TEST(date_time, parse_success)
{
    const auto parsed =
        hj::date_time::parse("2023-01-01 00:00:00", hj::date_time::time_fmt);

    ASSERT_TRUE(parsed.has_value());
    ASSERT_EQ(parsed->time(), make_utc(2023, 1, 1, 0, 0, 0).time());
}

TEST(date_time, parse_rejects_invalid_input)
{
    ASSERT_FALSE(
        hj::date_time::parse("2023-99-99 00:00:00", hj::date_time::time_fmt)
            .has_value());

    ASSERT_FALSE(hj::date_time::parse("2023-01-01 00:00:00 trailing",
                                      hj::date_time::time_fmt)
                     .has_value());
}

TEST(date_time, parse_output_parameter_is_transactional)
{
    hj::date_time dt(2024, 1, 1);

    ASSERT_FALSE(hj::date_time::parse(dt, "invalid", hj::date_time::time_fmt));

    ASSERT_EQ(dt, make_utc(2024, 1, 1));

    ASSERT_TRUE(hj::date_time::parse(dt,
                                     "2023-01-01 00:00:00",
                                     hj::date_time::time_fmt));

    ASSERT_EQ(dt, make_utc(2023, 1, 1));
}

TEST(date_time, epoch_seconds_round_trip)
{
    const auto dt       = make_utc(2026, 6, 26, 12, 0, 0);
    const auto sec      = dt.sec_since_epoch();
    const auto restored = hj::date_time::from_sec_since_epoch(sec);

    ASSERT_EQ(restored, dt);
}

TEST(date_time, epoch_milliseconds_round_trip)
{
    const auto dt       = make_utc(2026, 6, 26, 12, 0, 0, 500);
    const auto ms       = dt.ms_since_epoch();
    const auto restored = hj::date_time::from_ms_since_epoch(ms);

    ASSERT_EQ(restored, dt);
    ASSERT_EQ(restored.milliseconds(), 500);
}

TEST(date_time, negative_epoch_round_trip)
{
    const auto dt = make_utc(1969, 12, 31, 23, 59, 59);
    ASSERT_EQ(dt.sec_since_epoch(), -1);
    ASSERT_EQ(hj::date_time::from_sec_since_epoch(-1), dt);
}

TEST(date_time, timezone_epoch_is_identical)
{
    const auto utc = make_utc(1970, 1, 1, 0, 0, 0);
    const auto bj  = utc.to_timezone(hj::timezone::BEIJING);

    ASSERT_EQ(utc.sec_since_epoch(), 0);
    ASSERT_EQ(bj.sec_since_epoch(), 0);
    ASSERT_EQ(bj.time(), 0);
    ASSERT_EQ(bj.hour(), 8);
    ASSERT_EQ(bj.day(), 1);
}

TEST(date_time, null_object_is_explicitly_invalid)
{
    const hj::date_time dt;

    ASSERT_TRUE(dt.is_null());
    ASSERT_EQ(dt.string(), "");
    ASSERT_EQ(hj::date_time::format(dt), "");

    ASSERT_THROW(dt.year(), std::logic_error);
    ASSERT_THROW(dt.month(), std::logic_error);
    ASSERT_THROW(dt.day(), std::logic_error);
    ASSERT_THROW(dt.time(), std::logic_error);
    ASSERT_THROW(dt.sec_since_epoch(), std::logic_error);
    ASSERT_THROW(dt.day_of_week(), std::logic_error);
    ASSERT_THROW(dt.is_weekend(), std::logic_error);
}

TEST(date_time, seconds_minutes_hours)
{
    const auto d1 = make_utc(2023, 1, 1, 0, 59, 59);
    const auto d2 = make_utc(2023, 1, 1, 1, 0, 0);

    ASSERT_EQ(d1.seconds(), 59);
    ASSERT_EQ(d1.minute(), 59);
    ASSERT_EQ(d1.hour(), 0);

    ASSERT_EQ(d1.seconds_to(d2), 1);
    ASSERT_EQ(d1.minutes_to(d2), 0);
    ASSERT_EQ(d1.hours_to(d2), 0);

    const auto d3 = make_utc(2023, 1, 1, 1, 0, 0);
    const auto d4 = make_utc(2023, 1, 1, 2, 0, 0);

    ASSERT_EQ(d3.minutes_to(d4), 60);
    ASSERT_EQ(d3.hours_to(d4), 1);
}

TEST(date_time, negative_duration_truncates_toward_zero)
{
    const auto later   = make_utc(2023, 1, 2, 0, 0, 0);
    const auto earlier = make_utc(2023, 1, 1, 23, 59, 59);

    ASSERT_EQ(later.days_to(earlier), 0);
    ASSERT_EQ(later.hours_to(earlier), 0);
    ASSERT_EQ(later.minutes_to(earlier), 0);
    ASSERT_EQ(later.seconds_to(earlier), -1);
}

TEST(date_time, leap_year)
{
    ASSERT_EQ(make_utc(2024, 2, 28).next_day().date().tm_mday, 29);
    ASSERT_EQ(make_utc(2024, 2, 29).next_day().date().tm_mday, 1);
    ASSERT_EQ(make_utc(2023, 2, 28).next_day().date().tm_mday, 1);

    ASSERT_EQ(make_utc(2024, 2, 29).day_of_year(), 60);
    ASSERT_EQ(make_utc(2023, 3, 1).day_of_year(), 60);
}

TEST(date_time, day_week_month_year)
{
    const hj::date_time d(2023, 1, 31, 12, 1, 1);

    ASSERT_EQ(d.day_of_week(), hj::weekday::tuesday);
    ASSERT_EQ(d.day_of_week_str(), "Tue");
    ASSERT_EQ(d.day_of_month(), 31);
    ASSERT_EQ(d.day_of_year(), 31);
    ASSERT_EQ(d.month(), hj::month::january);
    ASSERT_EQ(d.year(), 2023);
}

TEST(date_time, start_end_of_day)
{
    const auto d = make_utc(2023, 1, 1, 12, 1, 1);

    ASSERT_EQ(d.start_of_day().string(), "2023-01-01 00:00:00");
    ASSERT_EQ(d.end_of_day().string(), "2023-01-01 23:59:59");
}

TEST(date_time, start_end_of_week)
{
    const auto d = make_utc(2023, 1, 31, 12, 1, 1);

    ASSERT_EQ(d.start_of_week().string(), "2023-01-30 00:00:00");
    ASSERT_EQ(d.end_of_week().string(), "2023-02-05 23:59:59");
}

TEST(date_time, start_end_of_month)
{
    const auto d = make_utc(2023, 1, 30, 12, 1, 1);

    ASSERT_EQ(d.start_of_month().string(), "2023-01-01 00:00:00");
    ASSERT_EQ(d.end_of_month().string(), "2023-01-31 23:59:59");
}

TEST(date_time, start_end_of_quarter)
{
    const auto d = make_utc(2023, 5, 30, 12, 1, 1);

    ASSERT_EQ(d.start_of_quarter().string(), "2023-04-01 00:00:00");
    ASSERT_EQ(d.end_of_quarter().string(), "2023-06-30 23:59:59");
}

TEST(date_time, start_end_of_half_year)
{
    const auto first  = make_utc(2023, 3, 15);
    const auto second = make_utc(2023, 9, 15);

    ASSERT_EQ(first.start_of_half_year().string(), "2023-01-01 00:00:00");
    ASSERT_EQ(first.end_of_half_year().string(), "2023-06-30 23:59:59");

    ASSERT_EQ(second.start_of_half_year().string(), "2023-07-01 00:00:00");
    ASSERT_EQ(second.end_of_half_year().string(), "2023-12-31 23:59:59");
}

TEST(date_time, start_end_of_year)
{
    const auto d = make_utc(2023, 6, 15);

    ASSERT_EQ(d.start_of_year().string(), "2023-01-01 00:00:00");
    ASSERT_EQ(d.end_of_year().string(), "2023-12-31 23:59:59");
}

TEST(date_time, next_pre_day)
{
    const auto d = make_utc(2023, 1, 31, 12, 1, 1);

    ASSERT_EQ(d.next_day().string(), "2023-02-01 00:00:00");
    ASSERT_EQ(d.next_day(3).string(), "2023-02-03 00:00:00");

    const auto p = make_utc(2023, 2, 1, 12, 1, 1);
    ASSERT_EQ(p.pre_day().string(), "2023-01-31 00:00:00");
    ASSERT_EQ(p.pre_day(5).string(), "2023-01-27 00:00:00");
}

TEST(date_time, next_pre_weekday)
{
    const auto saturday = make_utc(2025, 7, 12);

    ASSERT_EQ(saturday.next_weekday(hj::weekday::monday).string(),
              "2025-07-14 00:00:00");

    ASSERT_EQ(saturday.next_weekday(hj::weekday::friday).string(),
              "2025-07-18 00:00:00");

    ASSERT_EQ(saturday.pre_weekday(hj::weekday::monday).string(),
              "2025-07-07 00:00:00");

    ASSERT_EQ(saturday.pre_weekday(hj::weekday::friday).string(),
              "2025-07-11 00:00:00");
}

TEST(date_time, next_pre_month)
{
    const auto d = make_utc(2023, 2, 1, 12, 1, 1);

    ASSERT_EQ(d.next_month().string(), "2023-03-01 00:00:00");
    ASSERT_EQ(d.next_month(3).string(), "2023-05-01 00:00:00");

    ASSERT_EQ(d.pre_month().string(), "2023-01-01 00:00:00");
    ASSERT_EQ(d.pre_month(3).string(), "2022-11-01 00:00:00");
}

TEST(date_time, next_pre_quarter)
{
    const auto d = make_utc(2023, 2, 15);

    ASSERT_EQ(d.next_quarter().string(), "2023-04-01 00:00:00");
    ASSERT_EQ(d.next_quarter(2).string(), "2023-07-01 00:00:00");
    ASSERT_EQ(d.pre_quarter().string(), "2022-10-01 00:00:00");
    ASSERT_EQ(d.pre_quarter(2).string(), "2022-07-01 00:00:00");

    ASSERT_EQ(make_utc(2023, 11, 15).next_quarter().string(),
              "2024-01-01 00:00:00");
}

TEST(date_time, next_pre_half_year)
{
    const auto d = make_utc(2023, 2, 1);

    ASSERT_EQ(d.next_half_year().string(), "2023-07-01 00:00:00");
    ASSERT_EQ(d.next_half_year(3).string(), "2024-07-01 00:00:00");

    ASSERT_EQ(d.pre_half_year().string(), "2022-07-01 00:00:00");
    ASSERT_EQ(d.pre_half_year(2).string(), "2022-01-01 00:00:00");

    ASSERT_EQ(make_utc(2023, 9, 15).next_half_year().string(),
              "2024-01-01 00:00:00");
}

TEST(date_time, next_pre_year)
{
    const auto d = make_utc(2023, 6, 15, 12, 30, 45);

    ASSERT_EQ(d.next_year().string(), "2024-01-01 00:00:00");
    ASSERT_EQ(d.next_year(2).string(), "2025-01-01 00:00:00");
    ASSERT_EQ(d.pre_year().string(), "2022-01-01 00:00:00");
    ASSERT_EQ(d.pre_year(2).string(), "2021-01-01 00:00:00");
}

TEST(date_time, working_day_and_weekend)
{
    const auto friday   = make_utc(2025, 7, 11);
    const auto saturday = make_utc(2025, 7, 12);
    const auto sunday   = make_utc(2025, 7, 13);

    ASSERT_TRUE(friday.is_working_day());
    ASSERT_FALSE(saturday.is_working_day());
    ASSERT_FALSE(sunday.is_working_day());

    ASSERT_FALSE(friday.is_weekend());
    ASSERT_TRUE(saturday.is_weekend());
    ASSERT_TRUE(sunday.is_weekend());
}

TEST(date_time, custom_working_day_policy)
{
    const auto                             saturday = make_utc(2025, 7, 12);
    const hj::date_time::is_working_day_fn policy =
        [](const hj::date_time &dt) { return dt.day() == 12; };

    ASSERT_TRUE(saturday.is_working_day(policy));
    ASSERT_TRUE(saturday.next_working_day(1, policy).day() == 12);
}

TEST(date_time, next_pre_working_day)
{
    const auto friday = make_utc(2025, 7, 11);

    ASSERT_EQ(friday.next_working_day().string(), "2025-07-14 00:00:00");

    ASSERT_EQ(friday.next_working_day(5).string(), "2025-07-18 00:00:00");

    ASSERT_EQ(friday.next_working_day(6).string(), "2025-07-21 00:00:00");

    ASSERT_EQ(friday.pre_working_day().string(), "2025-07-10 00:00:00");

    ASSERT_EQ(friday.pre_working_day(5).string(), "2025-07-04 00:00:00");

    ASSERT_EQ(friday.pre_working_day(10).string(), "2025-06-27 00:00:00");
}

TEST(date_time, timezone_calendar_operations_preserve_offset)
{
    const auto utc = make_utc(2026, 8, 22, 2, 0, 0);
    const auto bj  = utc.to_timezone(hj::timezone::BEIJING);

    ASSERT_EQ(bj.start_of_day().timezone_offset().total_seconds(), 8 * 3600);
    ASSERT_EQ(bj.next_day().timezone_offset().total_seconds(), 8 * 3600);
    ASSERT_EQ(bj.next_month().timezone_offset().total_seconds(), 8 * 3600);
    ASSERT_EQ(bj.start_of_year().timezone_offset().total_seconds(), 8 * 3600);
}

TEST(date_time, local_timezone_conversion_keeps_instant)
{
    const auto utc   = make_utc(2026, 1, 1, 0, 0, 0);
    const auto local = utc.to_timezone(hj::timezone::LOCAL);

    ASSERT_EQ(local.sec_since_epoch(), utc.sec_since_epoch());
}

#if !defined(_WIN32)
TEST(date_time, timeval_constructor)
{
    timeval tv{};
    tv.tv_sec  = 1704067200;
    tv.tv_usec = 123456;

    const hj::date_time dt(tv);

    ASSERT_EQ(dt.sec_since_epoch(), 1704067200);
    ASSERT_EQ(dt.milliseconds(), 123);
}
#endif

TEST(date_time, current_epoch_values_are_positive)
{
    ASSERT_GT(hj::date_time::current_sec_since_epoch(), 0);
    ASSERT_GT(hj::date_time::current_ms_since_epoch(), 0);
}
