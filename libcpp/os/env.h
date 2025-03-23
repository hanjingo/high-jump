#ifndef ENV_H
#define ENV_H

#include <stdlib.h>
#include <string.h>

// __DATE__:[Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sept, Oct, Nov, Dec] 2 2024
// __TIME__:17:37:21
#define COMPILE_YEAR ((((__DATE__ [7] - '0') * 10 + (__DATE__ [8] - '0')) * 10 \
    + (__DATE__ [9] - '0')) * 10 + (__DATE__ [10] - '0'))

#define COMPILE_MONTH (__DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? 1 : 6)  \
    : __DATE__ [2] == 'b' ? 2 \
    : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 3 : 4) \
    : __DATE__ [2] == 'y' ? 5 \
    : __DATE__ [2] == 'l' ? 7 \
    : __DATE__ [2] == 'g' ? 8 \
    : __DATE__ [2] == 'p' ? 9 \
    : __DATE__ [2] == 't' ? 10 \
    : __DATE__ [2] == 'v' ? 11 : 12)

#define COMPILE_DAY ((__DATE__ [4] == ' ' ? 0 : __DATE__ [4] - '0') * 10 \
    + (__DATE__ [5] - '0'))

#define COMPILE_HOUR ((__TIME__ [0] - '0') * 10 + (__TIME__ [1] - '0'))

#define COMPILE_MINUTE ((__TIME__ [3] - '0') * 10 + (__TIME__ [4] - '0'))

#define COMPILE_SECONDS ((__TIME__ [6] - '0') * 10 + (__TIME__ [7] - '0'))

#ifndef COMPILE_TIME_FMT
#define COMPILE_TIME_FMT "%d-%d-%d %d:%d:%d"
#endif
#ifndef COMPILE_TIME_LEN
#define COMPILE_TIME_LEN 22
#endif
#if !defined(_COMPILE_TIME) && !defined(COMPILE_TIME)
#define _COMPILE_TIME()\
({\
    static char _date_time_buf[COMPILE_TIME_LEN] = {0}; \
    memset(_date_time_buf, 0, COMPILE_TIME_LEN); \
    sprintf(_date_time_buf, COMPILE_TIME_FMT, COMPILE_YEAR, COMPILE_MONTH, COMPILE_DAY, COMPILE_HOUR, COMPILE_MINUTE, COMPILE_SECONDS); \
    _date_time_buf; \
})\

#define COMPILE_TIME _COMPILE_TIME()
#endif


#if (__cplusplus >= 201402L)
#if defined(_MSC_VER)
#define DEPRECATED(msg) __declspec(deprecated(msg))
#elif defined(__GNUC__)
#define DEPRECATED(msg) __attribute__((deprecated(msg)))
#else
#define DEPRECATED(msg) [[deprecated(msg)]]
#endif

#else
#define DEPRECATED(msg)
#endif

#endif