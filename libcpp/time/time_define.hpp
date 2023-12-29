#ifndef TIME_DEFINE_HPP
#define TIME_DEFINE_HPP

#include <ctime>

namespace libcpp
{

static constexpr char STD_TM_FMT[] = "%Y-%m-%d %H:%M:%S";

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

}

#endif