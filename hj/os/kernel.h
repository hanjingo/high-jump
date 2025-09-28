#ifndef KERNEL_H
#define KERNEL_H

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h> // windows.h must be included before other headers

#elif defined(__APPLE__)
#include <sys/utsname.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>

#elif defined(__linux__)
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <unistd.h>

#endif

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KERNEL_MAX_STRING_LEN 256

typedef struct
{
    char     name[64];
    char     version[128];
    uint64_t uptime_seconds;
} kernel_info_t;

// Get kernel name (e.g., "Linux", "Darwin", "Windows")
inline const char *kernel_name(void)
{
#if defined(_WIN32) || defined(_WIN64)
    return "Windows";

#elif defined(__APPLE__)
    return "Darwin";

#elif defined(__linux__)
    return "Linux";

#else
    return "Unknown";

#endif
}

// Get kernel version string
inline const char *kernel_version(char *buffer, size_t buffer_size)
{
    if(!buffer || buffer_size == 0)
        return NULL;

#if defined(_WIN32) || defined(_WIN64)
    OSVERSIONINFOEX info;
    memset(&info, 0, sizeof(info));
    info.dwOSVersionInfoSize = sizeof(info);
    if(GetVersionEx((OSVERSIONINFO *) &info))
    {
        snprintf(buffer,
                 buffer_size,
                 "%lu.%lu.%lu",
                 info.dwMajorVersion,
                 info.dwMinorVersion,
                 info.dwBuildNumber);
        return buffer;
    }
    strncpy(buffer, "Unknown", buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    return buffer;

#elif defined(__APPLE__) || defined(__linux__)
    struct utsname buf;
    if(uname(&buf) == 0)
    {
        strncpy(buffer, buf.release, buffer_size - 1);
        buffer[buffer_size - 1] = '\0';
        return buffer;
    }
    strncpy(buffer, "Unknown", buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    return buffer;

#else
    strncpy(buffer, "Unknown", buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    return buffer;

#endif
}

inline uint64_t kernel_uptime(void)
{
#if defined(_WIN32) || defined(_WIN64)
    return GetTickCount64() / 1000;

#elif defined(__APPLE__)
    struct timeval boottime;
    size_t         len    = sizeof(boottime);
    int            mib[2] = {CTL_KERN, KERN_BOOTTIME};
    time_t         now    = time(NULL);
    if(sysctl(mib, 2, &boottime, &len, NULL, 0) == 0 && boottime.tv_sec != 0)
        return (uint64_t) (now - boottime.tv_sec);

    return 0;

#elif defined(__linux__)
    struct sysinfo info;
    if(sysinfo(&info) == 0)
        return (uint64_t) info.uptime;

    return 0;

#else
    return 0;

#endif
}

inline const char *
kernel_uptime_str(char *buffer, size_t buffer_size, const char *fmt)
{
    if(!buffer || buffer_size == 0)
        return NULL;

    uint64_t uptime_sec = kernel_uptime();
    uint64_t days       = uptime_sec / (24 * 3600);
    uint64_t hours      = (uptime_sec % (24 * 3600)) / 3600;
    uint64_t minutes    = (uptime_sec % 3600) / 60;
    uint64_t seconds    = uptime_sec % 60;
    snprintf(buffer, buffer_size, fmt, days, hours, minutes, seconds);
    return buffer;
}

inline bool kernel_info(kernel_info_t *info)
{
    if(!info)
        return false;

    memset(info, 0, sizeof(kernel_info_t));
    const char *name = kernel_name();
    strncpy(info->name, name, sizeof(info->name) - 1);
    kernel_version(info->version, sizeof(info->version));
    info->uptime_seconds = kernel_uptime();
    return true;
}

#ifdef __cplusplus
}
#endif

#endif // KERNEL_H