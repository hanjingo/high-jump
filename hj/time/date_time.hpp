/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#ifndef DATE_TIME_HPP
#define DATE_TIME_HPP

#include <chrono>
#include <cstdint>
#include <ctime>
#include <cstring>
#include <functional>
#include <iomanip>
#include <limits>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <array>

#if !defined(_WIN32)
#include <sys/time.h>
#endif

#include <boost/date_time/posix_time/posix_time.hpp>

namespace hj
{

class date_time;

inline constexpr std::time_t sec    = std::time_t(1);
inline constexpr std::time_t minute = std::time_t(60) * sec;
inline constexpr std::time_t hour   = std::time_t(60) * minute;
inline constexpr std::time_t day    = std::time_t(24) * hour;
inline constexpr std::time_t week   = std::time_t(7) * day;

enum class weekday
{
    sunday = 0,
    monday,
    tuesday,
    wednesday,
    thursday,
    friday,
    saturday
};

enum class month
{
    january = 1,
    february,
    march,
    april,
    may,
    june,
    july,
    august,
    september,
    october,
    november,
    december
};

struct fixed_timezone
{
    const char                      *name;
    const char                      *abbreviation;
    const char                      *description;
    boost::posix_time::time_duration offset;

    fixed_timezone(const char *name_,
                   const char *abbreviation_,
                   int         hours_offset,
                   int         minutes_offset,
                   const char *description_)
        : name{name_}
        , abbreviation{abbreviation_}
        , description{description_}
        , offset{boost::posix_time::minutes(
              static_cast<long>(hours_offset) * 60L + minutes_offset)}
    {
    }
};

namespace timezone
{
inline const fixed_timezone BEIJING{
    "Asia/Shanghai", "CST", 8, 0, "China Standard Time"};
inline const fixed_timezone HONG_KONG{
    "Asia/Hong_Kong", "HKT", 8, 0, "Hong Kong Time"};
inline const fixed_timezone TOKYO{
    "Asia/Tokyo", "JST", 9, 0, "Japan Standard Time"};
inline const fixed_timezone SEOUL{
    "Asia/Seoul", "KST", 9, 0, "Korea Standard Time"};
inline const fixed_timezone MUMBAI{
    "Asia/Kolkata", "IST", 5, 30, "India Standard Time"};
inline const fixed_timezone BANGKOK{
    "Asia/Bangkok", "ICT", 7, 0, "Indochina Time"};
inline const fixed_timezone SINGAPORE{
    "Asia/Singapore", "SGT", 8, 0, "Singapore Standard Time"};
inline const fixed_timezone DUBAI{
    "Asia/Dubai", "GST", 4, 0, "Gulf Standard Time"};

inline const fixed_timezone LONDON{
    "Europe/London", "GMT", 0, 0, "Greenwich Mean Time"};
inline const fixed_timezone BERLIN{
    "Europe/Berlin", "CET", 1, 0, "Central European Time (fixed UTC+1)"};
inline const fixed_timezone PARIS{
    "Europe/Paris", "CET", 1, 0, "Central European Time (fixed UTC+1)"};
inline const fixed_timezone ROME{
    "Europe/Rome", "CET", 1, 0, "Central European Time (fixed UTC+1)"};
inline const fixed_timezone MOSCOW{
    "Europe/Moscow", "MSK", 3, 0, "Moscow Standard Time"};
inline const fixed_timezone STOCKHOLM{
    "Europe/Stockholm", "CET", 1, 0, "Central European Time (fixed UTC+1)"};

inline const fixed_timezone NEW_YORK{
    "America/New_York", "EST", -5, 0, "Eastern Standard Time (fixed UTC-5)"};
inline const fixed_timezone CHICAGO{
    "America/Chicago", "CST", -6, 0, "Central Standard Time (fixed UTC-6)"};
inline const fixed_timezone DENVER{
    "America/Denver", "MST", -7, 0, "Mountain Standard Time (fixed UTC-7)"};
inline const fixed_timezone LOS_ANGELES{
    "America/Los_Angeles", "PST", -8, 0, "Pacific Standard Time (fixed UTC-8)"};
inline const fixed_timezone TORONTO{
    "America/Toronto", "EST", -5, 0, "Eastern Standard Time (fixed UTC-5)"};
inline const fixed_timezone SAO_PAULO{
    "America/Sao_Paulo", "BRT", -3, 0, "Brazil Time (fixed UTC-3)"};
inline const fixed_timezone MEXICO_CITY{
    "America/Mexico_City", "CST", -6, 0, "Central Standard Time (fixed UTC-6)"};

inline const fixed_timezone SYDNEY{
    "Australia/Sydney",
    "AEST",
    10,
    0,
    "Australian Eastern Standard Time (fixed UTC+10)"};
inline const fixed_timezone MELBOURNE{
    "Australia/Melbourne",
    "AEST",
    10,
    0,
    "Australian Eastern Standard Time (fixed UTC+10)"};
inline const fixed_timezone AUCKLAND{
    "Pacific/Auckland",
    "NZST",
    12,
    0,
    "New Zealand Standard Time (fixed UTC+12)"};
inline const fixed_timezone PERTH{
    "Australia/Perth",
    "AWST",
    8,
    0,
    "Australian Western Standard Time (fixed UTC+8)"};

inline const fixed_timezone CAIRO{
    "Africa/Cairo", "EET", 2, 0, "Eastern European Time (fixed UTC+2)"};
inline const fixed_timezone JOHANNESBURG{
    "Africa/Johannesburg",
    "SAST",
    2,
    0,
    "South Africa Standard Time (fixed UTC+2)"};
inline const fixed_timezone LAGOS{
    "Africa/Lagos", "WAT", 1, 0, "West Africa Time (fixed UTC+1)"};

inline const fixed_timezone UTC{
    "UTC", "UTC", 0, 0, "Coordinated Universal Time"};

inline const fixed_timezone LOCAL{"LOCAL", "LOCAL", 0, 0, "Host Local Time"};
} // namespace timezone

inline const boost::posix_time::ptime NullTime{
    boost::gregorian::date(boost::gregorian::pos_infin),
    boost::posix_time::time_duration(0, 0, 0)};

class date_time
{
  public:
    using is_working_day_fn = std::function<bool(const date_time &)>;
    inline static constexpr const char *time_fmt = "%Y-%m-%d %H:%M:%S";

  private:
    struct local_time_tag
    {
    };

    boost::posix_time::ptime         _tm{NullTime};
    boost::posix_time::time_duration _offset{0, 0, 0};

    date_time(const boost::posix_time::ptime         &local_time,
              const boost::posix_time::time_duration &offset,
              local_time_tag)
        : _tm{local_time}
        , _offset{offset}
    {
    }

    static bool is_local_timezone(const fixed_timezone &tz) noexcept
    {
        return std::strcmp(tz.name, timezone::LOCAL.name) == 0;
    }

    static bool is_utc_timezone(const fixed_timezone &tz) noexcept
    {
        return std::strcmp(tz.name, timezone::UTC.name) == 0;
    }

    static bool localtime_safe(std::time_t value, std::tm &result) noexcept
    {
#if defined(_WIN32)
        return localtime_s(&result, &value) == 0;
#else
        return localtime_r(&value, &result) != nullptr;
#endif
    }

    static bool gmtime_safe(std::time_t value, std::tm &result) noexcept
    {
#if defined(_WIN32)
        return gmtime_s(&result, &value) == 0;
#else
        return gmtime_r(&value, &result) != nullptr;
#endif
    }

    static boost::posix_time::ptime
    local_time_for_instant(const boost::posix_time::ptime   &instant,
                           boost::posix_time::time_duration &offset)
    {
        const std::time_t value = boost::posix_time::to_time_t(instant);

        std::tm local_tm{};
        std::tm utc_tm{};
        if(!localtime_safe(value, local_tm) || !gmtime_safe(value, utc_tm))
            throw std::runtime_error(
                "failed to convert time_t to local/UTC time");

        const auto local = boost::posix_time::ptime_from_tm(local_tm);
        const auto utc   = boost::posix_time::ptime_from_tm(utc_tm);

        const auto whole_seconds = local - utc;
        const auto utc_seconds   = boost::posix_time::from_time_t(value);
        const auto fraction      = instant - utc_seconds;

        offset = whole_seconds;
        return local + fraction;
    }

    static boost::posix_time::time_duration
    offset_for(const fixed_timezone           &tz,
               const boost::posix_time::ptime &instant)
    {
        if(is_local_timezone(tz))
        {
            boost::posix_time::time_duration offset;
            (void) local_time_for_instant(instant, offset);
            return offset;
        }

        return tz.offset;
    }

    static date_time
    from_instant(const boost::posix_time::ptime         &instant,
                 const boost::posix_time::time_duration &offset)
    {
        return date_time(instant + offset, offset, local_time_tag{});
    }

    boost::posix_time::ptime instant() const
    {
        require_valid();
        return _tm - _offset;
    }

    void require_valid() const
    {
        if(is_null())
            throw std::logic_error("invalid/null date_time");
    }

    static std::optional<date_time> parse_impl(std::string_view str,
                                               std::string_view fmt)
    {
        std::tm            tm{};
        std::istringstream ss{std::string(str)};
        ss >> std::get_time(&tm, std::string(fmt).c_str());

        if(ss.fail())
            return std::nullopt;

        ss >> std::ws;
        if(!ss.eof())
            return std::nullopt;

        try
        {
            return date_time(boost::posix_time::ptime_from_tm(tm));
        }
        catch(...)
        {
            return std::nullopt;
        }
    }

    static std::size_t format_to_impl(const date_time &dt,
                                      char            *buffer,
                                      std::size_t      size,
                                      std::string_view fmt)
    {
        if(buffer == nullptr || size == 0 || dt.is_null())
            return 0;

        std::tm     tm = boost::posix_time::to_tm(dt._tm);
        std::string f(fmt);
        return std::strftime(buffer, size, f.c_str(), &tm);
    }

  public:
    date_time() noexcept = default;

    date_time(const date_time &)                = default;
    date_time(date_time &&) noexcept            = default;
    date_time &operator=(const date_time &)     = default;
    date_time &operator=(date_time &&) noexcept = default;
    ~date_time()                                = default;

    explicit date_time(const std::tm &tm, std::int64_t ms = 0)
        : _tm{boost::posix_time::ptime_from_tm(tm)
              + boost::posix_time::milliseconds(ms)}
    {
    }

    explicit date_time(std::time_t value, std::int64_t ms = 0)
        : _tm{boost::posix_time::from_time_t(value)
              + boost::posix_time::milliseconds(ms)}
    {
    }

    date_time(unsigned short year,
              unsigned short month_value,
              unsigned short day_value,
              long           hour_value   = 0,
              long           minute_value = 0,
              long           second_value = 0,
              long           ms           = 0)
        : _tm{boost::posix_time::ptime(
              boost::gregorian::date(year, month_value, day_value),
              boost::posix_time::time_duration(hour_value,
                                               minute_value,
                                               second_value,
                                               static_cast<long>(ms) * 1000L))}
    {
    }

    explicit date_time(const boost::gregorian::date           &dt,
                       const boost::posix_time::time_duration &time_of_day)
        : _tm{boost::posix_time::ptime(dt, time_of_day)}
    {
    }

    explicit date_time(const boost::gregorian::date &dt,
                       long                          hour_value   = 0,
                       long                          minute_value = 0,
                       long                          second_value = 0,
                       long                          ms           = 0)
        : _tm{boost::posix_time::ptime(
              dt,
              boost::posix_time::time_duration(hour_value,
                                               minute_value,
                                               second_value,
                                               static_cast<long>(ms) * 1000L))}
    {
    }

    /*
     * ptime is interpreted as a UTC/absolute instant.
     */
    explicit date_time(const boost::posix_time::ptime &tm)
        : _tm{tm}
    {
    }

    /*
     * ptime is interpreted as an absolute instant and converted to the
     * requested fixed timezone representation.
     */
    explicit date_time(const boost::posix_time::ptime         &tm,
                       const boost::posix_time::time_duration &offset)
        : _tm{tm + offset}
        , _offset{offset}
    {
    }

    explicit date_time(const std::chrono::system_clock::time_point &tp)
        : date_time(std::chrono::system_clock::to_time_t(tp))
    {
    }

    explicit date_time(const std::chrono::system_clock::time_point &tp,
                       const boost::posix_time::time_duration      &offset)
        : date_time(std::chrono::system_clock::to_time_t(tp), 0)
    {
        _tm += offset;
        _offset = offset;
    }

    explicit date_time(const std::string &str,
                       const std::string &fmt = time_fmt)
    {
        auto parsed = parse_impl(str, fmt);
        if(parsed)
            *this = *parsed;
    }

#if !defined(_WIN32)
    explicit date_time(const timeval &tv)
        : _tm{boost::posix_time::from_time_t(tv.tv_sec)
              + boost::posix_time::microseconds(tv.tv_usec)}
    {
    }

    date_time(const char *str, const char *fmt)
    {
        std::tm tm{};
        if(str == nullptr || fmt == nullptr
           || strptime(str, fmt, &tm) == nullptr)
            return;

        try
        {
            _tm = boost::posix_time::ptime_from_tm(tm);
        }
        catch(...)
        {
            _tm = NullTime;
        }
    }
#endif

    static const date_time &epoch_time()
    {
        static const date_time epoch{
            boost::posix_time::ptime(boost::gregorian::date(1970, 1, 1))};
        return epoch;
    }

    static date_time now(const fixed_timezone &tz = timezone::UTC)
    {
        const auto utc = boost::posix_time::microsec_clock::universal_time();

        if(is_local_timezone(tz))
        {
            boost::posix_time::time_duration offset;
            const auto local = local_time_for_instant(utc, offset);
            return date_time(local, offset, local_time_tag{});
        }

        return from_instant(utc, offset_for(tz, utc));
    }

    static date_time today(const fixed_timezone &tz = timezone::UTC)
    {
        const auto current = now(tz);
        current.require_valid();

        return date_time(boost::posix_time::ptime(current._tm.date()),
                         current._offset,
                         local_time_tag{});
    }

    /*
     * Convert this instant to another fixed timezone representation.
     * The instant is preserved; only the local calendar representation
     * changes.
     */
    date_time to_timezone(const fixed_timezone &tz) const
    {
        const auto current = instant();
        const auto offset  = offset_for(tz, current);
        return from_instant(current, offset);
    }

    boost::posix_time::time_duration timezone_offset() const noexcept
    {
        return _offset;
    }

    const boost::posix_time::ptime &local_ptime() const noexcept { return _tm; }

    static std::size_t format(const date_time &dt,
                              char            *buffer,
                              std::size_t      size,
                              std::string_view fmt = time_fmt)
    {
        return format_to_impl(dt, buffer, size, fmt);
    }

    static std::string format(const date_time &dt,
                              std::string_view fmt = time_fmt)
    {
        if(dt.is_null())
            return {};

        std::array<char, 128> buffer;
        std::tm               tm = boost::posix_time::to_tm(dt._tm);
        std::string           f(fmt);
        size_t                written =
            std::strftime(buffer.data(), buffer.size(), f.c_str(), &tm);
        if(written == 0)
            return {};

        return std::string(buffer.data(), written);
    }

    static std::optional<date_time> parse(std::string_view str,
                                          std::string_view fmt = time_fmt)
    {
        return parse_impl(str, fmt);
    }

    static bool
    parse(date_time &dt, std::string_view str, std::string_view fmt = time_fmt)
    {
        auto parsed = parse_impl(str, fmt);
        if(!parsed)
            return false;

        dt = *parsed;
        return true;
    }

    static bool parse(date_time &dt, const char *str, const char *fmt)
    {
        if(str == nullptr || fmt == nullptr)
            return false;

        return parse(dt, std::string_view(str), std::string_view(fmt));
    }

    std::int64_t sec_since_epoch() const
    {
        return (instant() - epoch_time()._tm).total_seconds();
    }

    static date_time from_sec_since_epoch(std::int64_t value)
    {
        return date_time(epoch_time()._tm + boost::posix_time::seconds(value));
    }

    static std::int64_t current_sec_since_epoch()
    {
        return now().sec_since_epoch();
    }

    std::int64_t ms_since_epoch() const
    {
        return (instant() - epoch_time()._tm).total_milliseconds();
    }

    static date_time from_ms_since_epoch(std::int64_t value)
    {
        return date_time(epoch_time()._tm
                         + boost::posix_time::milliseconds(value));
    }

    static std::int64_t current_ms_since_epoch()
    {
        return now().ms_since_epoch();
    }

    bool is_null() const noexcept { return _tm.is_pos_infinity(); }

    bool is_bigger(const date_time &dt) const noexcept { return *this > dt; }

    bool is_smaller(const date_time &dt) const noexcept { return *this < dt; }

    bool is_equal(const date_time &dt) const noexcept { return *this == dt; }

    bool is_working_day() const
    {
        require_valid();
        return !is_weekend();
    }

    bool is_working_day(const is_working_day_fn &predicate) const
    {
        require_valid();
        if(!predicate)
            throw std::invalid_argument("working-day predicate is empty");
        return predicate(*this);
    }

    bool is_weekend() const
    {
        const auto value = day_of_week();
        return value == weekday::saturday || value == weekday::sunday;
    }

    std::string string(std::string_view fmt = time_fmt) const
    {
        return format(*this, fmt);
    }

    std::tm date() const
    {
        require_valid();
        return boost::posix_time::to_tm(_tm);
    }

    std::time_t time() const { return boost::posix_time::to_time_t(instant()); }

    std::int64_t seconds() const
    {
        require_valid();
        return _tm.time_of_day().seconds();
    }

    std::int64_t milliseconds() const
    {
        require_valid();
        return _tm.time_of_day().fractional_seconds() / 1000;
    }

    std::int64_t seconds_to(const date_time &dt) const
    {
        return (dt.instant() - instant()).total_seconds();
    }

    std::int64_t minute() const
    {
        require_valid();
        return _tm.time_of_day().minutes();
    }

    std::int64_t minutes_to(const date_time &dt) const
    {
        return seconds_to(dt) / 60;
    }

    std::int64_t hour() const
    {
        require_valid();
        return _tm.time_of_day().hours();
    }

    std::int64_t hours_to(const date_time &dt) const
    {
        return seconds_to(dt) / 3600;
    }

    std::int64_t day() const
    {
        require_valid();
        return _tm.date().day();
    }

    std::int64_t days_to(const date_time &dt) const
    {
        return seconds_to(dt) / 86400;
    }

    weekday day_of_week() const
    {
        require_valid();
        return static_cast<weekday>(static_cast<int>(_tm.date().day_of_week()));
    }

    std::string day_of_week_str() const
    {
        switch(day_of_week())
        {
            case weekday::sunday:
                return "Sun";
            case weekday::monday:
                return "Mon";
            case weekday::tuesday:
                return "Tue";
            case weekday::wednesday:
                return "Wed";
            case weekday::thursday:
                return "Thu";
            case weekday::friday:
                return "Fri";
            case weekday::saturday:
                return "Sat";
        }

        return {};
    }

    std::int64_t day_of_month() const { return day(); }

    std::int64_t day_of_year() const
    {
        require_valid();
        return _tm.date().day_of_year();
    }

    hj::month month() const
    {
        require_valid();
        return static_cast<hj::month>(static_cast<int>(_tm.date().month()));
    }

    std::string month_str() const
    {
        switch(month())
        {
            case hj::month::january:
                return "January";
            case hj::month::february:
                return "February";
            case hj::month::march:
                return "March";
            case hj::month::april:
                return "April";
            case hj::month::may:
                return "May";
            case hj::month::june:
                return "June";
            case hj::month::july:
                return "July";
            case hj::month::august:
                return "August";
            case hj::month::september:
                return "September";
            case hj::month::october:
                return "October";
            case hj::month::november:
                return "November";
            case hj::month::december:
                return "December";
        }

        return {};
    }

    std::int64_t year() const
    {
        require_valid();
        return _tm.date().year();
    }

    date_time start_of_day() const
    {
        require_valid();
        return date_time(boost::posix_time::ptime(_tm.date()),
                         _offset,
                         local_time_tag{});
    }

    date_time end_of_day() const
    {
        require_valid();
        return date_time(boost::posix_time::ptime(
                             _tm.date(),
                             boost::posix_time::time_duration(23, 59, 59)),
                         _offset,
                         local_time_tag{});
    }

    date_time start_of_week() const
    {
        require_valid();

        const int current = static_cast<int>(_tm.date().day_of_week());

        const int delta =
            current == static_cast<int>(weekday::sunday) ? -6 : 1 - current;

        return date_time(
            boost::posix_time::ptime(_tm.date()
                                     + boost::gregorian::date_duration(delta)),
            _offset,
            local_time_tag{});
    }

    date_time end_of_week() const
    {
        require_valid();

        const int current = static_cast<int>(_tm.date().day_of_week());

        const int delta =
            current == static_cast<int>(weekday::sunday) ? 0 : 7 - current;

        return date_time(
            boost::posix_time::ptime(
                _tm.date() + boost::gregorian::date_duration(delta),
                boost::posix_time::time_duration(23, 59, 59)),
            _offset,
            local_time_tag{});
    }

    date_time start_of_month() const
    {
        require_valid();
        return date_time(
            boost::posix_time::ptime(boost::gregorian::date(_tm.date().year(),
                                                            _tm.date().month(),
                                                            1)),
            _offset,
            local_time_tag{});
    }

    date_time end_of_month() const
    {
        require_valid();
        return date_time(boost::posix_time::ptime(
                             _tm.date().end_of_month(),
                             boost::posix_time::time_duration(23, 59, 59)),
                         _offset,
                         local_time_tag{});
    }

    date_time start_of_quarter() const
    {
        require_valid();

        const int m           = static_cast<int>(_tm.date().month());
        const int start_month = ((m - 1) / 3) * 3 + 1;

        return date_time(
            boost::posix_time::ptime(
                boost::gregorian::date(_tm.date().year(), start_month, 1)),
            _offset,
            local_time_tag{});
    }

    date_time end_of_quarter() const
    {
        require_valid();

        const int  m           = static_cast<int>(_tm.date().month());
        const int  start_month = ((m - 1) / 3) * 3 + 1;
        const auto end_month_date =
            boost::gregorian::date(_tm.date().year(), start_month, 1)
            + boost::gregorian::months(2);

        return date_time(boost::posix_time::ptime(
                             end_month_date.end_of_month(),
                             boost::posix_time::time_duration(23, 59, 59)),
                         _offset,
                         local_time_tag{});
    }

    date_time start_of_half_year() const
    {
        require_valid();

        const int m           = static_cast<int>(_tm.date().month());
        const int start_month = m <= 6 ? 1 : 7;

        return date_time(
            boost::posix_time::ptime(
                boost::gregorian::date(_tm.date().year(), start_month, 1)),
            _offset,
            local_time_tag{});
    }

    date_time end_of_half_year() const
    {
        require_valid();

        const int m         = static_cast<int>(_tm.date().month());
        const int end_month = m <= 6 ? 6 : 12;

        const auto end_date =
            boost::gregorian::date(_tm.date().year(), end_month, 1)
                .end_of_month();

        return date_time(boost::posix_time::ptime(
                             end_date,
                             boost::posix_time::time_duration(23, 59, 59)),
                         _offset,
                         local_time_tag{});
    }

    date_time start_of_year() const
    {
        require_valid();
        return date_time(boost::posix_time::ptime(
                             boost::gregorian::date(_tm.date().year(), 1, 1)),
                         _offset,
                         local_time_tag{});
    }

    date_time end_of_year() const
    {
        require_valid();
        return date_time(boost::posix_time::ptime(
                             boost::gregorian::date(_tm.date().year(), 12, 31),
                             boost::posix_time::time_duration(23, 59, 59)),
                         _offset,
                         local_time_tag{});
    }

    date_time next_day(std::uint64_t n = 1) const
    {
        require_valid();
        return date_time(
            boost::posix_time::ptime(
                _tm.date()
                + boost::gregorian::date_duration(static_cast<long>(n))),
            _offset,
            local_time_tag{});
    }

    date_time pre_day(std::uint64_t n = 1) const
    {
        require_valid();
        return date_time(
            boost::posix_time::ptime(
                _tm.date()
                - boost::gregorian::date_duration(static_cast<long>(n))),
            _offset,
            local_time_tag{});
    }

    date_time next_weekday(weekday target_day, std::uint64_t n_week = 0) const
    {
        require_valid();

        const int current = static_cast<int>(day_of_week());
        const int target  = static_cast<int>(target_day);

        int delta = target - current;
        if(delta <= 0)
            delta += 7;

        delta += static_cast<int>(n_week * 7);

        return next_day(static_cast<std::uint64_t>(delta));
    }

    date_time pre_weekday(weekday target_day, std::uint64_t n_week = 0) const
    {
        require_valid();

        const int current = static_cast<int>(day_of_week());
        const int target  = static_cast<int>(target_day);

        int delta = current - target;
        if(delta <= 0)
            delta += 7;

        delta += static_cast<int>(n_week * 7);

        return pre_day(static_cast<std::uint64_t>(delta));
    }

    date_time next_working_day(std::uint64_t n_day = 1) const
    {
        return next_working_day(n_day, [](const date_time &value) {
            return value.is_working_day();
        });
    }

    date_time next_working_day(std::uint64_t            n_day,
                               const is_working_day_fn &predicate) const
    {
        require_valid();

        if(!predicate)
            throw std::invalid_argument("working-day predicate is empty");

        date_time     current = next_day();
        std::uint64_t count   = 0;

        while(true)
        {
            if(predicate(current))
            {
                ++count;
                if(count == (n_day == 0 ? 1 : n_day))
                    return current;
            }
            current = current.next_day();
        }
    }

    date_time pre_working_day(std::uint64_t n_day = 1) const
    {
        return pre_working_day(n_day, [](const date_time &value) {
            return value.is_working_day();
        });
    }

    date_time pre_working_day(std::uint64_t            n_day,
                              const is_working_day_fn &predicate) const
    {
        require_valid();

        if(!predicate)
            throw std::invalid_argument("working-day predicate is empty");

        date_time     current = pre_day();
        std::uint64_t count   = 0;

        while(true)
        {
            if(predicate(current))
            {
                ++count;
                if(count == (n_day == 0 ? 1 : n_day))
                    return current;
            }
            current = current.pre_day();
        }
    }

    date_time next_month(std::uint64_t n = 1) const
    {
        require_valid();

        const auto target =
            _tm.date() + boost::gregorian::months(static_cast<long>(n));

        return date_time(
            boost::posix_time::ptime(
                boost::gregorian::date(target.year(), target.month(), 1)),
            _offset,
            local_time_tag{});
    }

    date_time pre_month(std::uint64_t n = 1) const
    {
        require_valid();

        const auto target =
            _tm.date() - boost::gregorian::months(static_cast<long>(n));

        return date_time(
            boost::posix_time::ptime(
                boost::gregorian::date(target.year(), target.month(), 1)),
            _offset,
            local_time_tag{});
    }

    date_time next_quarter(std::uint64_t n = 1) const
    {
        require_valid();

        const std::int64_t current_index =
            static_cast<std::int64_t>(_tm.date().year()) * 4
            + (static_cast<int>(_tm.date().month()) - 1) / 3;

        const std::int64_t target_index =
            current_index + static_cast<std::int64_t>(n);

        const int target_year  = static_cast<int>(target_index / 4);
        const int quarter      = static_cast<int>(target_index % 4);
        const int target_month = quarter * 3 + 1;

        return date_time(
            boost::posix_time::ptime(
                boost::gregorian::date(target_year, target_month, 1)),
            _offset,
            local_time_tag{});
    }

    date_time pre_quarter(std::uint64_t n = 1) const
    {
        require_valid();

        const std::int64_t year = static_cast<std::int64_t>(_tm.date().year());
        const int month_idx = static_cast<int>(_tm.date().month()) - 1; // 0-11
        const std::int64_t current_index = year * 4 + (month_idx / 3);

        const std::int64_t target_index =
            current_index - static_cast<std::int64_t>(n);

        std::int64_t target_year =
            (target_index >= 0) ? (target_index / 4) : ((target_index - 3) / 4);
        int quarter = (target_index >= 0) ? (target_index % 4)
                                          : (3 + (target_index + 1) % 4);

        const int target_month = quarter * 3 + 1;

        return date_time(boost::posix_time::ptime(boost::gregorian::date(
                             static_cast<int>(target_year),
                             target_month,
                             1)),
                         _offset,
                         local_time_tag{});
    }

    date_time next_half_year(std::uint64_t n = 1) const
    {
        require_valid();

        const std::int64_t current_index =
            static_cast<std::int64_t>(_tm.date().year()) * 2
            + (static_cast<int>(_tm.date().month()) - 1) / 6;

        const std::int64_t target_index =
            current_index + static_cast<std::int64_t>(n);

        const int target_year  = static_cast<int>(target_index / 2);
        const int half         = static_cast<int>(target_index % 2);
        const int target_month = half * 6 + 1;

        return date_time(
            boost::posix_time::ptime(
                boost::gregorian::date(target_year, target_month, 1)),
            _offset,
            local_time_tag{});
    }

    date_time pre_half_year(std::uint64_t n = 1) const
    {
        require_valid();

        const std::int64_t current_index =
            static_cast<std::int64_t>(_tm.date().year()) * 2
            + (static_cast<int>(_tm.date().month()) - 1) / 6;

        const std::int64_t target_index =
            current_index - static_cast<std::int64_t>(n);

        const int target_year  = static_cast<int>(target_index / 2);
        const int half         = static_cast<int>(target_index % 2);
        const int target_month = half * 6 + 1;

        return date_time(
            boost::posix_time::ptime(
                boost::gregorian::date(target_year, target_month, 1)),
            _offset,
            local_time_tag{});
    }

    date_time next_year(std::uint64_t n = 1) const
    {
        require_valid();

        const auto target_year = static_cast<int>(_tm.date().year() + n);

        return date_time(
            boost::posix_time::ptime(boost::gregorian::date(target_year, 1, 1)),
            _offset,
            local_time_tag{});
    }

    date_time pre_year(std::uint64_t n = 1) const
    {
        require_valid();

        const auto target_year = static_cast<int>(_tm.date().year() - n);

        return date_time(
            boost::posix_time::ptime(boost::gregorian::date(target_year, 1, 1)),
            _offset,
            local_time_tag{});
    }

    friend bool operator==(const date_time &lhs, const date_time &rhs) noexcept
    {
        return (lhs._tm - lhs._offset) == (rhs._tm - rhs._offset);
    }

    friend bool operator!=(const date_time &lhs, const date_time &rhs) noexcept
    {
        return !(lhs == rhs);
    }

    friend bool operator<(const date_time &lhs, const date_time &rhs) noexcept
    {
        return lhs._tm - lhs._offset < rhs._tm - rhs._offset;
    }

    friend bool operator<=(const date_time &lhs, const date_time &rhs) noexcept
    {
        return !(rhs < lhs);
    }

    friend bool operator>(const date_time &lhs, const date_time &rhs) noexcept
    {
        return rhs < lhs;
    }

    friend bool operator>=(const date_time &lhs, const date_time &rhs) noexcept
    {
        return !(lhs < rhs);
    }
};

} // namespace hj

#endif // DATE_TIME_HPP
