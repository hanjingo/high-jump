#ifndef ENV_H
#define ENV_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// __DATE__:[Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sept, Oct, Nov, Dec] 2 2024
// __TIME__:17:37:21
#define COMPILE_YEAR ( \
    (__DATE__ [7] - '0') * 1000 + \
    (__DATE__ [8] - '0') * 100 + \
    (__DATE__ [9] - '0') * 10 + \
    (__DATE__ [10] - '0'))

#define COMPILE_MONTH ( \
    __DATE__[0] == 'J' && __DATE__[1] == 'a' ? 1 : \
    __DATE__[0] == 'F' ? 2 : \
    __DATE__[0] == 'M' && __DATE__[2] == 'r' ? 3 : \
    __DATE__[0] == 'A' && __DATE__[1] == 'p' ? 4 : \
    __DATE__[0] == 'M' && __DATE__[2] == 'y' ? 5 : \
    __DATE__[0] == 'J' && __DATE__[2] == 'n' ? 6 : \
    __DATE__[0] == 'J' && __DATE__[2] == 'l' ? 7 : \
    __DATE__[0] == 'A' && __DATE__[1] == 'u' ? 8 : \
    __DATE__[0] == 'S' ? 9 : \
    __DATE__[0] == 'O' ? 10 : \
    __DATE__[0] == 'N' ? 11 : 12)

#define COMPILE_DAY ( \
    (__DATE__ [4] == ' ' ? 0 : __DATE__ [4] - '0') * 10 +\
    (__DATE__ [5] - '0'))

#define COMPILE_HOUR ((__TIME__[0] - '0') * 10 + (__TIME__[1] - '0'))

#define COMPILE_MINUTE ((__TIME__[3] - '0') * 10 + (__TIME__[4] - '0'))

#define COMPILE_SECOND ((__TIME__[6] - '0') * 10 + (__TIME__[7] - '0'))

#define COMPILE_TIME_FMT_ISO8601 "%04d-%02d-%02d %02d:%02d:%02d"
#ifndef COMPILE_TIME_FMT
#define COMPILE_TIME_FMT COMPILE_TIME_FMT_ISO8601
#endif
#ifndef COMPILE_TIME_LEN
#define COMPILE_TIME_LEN 20
#endif

inline const char* _COMPILE_TIME() {
    static char _date_time_buf[COMPILE_TIME_LEN] = {0};
    snprintf(_date_time_buf, COMPILE_TIME_LEN, COMPILE_TIME_FMT,
             COMPILE_YEAR, COMPILE_MONTH, COMPILE_DAY,
             COMPILE_HOUR, COMPILE_MINUTE, COMPILE_SECOND);
    return _date_time_buf;
}
#define COMPILE_TIME _COMPILE_TIME()


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


#if defined(QT_VERSION) && defined(QT_CORE_LIB)
    #define LIBCPP_QT_ENVIRONMENT 1

    #define LIBCPP_QT_VERSION QT_VERSION
    #define LIBCPP_QT_VERSION_MAJOR QT_VERSION_MAJOR
    #define LIBCPP_QT_VERSION_MINOR QT_VERSION_MINOR
    #define LIBCPP_QT_VERSION_PATCH QT_VERSION_PATCH

    #define LIBCPP_QT_VERSION_CHECK(major, minor, patch) \
        QT_VERSION_CHECK(major, minor, patch)
#else
    #define LIBCPP_QT_ENVIRONMENT 0
#endif

#endif