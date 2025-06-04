#ifndef DATE_TIME_HPP
#define DATE_TIME_HPP

#include <ctime>
#include <chrono>
#include <iostream>

#include <boost/date_time/posix_time/posix_time.hpp>

namespace libcpp
{

static constexpr std::time_t sec    = std::time_t(1);
static constexpr std::time_t minute = std::time_t(60) * sec;
static constexpr std::time_t hour   = std::time_t(60) * minute;
static constexpr std::time_t day    = std::time_t(24) * hour;
static constexpr std::time_t week   = std::time_t(7) * day;

enum week_day {
    sunday = 0,
    monday,
    tuesday,
    wednesday,
    thursday,
    firday,
    saturday
};

enum moon {
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

static const boost::posix_time::ptime NullTime{boost::gregorian::date(boost::gregorian::pos_infin),
           boost::posix_time::time_duration(0, 0, 0)};

# define TIME_FMT "%Y-%m-%d %H:%M:%S"

class date_time
{
public:
    date_time()
        : tm_{NullTime} {};
    date_time(const date_time& dt)
        : tm_{dt.tm_} {};
    explicit date_time(const std::tm& tm, long ms = 0)
        : tm_{boost::posix_time::ptime_from_tm(tm) + boost::posix_time::time_duration(0, 0, 0, ms * 1000)} {};
    explicit date_time(const std::time_t time, long ms = 0)
        : tm_{boost::posix_time::from_time_t(time) + boost::posix_time::time_duration(0, 0, 0, ms * 1000)} {};

#if not defined(_WIN32)
    explicit date_time(const timeval& tv)
        : tm_{boost::posix_time::from_time_t(tv.tv_sec) +
              boost::posix_time::time_duration(0, 0, 0, tv.tv_usec * 1000)} {};
    date_time(const char* str, const char* fmt)
    {
        std::tm tm;
        strptime(str, fmt, &tm);
        tm_ = boost::posix_time::ptime_from_tm(tm);
    };
    date_time(const std::string& str, const std::string& fmt = TIME_FMT)
    {
        std::tm tm;
        strptime(str.c_str(), fmt.c_str(), &tm);
        tm_ = boost::posix_time::ptime_from_tm(tm);
    }
#endif

    date_time(unsigned short year, unsigned short month, unsigned short day,
             long hour = 0, long minute = 0, long seconds = 0, long ms = 0)
        : tm_{boost::posix_time::ptime(boost::gregorian::date(year, month, day),
                                       boost::posix_time::time_duration(hour, minute, seconds, ms * 1000))}
    {}

    explicit date_time(const boost::posix_time::ptime& tm) : tm_{tm} {};
    explicit date_time(const boost::gregorian::date& dt,
                      long hour = 0, long minute = 0, long seconds = 0, long ms = 0)
        : tm_{boost::posix_time::ptime(dt, boost::posix_time::time_duration(hour, minute, seconds, ms * 1000))}
    {};
    explicit date_time(const std::chrono::system_clock::time_point& tp)
        : date_time(std::chrono::system_clock::to_time_t(tp)) {}

    ~date_time() {}

    static date_time now()
    {
        return date_time(boost::posix_time::microsec_clock::universal_time());
    }

    static date_time today()
    {
        return date_time(boost::posix_time::microsec_clock::universal_time().date());
    }

    static void format(const date_time& dt, char* str, const std::size_t sz, const char* fmt = TIME_FMT)
    {
        std::tm tm = boost::posix_time::to_tm(dt.tm_);
        strftime(str, sz, fmt, &tm);
    }

    static std::string format(const date_time& dt, const std::string& fmt = TIME_FMT)
    {
        std::tm tm = boost::posix_time::to_tm(dt.tm_);
        char buf[256];
        strftime(buf, sizeof(buf), fmt.c_str(), &tm);
        return std::string(buf);
    }

    static date_time parse(const std::string& str, const std::string& fmt = TIME_FMT)
    {
        std::tm tm;
        //strptime(str.c_str(), fmt.c_str(), &tm);
        std::istringstream ss(str);
        ss >> std::get_time(&tm, fmt.c_str());
        if (ss.fail())
            throw std::runtime_error("Failed to parse date/time string");

        return date_time(tm);
    }

    static void parse(date_time& dt, const char* str, const char* fmt)
    {
        std::tm tm;
        //strptime(str, fmt, &tm);
        std::istringstream ss(str);
        ss >> std::get_time(&tm, fmt);
        if (ss.fail())
            throw std::runtime_error("Failed to parse date/time string");

        dt.tm_ = boost::posix_time::ptime_from_tm(tm);
    }

    static void parse(date_time& dt, const std::string& str, const std::string& fmt = TIME_FMT)
    {
        std::tm tm;
        //strptime(str.c_str(), fmt.c_str(), &tm);
        std::istringstream ss(str);
        ss >> std::get_time(&tm, fmt.c_str());
        if (ss.fail())
            throw std::runtime_error("Failed to parse date/time string");

        dt.tm_ = boost::posix_time::ptime_from_tm(tm);
    }

public:
    date_time& operator=(const date_time& dt)
    {
        if (this == &dt) {
            return *this;
        }

        tm_ = dt.tm_;
        return *this;
    }

public:
    inline bool is_null()
    {
        return NullTime == tm_;
    }

    inline std::string string(const std::string& fmt = TIME_FMT)
    {
        std::tm tm = boost::posix_time::to_tm(tm_);
        char buf[256];
        strftime(buf, sizeof(buf), fmt.c_str(), &tm);
        return std::string(buf);
    }

    inline std::tm date()
    {
        return boost::posix_time::to_tm(tm_);
    }

    inline std::time_t time()
    {
        return boost::posix_time::to_time_t(tm_);
    }

    inline int seconds()
    {
        return tm_.time_of_day().seconds();
    }

    inline long seconds_to(const date_time& dt)
    {
        return (dt.tm_ - tm_).total_seconds();
    }

    inline int minute()
    {
        return tm_.time_of_day().minutes();
    }

    inline long minutes_to(const date_time& dt)
    {
        return (dt.tm_ - tm_).total_seconds() / 60;
    }

    inline int hour()
    {
        return tm_.time_of_day().hours();
    }

    inline long hours_to(const date_time& dt)
    {
        return (dt.tm_ - tm_).total_seconds() / 3600;
    }

    inline int day()
    {
        return tm_.date().day();
    }

    inline long days_to(const date_time& dt)
    {
        return (dt.tm_ - tm_).total_seconds() / 86400;
    }

    inline week_day day_of_week()
    {
        return week_day(int(tm_.date().day_of_week()));
    }

    inline int day_of_month()
    {
        return tm_.date().day();
    }

    inline int day_of_year()
    {
        return tm_.date().day_of_year();
    }

    inline moon month()
    {
        return moon(int(tm_.date().month()));
    }

    inline int year()
    {
        return tm_.date().year();
    }

    inline date_time start_of_day()
    {
        return date_time(tm_.date());
    }

    inline date_time end_of_day()
    {
        return date_time(year(), month(), day(), 23, 59, 59);
    }

    inline date_time start_of_week()
    {
        return day_of_week() == week_day::sunday ? date_time(tm_.date() + boost::gregorian::date_duration(-6)).start_of_day()
               : date_time(tm_.date() + boost::gregorian::date_duration(1 - day_of_week())).start_of_day();
    }

    inline date_time end_of_week()
    {
        return day_of_week() == week_day::sunday ? date_time(tm_.date(), 23, 59, 59)
               : date_time(tm_.date() + boost::gregorian::date_duration(7 - day_of_week()), 23, 59, 59);
    }

    inline date_time start_of_month()
    {
        return date_time(year(), month(), 1);
    }

    inline date_time end_of_month()
    {
        return date_time(tm_.date().end_of_month(), 23, 59, 59);
    }

    inline date_time start_of_quarter()
    {
        int m = month();
        if (m <= 3) {
            return date_time(year(), 1, 1);
        } else if (m <= 6) {
            return date_time(year(), 4, 1);
        } else if (m <= 9) {
            return date_time(year(), 7, 1);
        } else {
            return date_time(year(), 10, 1);
        }
    }

    inline date_time end_of_quarter()
    {
        int m = month();
        if (m <= 3) {
            return date_time(year(), 3, 31, 23, 59, 59);
        } else if (m <= 6) {
            return date_time(year(), 6, 30, 23, 59, 59);
        } else if (m <= 9) {
            return date_time(year(), 9, 30, 23, 59, 59);
        } else {
            return date_time(year(), 12, 31, 23, 59, 59);
        }
    }

    inline date_time start_of_half_year()
    {
        return month() <= june ? date_time(tm_.date().year(), 1, 1)
               : date_time(tm_.date().year(), 7, 1);
    }

    inline date_time end_of_half_year()
    {
        return month() <= june ? date_time(tm_.date().year(), 6, 30, 23, 59, 59)
               : date_time(tm_.date().year(), 12, 31, 23, 59, 59);
    }

    inline date_time start_of_year()
    {
        return date_time(year(), 1, 1);
    }

    inline date_time end_of_year()
    {
        return date_time(year(), 12, 31, 23, 59, 59);
    }

    inline date_time next_day()
    {
        return date_time(tm_.date() + boost::gregorian::date_duration(1));
    }

    inline date_time next_day_n(unsigned int n)
    {
        return date_time(tm_.date() + boost::gregorian::date_duration(n));
    }

    inline date_time pre_day()
    {
        return date_time(tm_.date() - boost::gregorian::date_duration(1));
    }

    inline date_time pre_day_n(unsigned int n)
    {
        return date_time(tm_.date() - boost::gregorian::date_duration(n));
    }

    inline date_time next_month()
    {
        return date_time(tm_.date().end_of_month() + boost::gregorian::date_duration(1));
    }

    inline date_time next_month_n(unsigned int n)
    {
        date_time dt = *this;
        for (unsigned int i = 0; i < n; i++) {
            dt = dt.next_month();
        }
        return dt;
    }

    inline date_time pre_month()
    {
        auto y = year();
        auto m = tm_.date().month();
        return m == 1 ? date_time(y - 1, 12, 1) : date_time(y, m - 1, 1);
    }

    inline date_time pre_month_n(unsigned int n)
    {
        date_time dt = *this;
        for (unsigned int i = 0; i < n; i++) {
            dt = dt.pre_month();
        }
        return dt;
    }

    inline date_time next_quarter()
    {
        auto dt = end_of_quarter();
        dt.tm_ += boost::gregorian::date_duration(1);
        return dt.start_of_day();
    }

    inline date_time next_quarter_n(unsigned int n)
    {
        date_time dt = *this;
        for (auto i = 0; i < n; i++) {
            dt = dt.next_quarter();
        }
        return dt;
    }

    inline date_time pre_quarter()
    {
        date_time dt = start_of_quarter();
        dt.tm_ -= boost::gregorian::date_duration(1);
        return dt.start_of_quarter();
    }

    inline date_time pre_quarter_n(unsigned int n)
    {
        date_time dt = *this;
        for (auto i = 0; i < n; i++) {
            dt = dt.pre_quarter();
        }
        return dt;
    }

    inline date_time next_half_year()
    {
        date_time dt = end_of_half_year();
        return date_time(dt.tm_ + boost::gregorian::date_duration(1)).start_of_day();
    }

    inline date_time next_half_year_n(unsigned int n)
    {
        date_time dt = *this;
        for (auto i = 0; i < n; i++) {
            dt = dt.next_half_year();
        }
        return dt;
    }

    inline date_time pre_half_year()
    {
        date_time dt = start_of_half_year();
        return date_time(dt.tm_ - boost::gregorian::date_duration(1)).start_of_half_year();
    }

    inline date_time pre_half_year_n(unsigned int n)
    {
        date_time dt = *this;
        for (auto i = 0; i < n; i++) {
            dt = dt.pre_half_year();
        }
        return dt;
    }

    inline date_time next_year()
    {
        date_time dt = end_of_year();
        return date_time(dt.tm_ + boost::gregorian::date_duration(1)).start_of_day();
    }

    inline date_time next_year_n(unsigned int n)
    {
        date_time dt = *this;
        for (auto i = 0; i < n; i++) {
            dt = dt.next_year();
        }
        return dt;
    }

    inline date_time pre_year()
    {
        return date_time(year() - 1, 1, 1);
    }

    inline date_time pre_year_n(unsigned int n)
    {
        date_time dt = *this;
        for (auto i = 0; i < n; i++) {
            dt = dt.pre_year();
        }
        return dt;
    }

private:
    boost::posix_time::ptime tm_;
};

}

#endif