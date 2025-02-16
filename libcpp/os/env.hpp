#ifndef ENV_HPP
#define ENV_HPP

#include <stdlib.h>
#include <cstring>

#if defined(WIN32)
#define __WINDOWS__
#endif

// __DATE__:[Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sept, Oct, Nov, Dec] 2 2024
// __TIME__:17:37:21
#define YEAR ((((__DATE__ [7] - '0') * 10 + (__DATE__ [8] - '0')) * 10 \
    + (__DATE__ [9] - '0')) * 10 + (__DATE__ [10] - '0'))

#define MONTH (__DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? 1 : 6)  \
    : __DATE__ [2] == 'b' ? 2 \
    : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 3 : 4) \
    : __DATE__ [2] == 'y' ? 5 \
    : __DATE__ [2] == 'l' ? 7 \
    : __DATE__ [2] == 'g' ? 8 \
    : __DATE__ [2] == 'p' ? 9 \
    : __DATE__ [2] == 't' ? 10 \
    : __DATE__ [2] == 'v' ? 11 : 12)

#define DAY ((__DATE__ [4] == ' ' ? 0 : __DATE__ [4] - '0') * 10 \
    + (__DATE__ [5] - '0'))

#define HOUR ((__TIME__ [0] - '0') * 10 + (__TIME__ [1] - '0'))

#define MINUTE ((__TIME__ [3] - '0') * 10 + (__TIME__ [4] - '0'))

#define SECONDS ((__TIME__ [6] - '0') * 10 + (__TIME__ [7] - '0'))

#if defined(_MSC_VER) //  MSVC
#define DATE_TIME 
// TODO
#elif defined(__GNUC__) //  GCC
#define _DATE_TIME()\
({\
    static char _date_time_buf[17] = {0}; \
    memset(_date_time_buf, 0, 17); \
    sprintf(_date_time_buf, "%d-%d-%d %d:%d:%d", YEAR, MONTH, DAY, HOUR, MINUTE, SECONDS); \
    _date_time_buf; \
})\

#define DATE_TIME _DATE_TIME()
#else
#endif

namespace libcpp
{

char* getenv(const char* key)
{
    return std::getenv(key);
}

}

#endif