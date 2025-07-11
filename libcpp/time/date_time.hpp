#ifndef DATE_TIME_HPP
#define DATE_TIME_HPP

#include <ctime>
#include <chrono>
#include <iostream>
#include <functional>

#include <boost/date_time/posix_time/posix_time.hpp>

#ifndef TIME_FMT
#define TIME_FMT "%Y-%m-%d %H:%M:%S"
#endif

namespace libcpp
{

class date_time;

static constexpr std::time_t sec    = std::time_t(1);
static constexpr std::time_t minute = std::time_t(60) * sec;
static constexpr std::time_t hour   = std::time_t(60) * minute;
static constexpr std::time_t day    = std::time_t(24) * hour;
static constexpr std::time_t week   = std::time_t(7) * day;

enum class week_day 
{
    sunday = 0,
    monday,
    tuesday,
    wednesday,
    thursday,
    friday,
    saturday
};

enum class moon 
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

struct timezone_info 
{
    const char* name;
    const char* abbreviation;
    long hours_offset;
    long minutes_offset;
    const char* description;
};

namespace timezone 
{
    // ASIA
    constexpr timezone_info BEIJING     = {"Asia/Shanghai", "CST", 8, 0, "China Standard Time"};
    constexpr timezone_info HONG_KONG   = {"Asia/Hong_Kong", "HKT", 8, 0, "Hong Kong Time"};
    constexpr timezone_info TOKYO       = {"Asia/Tokyo", "JST", 9, 0, "Japan Standard Time"};
    constexpr timezone_info SEOUL       = {"Asia/Seoul", "KST", 9, 0, "Korea Standard Time"};
    constexpr timezone_info MUMBAI      = {"Asia/Kolkata", "IST", 5, 30, "India Standard Time"};
    constexpr timezone_info BANGKOK     = {"Asia/Bangkok", "ICT", 7, 0, "Indochina Time"};
    constexpr timezone_info SINGAPORE   = {"Asia/Singapore", "SGT", 8, 0, "Singapore Standard Time"};
    constexpr timezone_info DUBAI       = {"Asia/Dubai", "GST", 4, 0, "Gulf Standard Time"};
    
    // EUROPE
    constexpr timezone_info LONDON      = {"Europe/London", "GMT", 0, 0, "Greenwich Mean Time"};
    constexpr timezone_info BERLIN      = {"Europe/Berlin", "CET", 1, 0, "Central European Time"};
    constexpr timezone_info PARIS       = {"Europe/Paris", "CET", 1, 0, "Central European Time"};
    constexpr timezone_info ROME        = {"Europe/Rome", "CET", 1, 0, "Central European Time"};
    constexpr timezone_info MOSCOW      = {"Europe/Moscow", "MSK", 3, 0, "Moscow Standard Time"};
    constexpr timezone_info STOCKHOLM   = {"Europe/Stockholm", "CET", 1, 0, "Central European Time"};
    
    // AMERICAS
    constexpr timezone_info NEW_YORK    = {"America/New_York", "EST", -5, 0, "Eastern Standard Time"};
    constexpr timezone_info CHICAGO     = {"America/Chicago", "CST", -6, 0, "Central Standard Time"};
    constexpr timezone_info DENVER      = {"America/Denver", "MST", -7, 0, "Mountain Standard Time"};
    constexpr timezone_info LOS_ANGELES = {"America/Los_Angeles", "PST", -8, 0, "Pacific Standard Time"};
    constexpr timezone_info TORONTO     = {"America/Toronto", "EST", -5, 0, "Eastern Standard Time"};
    constexpr timezone_info SAO_PAULO   = {"America/Sao_Paulo", "BRT", -3, 0, "Brazil Time"};
    constexpr timezone_info MEXICO_CITY = {"America/Mexico_City", "CST", -6, 0, "Central Standard Time"};
    
    // AUSTRALIA & OCEANIA
    constexpr timezone_info SYDNEY      = {"Australia/Sydney", "AEST", 10, 0, "Australian Eastern Standard Time"};
    constexpr timezone_info MELBOURNE   = {"Australia/Melbourne", "AEST", 10, 0, "Australian Eastern Standard Time"};
    constexpr timezone_info AUCKLAND    = {"Pacific/Auckland", "NZST", 12, 0, "New Zealand Standard Time"};
    constexpr timezone_info PERTH       = {"Australia/Perth", "AWST", 8, 0, "Australian Western Standard Time"};

    // AFRICA
    constexpr timezone_info CAIRO       = {"Africa/Cairo", "EET", 2, 0, "Eastern European Time"};
    constexpr timezone_info JOHANNESBURG= {"Africa/Johannesburg", "SAST", 2, 0, "South Africa Standard Time"};
    constexpr timezone_info LAGOS       = {"Africa/Lagos", "WAT", 1, 0, "West Africa Time"};
    
    // other
    constexpr timezone_info UTC         = {"UTC", "UTC", 0, 0, "Coordinated Universal Time"};
    constexpr timezone_info LOCAL       = {"LOCAL", "LOCAL", 0, 0, "Local Time"};
}

static const boost::posix_time::ptime NullTime{boost::gregorian::date(boost::gregorian::pos_infin),
           boost::posix_time::time_duration(0, 0, 0)};

class date_time
{
public:
    using is_working_day_fn = std::function<bool(const date_time&)>;

public:
    date_time()
        : tm_{NullTime} 
    {};
    date_time(const date_time& dt)
        : tm_{dt.tm_} 
    {};
    explicit date_time(const std::tm& tm, int64_t ms = 0)
        : tm_{boost::posix_time::ptime_from_tm(tm) + boost::posix_time::time_duration(0, 0, 0, int64_t(ms) * 1000)} 
    {};
    explicit date_time(const std::time_t time, int64_t ms = 0)
        : tm_{boost::posix_time::from_time_t(time) + boost::posix_time::time_duration(0, 0, 0, int64_t(ms) * 1000)} 
    {};
    date_time(unsigned short year, unsigned short month, unsigned short day, long hour = 0, long minute = 0, 
                long seconds = 0, long ms = 0)
        : tm_{boost::posix_time::ptime(
                boost::gregorian::date(year, month, day),
                boost::posix_time::time_duration(hour, minute, seconds, int64_t(ms) * 1000))}
    {};
    explicit date_time(const boost::posix_time::ptime& tm) 
        : tm_{tm} 
    {};
    explicit date_time(const boost::gregorian::date& dt, long hour = 0, long minute = 0, long seconds = 0, long ms = 0)
        : tm_{boost::posix_time::ptime(dt, boost::posix_time::time_duration(hour, minute, seconds, int64_t(ms) * 1000))}
    {};
    explicit date_time(const std::chrono::system_clock::time_point& tp)
        : date_time(std::chrono::system_clock::to_time_t(tp)) 
    {};

    explicit date_time(const std::string& str, const std::string& fmt = TIME_FMT)
    {
        std::tm tm = {};
        std::istringstream ss(str);
        ss >> std::get_time(&tm, fmt.c_str());
        if (!ss.fail())
            tm_ = boost::posix_time::ptime_from_tm(tm);
    };

#if !defined(_WIN32)
    explicit date_time(const timeval& tv)
        : tm_{boost::posix_time::from_time_t(tv.tv_sec) +
              boost::posix_time::time_duration(0, 0, 0, tv.tv_usec * 1000)} {};
    date_time(const char* str, const char* fmt)
    {
        std::tm tm;
        strptime(str, fmt, &tm);
        tm_ = boost::posix_time::ptime_from_tm(tm);
    };
#endif

    ~date_time() {};

public:
    date_time& operator=(const date_time& dt)
    {
        if (this == &dt)
            return *this;

        tm_ = dt.tm_;
        return *this;
    }

    inline bool operator==(const date_time& other) const
    {
        return tm_ == other.tm_;
    }

    inline bool operator!=(const date_time& other) const
    {
        return tm_ != other.tm_;
    }

    inline bool operator<(const date_time& other) const
    {
        return tm_ < other.tm_;
    }

    inline bool operator<=(const date_time& other) const
    {
        return tm_ <= other.tm_;
    }

    inline bool operator>(const date_time& other) const
    {
        return tm_ > other.tm_;
    }

    inline bool operator>=(const date_time& other) const
    {
        return tm_ >= other.tm_;
    }

    static date_time now(const timezone_info& tz = timezone::LOCAL)
    {
        if (tz.name == timezone::UTC.name) 
        {
            return date_time(boost::posix_time::microsec_clock::universal_time());
        } 
        else if (tz.name == timezone::LOCAL.name) 
        {
            return date_time(boost::posix_time::microsec_clock::local_time());
        } 
        else 
        {
            auto utc_time = boost::posix_time::microsec_clock::universal_time();
            auto offset = boost::posix_time::time_duration(tz.hours_offset, tz.minutes_offset, 0);
            return date_time(utc_time + offset);
        }
    }

    static date_time today(const timezone_info& tz = timezone::LOCAL)
    {
        if (tz.name == timezone::UTC.name) 
        {
            return date_time(boost::gregorian::day_clock::universal_day());
        } 
        else if (tz.name == timezone::LOCAL.name) 
        {
            return date_time(boost::gregorian::day_clock::local_day());
        } 
        else 
        {
            auto utc_day = boost::gregorian::day_clock::universal_day();
            auto offset = boost::posix_time::time_duration(tz.hours_offset, tz.minutes_offset, 0);
            return date_time(utc_day, offset.hours(), offset.minutes(), 0, 0);
        }
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
    inline bool is_null()
    {
        return NullTime == tm_;
    }

    inline bool is_bigger(const date_time& dt)
    {
        return tm_ > dt.tm_;
    }

    inline bool is_smaller(const date_time& dt)
    {
        return tm_ < dt.tm_;
    }

    inline bool is_equal(const date_time& dt)
    {
        return tm_ == dt.tm_;
    }

    virtual inline bool is_working_day()
    {
        // week_day wd = day_of_week();
        // return wd != week_day::saturday && wd != week_day::sunday;

        return working_day_func_(*this);
    }

    inline bool is_weekend()
    {
        return !is_working_day();
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

    inline int64_t seconds()
    {
        return tm_.time_of_day().seconds();
    }

    inline int64_t seconds_to(const date_time& dt)
    {
        return (dt.tm_ - tm_).total_seconds();
    }

    inline int64_t minute()
    {
        return tm_.time_of_day().minutes();
    }

    inline int64_t minutes_to(const date_time& dt)
    {
        return (dt.tm_ - tm_).total_seconds() / 60;
    }

    inline int64_t hour()
    {
        return tm_.time_of_day().hours();
    }

    inline int64_t hours_to(const date_time& dt)
    {
        return (dt.tm_ - tm_).total_seconds() / 3600;
    }

    inline int64_t day()
    {
        return tm_.date().day();
    }

    inline int64_t days_to(const date_time& dt)
    {
        return (dt.tm_ - tm_).total_seconds() / 86400;
    }

    inline week_day day_of_week()
    {
        if (is_null())
            return week_day::sunday; // Default to Sunday if null

        return static_cast<week_day>(int64_t(tm_.date().day_of_week()));
    }

    inline std::string day_of_week_str()
    {
        if (is_null())
            return "???"; // Default to "???" if null

        switch (day_of_week()) 
        {
        case week_day::monday:    return "Mon";
        case week_day::tuesday:   return "Tue";
        case week_day::wednesday: return "Wed";
        case week_day::thursday:  return "Thu";
        case week_day::friday:    return "Fri";
        case week_day::saturday:  return "Sat";
        case week_day::sunday:    return "Sun";
        default:                  return "???";
        }
    }

    inline int64_t day_of_month()
    {
        return tm_.date().day();
    }

    inline int64_t day_of_year()
    {
        return tm_.date().day_of_year();
    }

    inline moon month()
    {
        if (is_null())
            return moon::january; // Default to January if null

        return static_cast<moon>(int64_t(tm_.date().month()));
    }

    inline std::string month_str()
    {
        if (is_null())
            return "???"; // Default to "???" if null

        switch (month()) 
        {
        case moon::january:   return "January";
        case moon::february:  return "February";
        case moon::march:     return "March";
        case moon::april:     return "April";
        case moon::may:       return "May";
        case moon::june:      return "June";
        case moon::july:      return "July";
        case moon::august:    return "August";
        case moon::september: return "September";
        case moon::october:   return "October";
        case moon::november:  return "November";
        case moon::december:  return "December";
        default:              return "???";
        }
    }

    inline int64_t year()
    {
        return tm_.date().year();
    }

    inline date_time start_of_day()
    {
        return date_time(tm_.date());
    }

    inline date_time end_of_day()
    {
        return date_time(tm_.date().year(), tm_.date().month(), tm_.date().day(), 23, 59, 59);
    }

    inline date_time start_of_week()
    {
        return day_of_week() == week_day::sunday ? 
              date_time(tm_.date() + boost::gregorian::date_duration(-6)).start_of_day()
            : date_time(tm_.date() + boost::gregorian::date_duration(1 - static_cast<int64_t>(day_of_week()))).start_of_day();
    }

    inline date_time end_of_week()
    {
        return day_of_week() == week_day::sunday ? 
              date_time(tm_.date(), 23, 59, 59)
            : date_time(tm_.date() + boost::gregorian::date_duration(7 - static_cast<int64_t>(day_of_week())), 23, 59, 59);
    }

    inline date_time start_of_month()
    {
        return date_time(tm_.date().year(), static_cast<unsigned short>(month()), 1);
    }

    inline date_time end_of_month()
    {
        return date_time(tm_.date().end_of_month(), 23, 59, 59);
    }

    inline date_time start_of_quarter()
    {
        int64_t m = static_cast<int64_t>(month());
        if (m <= 3)
            return date_time(tm_.date().year(), 1, 1);
        else if (m <= 6)
            return date_time(tm_.date().year(), 4, 1);
        else if (m <= 9)
            return date_time(tm_.date().year(), 7, 1);
        else
            return date_time(tm_.date().year(), 10, 1);
    }

    inline date_time end_of_quarter()
    {
        int64_t m = static_cast<int64_t>(month());
        if (m <= 3)
            return date_time(tm_.date().year(), 3, 31, 23, 59, 59);
        else if (m <= 6)
            return date_time(tm_.date().year(), 6, 30, 23, 59, 59);
        else if (m <= 9)
            return date_time(tm_.date().year(), 9, 30, 23, 59, 59);
        else
            return date_time(tm_.date().year(), 12, 31, 23, 59, 59);
    }

    inline date_time start_of_half_year()
    {
        return static_cast<int64_t>(month()) <= static_cast<int64_t>(moon::june) ? 
            date_time(tm_.date().year(), 1, 1) : date_time(tm_.date().year(), 7, 1);
    }

    inline date_time end_of_half_year()
    {
        return static_cast<int64_t>(month()) <= static_cast<int64_t>(moon::june) ? 
            date_time(tm_.date().year(), 6, 30, 23, 59, 59) : date_time(tm_.date().year(), 12, 31, 23, 59, 59);
    }

    inline date_time start_of_year()
    {
        return date_time(tm_.date().year(), 1, 1);
    }

    inline date_time end_of_year()
    {
        return date_time(tm_.date().year(), 12, 31, 23, 59, 59);
    }

    inline date_time next_day(const unsigned long n = 1)
    {
        return date_time(tm_.date() + boost::gregorian::date_duration(n));
    }

    inline date_time pre_day(const unsigned long n = 1)
    {
        return date_time(tm_.date() - boost::gregorian::date_duration(n));
    }

    inline date_time next_weekday(const week_day target_day, const unsigned long n_week = 0)
    {
        unsigned long current_day = static_cast<unsigned long>(day_of_week());
        unsigned long target = static_cast<unsigned long>(target_day);
        if (target > current_day)
            return next_day(target - current_day + n_week * 7);
        else
            return next_day(7 - (current_day - target) + n_week * 7);
    }

    inline date_time pre_weekday(const week_day target_day, const unsigned long n_week = 0)
    {
        unsigned long current_day = static_cast<unsigned long>(day_of_week());
        unsigned long target = static_cast<unsigned long>(target_day);
        if (target < current_day)
            return pre_day(current_day - target + n_week * 7);
        else
            return pre_day(7 - (target - current_day) + n_week * 7);
    }

    inline date_time next_working_day(const unsigned long n_day = 1)
    {
        date_time current = next_day(1);
    
        if (n_day == 0) 
        {
            while (!current.is_working_day())
                current = current.next_day(1);

            return current;
        } 
        else 
        {
            unsigned long count = 0;
            while (true) 
            {
                if (current.is_working_day()) 
                {
                    count++;
                    if (count == n_day)
                        return current;
                }
                current = current.next_day(1);
            }
        }
    }

    inline date_time pre_working_day(const unsigned long n_day = 1)
    {
        date_time current = pre_day(1);
        if (n_day == 0) 
        {
            while (!current.is_working_day())
                current = current.pre_day(1);

            return current;
        } 
        else 
        {
            unsigned long count = 0;
            while (true) 
            {
                if (current.is_working_day()) 
                {
                    count++;
                    if (count == n_day)
                        return current;
                }

                current = current.pre_day(1);
            }
        }
    }

    inline date_time next_month(const unsigned long n = 1)
    {
        auto current_date = tm_.date();
        auto target_date = current_date + boost::gregorian::months(n);

        return date_time(boost::gregorian::date(target_date.year(), target_date.month(), 1));
    }

    inline date_time pre_month(const unsigned long n = 1)
    {
        auto current_date = tm_.date();
        auto target_date = current_date - boost::gregorian::months(n);
        
        return date_time(boost::gregorian::date(target_date.year(), target_date.month(), 1));
    }

    inline date_time next_quarter(const uint64_t n = 1)
    {
        int current_month = static_cast<int>(tm_.date().month());
        int current_quarter = (current_month - 1) / 3;  // 0, 1, 2, 3
        int current_quarter_start_month = current_quarter * 3 + 1;  // 1, 4, 7, 10
        
        int target_quarter = current_quarter + n;
        int target_year = tm_.date().year() + (target_quarter / 4);
        target_quarter = target_quarter % 4;
        int target_month = target_quarter * 3 + 1;
        
        return date_time(boost::gregorian::date(target_year, target_month, 1));
    }

    inline date_time pre_quarter(const uint64_t n = 1)
    {
        int current_month = static_cast<int>(tm_.date().month());
        int current_quarter = (current_month - 1) / 3;  // 0, 1, 2, 3
        int target_quarter = current_quarter - n;
        int target_year = tm_.date().year();

        while (target_quarter < 0) 
        {
            target_quarter += 4;
            target_year--;
        }
        
        int target_month = target_quarter * 3 + 1;
        return date_time(boost::gregorian::date(target_year, target_month, 1));
    }

    inline date_time next_half_year(const uint64_t n = 1)
    {
        int current_month = static_cast<int>(tm_.date().month());
        int current_half = (current_month - 1) / 6;

        int target_half = current_half + n;
        int target_year = tm_.date().year() + (target_half / 2);

        target_half = target_half % 2;
        
        int target_month = target_half * 6 + 1;
        
        return date_time(boost::gregorian::date(target_year, target_month, 1));
    }

    inline date_time pre_half_year(const uint64_t n = 1)
    {
        int current_month = static_cast<int>(tm_.date().month());
        int current_half = (current_month - 1) / 6;
        
        int target_half = current_half - n;
        int target_year = tm_.date().year();
        
        while (target_half < 0) 
        {
            target_half += 2;
            target_year--;
        }
        
        int target_month = target_half * 6 + 1;
        
        return date_time(boost::gregorian::date(target_year, target_month, 1));
    }

    inline date_time next_year(const uint64_t n = 1)
    {
        int target_year = tm_.date().year() + n;
        return date_time(boost::gregorian::date(target_year, 1, 1));
    }

    inline date_time pre_year(const uint64_t n = 1)
    {
        int target_year = tm_.date().year() - n;
        return date_time(boost::gregorian::date(target_year, 1, 1));
    }

    inline void set_working_day_func(is_working_day_fn&& func)
    {
        working_day_func_ = std::move(func);
    }

private:
    boost::posix_time::ptime tm_;

    is_working_day_fn working_day_func_ = [](const libcpp::date_time& dt) -> bool {
        return static_cast<week_day>(int64_t(dt.tm_.date().day_of_week())) != week_day::saturday && 
                static_cast<week_day>(int64_t(dt.tm_.date().day_of_week())) != week_day::sunday;
    };
};

}

#endif