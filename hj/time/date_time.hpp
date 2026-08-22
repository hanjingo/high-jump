/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
#include <boost/date_time/local_time/local_time.hpp>

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

namespace timezone
{
static constexpr char TZ_DB[] =
    R"(Region,STDABBR,STDNAME,DSTABBR,DSTNAME,GMTOFFSET,DSTADJUST,START_DATE_RULE,START_TIME,END_DATE_RULE,END_TIME
"Etc/UTC","UTC","Coordinated Universal Time","","","+00:00:00","","","","",""
"Asia/Shanghai","CST","China Standard Time","","","+08:00:00","","","","",""
"Asia/Beijing","CST","China Standard Time","","","+08:00:00","","","","",""
"Asia/Tokyo","JST","Japan Standard Time","","","+09:00:00","","","","",""
"Asia/Seoul","KST","Korea Standard Time","","","+09:00:00","","","","",""
"Asia/Kolkata","IST","India Standard Time","","","+05:30:00","","","","",""
"Asia/Bangkok","ICT","Indochina Time","","","+07:00:00","","","","",""
"Asia/Singapore","SGT","Singapore Standard Time","","","+08:00:00","","","","",""
"Asia/Dubai","GST","Gulf Standard Time","","","+04:00:00","","","","",""
"Asia/Hong_Kong","HKT","Hong Kong Time","","","+08:00:00","","","","",""
"Europe/London","GMT","Greenwich Mean Time","BST","British Summer Time","+00:00:00","+01:00:00","-1;0;3","+01:00:00","-1;0;10","+01:00:00"
"Europe/Berlin","CET","Central European Time","CEST","Central European Summer Time","+01:00:00","+01:00:00","-1;0;3","+02:00:00","-1;0;10","+03:00:00"
"Europe/Paris","CET","Central European Time","CEST","Central European Summer Time","+01:00:00","+01:00:00","-1;0;3","+02:00:00","-1;0;10","+03:00:00"
"Europe/Rome","CET","Central European Time","CEST","Central European Summer Time","+01:00:00","+01:00:00","-1;0;3","+02:00:00","-1;0;10","+03:00:00"
"Europe/Moscow","MSK","Moscow Standard Time","","","+03:00:00","","","","",""
"Europe/Stockholm","CET","Central European Time","CEST","Central European Summer Time","+01:00:00","+01:00:00","-1;0;3","+02:00:00","-1;0;10","+03:00:00"
"America/New_York","EST","Eastern Standard Time","EDT","Eastern Daylight Time","-05:00:00","+01:00:00","2;0;3","+02:00:00","1;0;11","+02:00:00"
"America/Chicago","CST","Central Standard Time","CDT","Central Daylight Time","-06:00:00","+01:00:00","2;0;3","+02:00:00","1;0;11","+02:00:00"
"America/Denver","MST","Mountain Standard Time","MDT","Mountain Daylight Time","-07:00:00","+01:00:00","2;0;3","+02:00:00","1;0;11","+02:00:00"
"America/Los_Angeles","PST","Pacific Standard Time","PDT","Pacific Daylight Time","-08:00:00","+01:00:00","2;0;3","+02:00:00","1;0;11","+02:00:00"
"America/Toronto","EST","Eastern Standard Time","EDT","Eastern Daylight Time","-05:00:00","+01:00:00","2;0;3","+02:00:00","1;0;11","+02:00:00"
"America/Sao_Paulo","BRT","Brasilia Time","","","-03:00:00","","","","",""
"America/Mexico_City","CST","Central Standard Time","","","-06:00:00","","","","",""
"Australia/Sydney","AEST","Australian Eastern Standard Time","AEDT","Australian Eastern Daylight Time","+10:00:00","+01:00:00","1;0;10","+02:00:00","1;0;4","+03:00:00"
"Australia/Melbourne","AEST","Australian Eastern Standard Time","AEDT","Australian Eastern Daylight Time","+10:00:00","+01:00:00","1;0;10","+02:00:00","1;0;4","+03:00:00"
"Pacific/Auckland","NZST","New Zealand Standard Time","NZDT","New Zealand Daylight Time","+12:00:00","+01:00:00","-1;0;9","+02:00:00","1;0;4","+03:00:00"
"Australia/Perth","AWST","Australian Western Standard Time","","","+08:00:00","","","","",""
"Africa/Cairo","EET","Eastern European Time","EEST","Eastern European Summer Time","+02:00:00","+01:00:00","-1;5;4","+00:00:00","-1;4;10","+00:00:00"
"Africa/Johannesburg","SAST","South Africa Standard Time","","","+02:00:00","","","","",""
"Africa/Lagos","WAT","West Africa Time","","","+01:00:00","","","","",""
)";
} // namespace timezone

struct dynamic_timezone
{
    using db_t = boost::local_time::tz_database;

    std::string                      name;
    boost::local_time::time_zone_ptr tz;

    dynamic_timezone(const std::string &dt_name)
        : name{dt_name}
    {
        tz = db().time_zone_from_region(name);
        if(!tz)
            throw std::runtime_error("Unknown timezone: " + dt_name);
    }

    static db_t &db()
    {
        static db_t database = [] {
            db_t               value;
            std::istringstream ss(timezone::TZ_DB);
            // Skip CSV header.
            std::string header;
            std::getline(ss, header);
            value.load_from_stream(ss);
            return value;
        }();
        return database;
    }
};

namespace timezone
{
// other
inline const dynamic_timezone &utc()
{
    static const dynamic_timezone tz{"Etc/UTC"};
    return tz;
}

// asian
inline const dynamic_timezone &shanghai()
{
    static const dynamic_timezone tz{"Asia/Shanghai"};
    return tz;
}

inline const dynamic_timezone &beijing()
{
    static const dynamic_timezone tz{"Asia/Beijing"};
    return tz;
}

inline const dynamic_timezone &hong_kong()
{
    static const dynamic_timezone tz{"Asia/Hong_Kong"};
    return tz;
}

inline const dynamic_timezone &tokyo()
{
    static const dynamic_timezone tz{"Asia/Tokyo"};
    return tz;
}

inline const dynamic_timezone &seoul()
{
    static const dynamic_timezone tz{"Asia/Seoul"};
    return tz;
}

inline const dynamic_timezone &mumbai()
{
    static const dynamic_timezone tz{"Asia/Kolkata"};
    return tz;
}

inline const dynamic_timezone &bangkok()
{
    static const dynamic_timezone tz{"Asia/Bangkok"};
    return tz;
}

inline const dynamic_timezone &singapore()
{
    static const dynamic_timezone tz{"Asia/Singapore"};
    return tz;
}

inline const dynamic_timezone &dubai()
{
    static const dynamic_timezone tz{"Asia/Dubai"};
    return tz;
}

// Europe
inline const dynamic_timezone &london()
{
    static const dynamic_timezone tz{"Europe/London"};
    return tz;
}
inline const dynamic_timezone &berlin()
{
    static const dynamic_timezone tz{"Europe/Berlin"};
    return tz;
}
inline const dynamic_timezone &paris()
{
    static const dynamic_timezone tz{"Europe/Paris"};
    return tz;
}
inline const dynamic_timezone &rome()
{
    static const dynamic_timezone tz{"Europe/Rome"};
    return tz;
}
inline const dynamic_timezone &moscow()
{
    static const dynamic_timezone tz{"Europe/Moscow"};
    return tz;
}
inline const dynamic_timezone &stockholm()
{
    static const dynamic_timezone tz{"Europe/Stockholm"};
    return tz;
}

// Americas
inline const dynamic_timezone &new_york()
{
    static const dynamic_timezone tz{"America/New_York"};
    return tz;
}
inline const dynamic_timezone &chicago()
{
    static const dynamic_timezone tz{"America/Chicago"};
    return tz;
}
inline const dynamic_timezone &denver()
{
    static const dynamic_timezone tz{"America/Denver"};
    return tz;
}
inline const dynamic_timezone &los_angeles()
{
    static const dynamic_timezone tz{"America/Los_Angeles"};
    return tz;
}
inline const dynamic_timezone &toronto()
{
    static const dynamic_timezone tz{"America/Toronto"};
    return tz;
}
inline const dynamic_timezone &sao_paulo()
{
    static const dynamic_timezone tz{"America/Sao_Paulo"};
    return tz;
}
inline const dynamic_timezone &mexico_city()
{
    static const dynamic_timezone tz{"America/Mexico_City"};
    return tz;
}

// Australia & Oceania
inline const dynamic_timezone &sydney()
{
    static const dynamic_timezone tz{"Australia/Sydney"};
    return tz;
}
inline const dynamic_timezone &melbourne()
{
    static const dynamic_timezone tz{"Australia/Melbourne"};
    return tz;
}
inline const dynamic_timezone &auckland()
{
    static const dynamic_timezone tz{"Pacific/Auckland"};
    return tz;
}
inline const dynamic_timezone &perth()
{
    static const dynamic_timezone tz{"Australia/Perth"};
    return tz;
}

// Africa
inline const dynamic_timezone &cairo()
{
    static const dynamic_timezone tz{"Africa/Cairo"};
    return tz;
}
inline const dynamic_timezone &johannesburg()
{
    static const dynamic_timezone tz{"Africa/Johannesburg"};
    return tz;
}
inline const dynamic_timezone &lagos()
{
    static const dynamic_timezone tz{"Africa/Lagos"};
    return tz;
}

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

    explicit date_time(const boost::posix_time::ptime         &tm,
                       const boost::posix_time::time_duration &offset)
        : _tm{tm + offset}
        , _offset{offset}
    {
    }

    explicit date_time(const std::chrono::system_clock::time_point &tp,
                       const boost::posix_time::time_duration      &offset)
        : date_time(std::chrono::system_clock::to_time_t(tp), 0)
    {
        _tm += offset;
        _offset = offset;
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

    static date_time
    from_instant(const boost::posix_time::ptime         &instant,
                 const boost::posix_time::time_duration &offset)
    {
        return date_time(instant + offset, offset, local_time_tag{});
    }

    boost::posix_time::ptime instant() const { return _tm - _offset; }

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

    explicit date_time(const std::chrono::system_clock::time_point &tp)
        : date_time(std::chrono::system_clock::to_time_t(tp))
    {
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

    static date_time now(const dynamic_timezone &dtz = timezone::utc())
    {
        const auto utc = boost::posix_time::microsec_clock::universal_time();
        boost::local_time::local_date_time ldt(utc, dtz.tz);
        auto offset = ldt.zone()->base_utc_offset();
        if(ldt.is_dst())
            offset += ldt.zone()->dst_offset();

        return from_instant(utc, offset);
    }

    static date_time today(const dynamic_timezone &dtz = timezone::utc())
    {
        const auto current = now(dtz);
        current.require_valid();
        return date_time(boost::posix_time::ptime(current._tm.date()),
                         current._offset,
                         local_time_tag{});
    }

    date_time to_timezone(const dynamic_timezone &dtz) const
    {
        boost::local_time::local_date_time ldt(instant(), dtz.tz);
        auto offset = ldt.zone()->base_utc_offset();
        if(ldt.is_dst())
            offset += ldt.zone()->dst_offset();

        return from_instant(instant(), offset);
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
        require_valid();
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

    std::time_t time() const
    {
        require_valid();
        return boost::posix_time::to_time_t(instant());
    }

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