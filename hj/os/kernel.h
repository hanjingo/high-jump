/*
 * This file is part of high-jump(hj).
 *
 * Copyright 2025 hanjingo <hehehunanchina@live.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HJ_KERNEL_API
#if defined(HJ_KERNEL_STATIC)
#define HJ_KERNEL_API static inline
#else
#define HJ_KERNEL_API extern
#endif
#endif

#ifndef HJ_KERNEL_MAX_STRING_LEN
#define HJ_KERNEL_MAX_STRING_LEN 256
#endif

typedef struct
{
    char     name[64];
    char     version[128];
    uint64_t uptime_seconds;
} kernel_info_t;

// ------------------------ Kernel API Declarations ------------------------
HJ_KERNEL_API const char *hj_kernel_name(void);
HJ_KERNEL_API const char *hj_kernel_version(char *buffer, size_t buffer_size);
HJ_KERNEL_API uint64_t    hj_kernel_uptime(void);
HJ_KERNEL_API const char *
hj_kernel_uptime_str(char *buffer, size_t buffer_size, const char *fmt);
HJ_KERNEL_API bool hj_kernel_info(kernel_info_t *info);

#ifdef __cplusplus
}
#endif

#endif // KERNEL_H


// --------------------- Implementation -------------------------
// To include implementation, define HJ_KERNEL_IMPL before including
// this header in ONE C/C++ source file.
#if (defined(HJ_KERNEL_IMPL) || defined(HJ_KERNEL_STATIC))                     \
    && !defined(HJ_KERNEL_IMPL_DONE)
#define HJ_KERNEL_IMPL_DONE

#if defined(_WIN32) || defined(_WIN64)
#ifndef NOMINMAX
#define NOMINMAX
#endif
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// Get kernel name (e.g., "Linux", "Darwin", "Windows")
HJ_KERNEL_API const char *hj_kernel_name(void)
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
HJ_KERNEL_API const char *hj_kernel_version(char *buffer, size_t buffer_size)
{
    if(!buffer || buffer_size == 0)
        return NULL;

#if defined(_WIN32) || defined(_WIN64)
    typedef LONG(WINAPI * RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
    HMODULE hMod = GetModuleHandleA("ntdll.dll");
    if(hMod)
    {
        RtlGetVersionPtr fxRtlGetVersion =
            (RtlGetVersionPtr) GetProcAddress(hMod, "RtlGetVersion");
        if(fxRtlGetVersion)
        {
            RTL_OSVERSIONINFOW rovi;
            memset(&rovi, 0, sizeof(rovi));
            rovi.dwOSVersionInfoSize = sizeof(rovi);
            if(fxRtlGetVersion(&rovi) == 0) // STATUS_SUCCESS
            {
                snprintf(buffer,
                         buffer_size,
                         "%lu.%lu.%lu",
                         rovi.dwMajorVersion,
                         rovi.dwMinorVersion,
                         rovi.dwBuildNumber);
                return buffer;
            }
        }
    }
    snprintf(buffer, buffer_size, "Unknown");
    return buffer;

#elif defined(__APPLE__) || defined(__linux__)
    struct utsname buf;
    if(uname(&buf) == 0)
    {
        snprintf(buffer, buffer_size, "%s", buf.release);
        return buffer;
    }
    snprintf(buffer, buffer_size, "Unknown");
    return buffer;

#else
    snprintf(buffer, buffer_size, "Unknown");
    return buffer;

#endif
}

HJ_KERNEL_API uint64_t hj_kernel_uptime(void)
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

HJ_KERNEL_API const char *
hj_kernel_uptime_str(char *buffer, size_t buffer_size, const char *fmt)
{
    if(!buffer || buffer_size == 0 || !fmt)
        return NULL;

    uint64_t           uptime_sec = hj_kernel_uptime();
    unsigned long long days = (unsigned long long) (uptime_sec / (24 * 3600));
    unsigned long long hours =
        (unsigned long long) ((uptime_sec % (24 * 3600)) / 3600);
    unsigned long long minutes =
        (unsigned long long) ((uptime_sec % 3600) / 60);
    unsigned long long seconds = (unsigned long long) (uptime_sec % 60);

    snprintf(buffer, buffer_size, fmt, days, hours, minutes, seconds);
    return buffer;
}

HJ_KERNEL_API bool hj_kernel_info(kernel_info_t *info)
{
    if(!info)
        return false;

    memset(info, 0, sizeof(kernel_info_t));
    snprintf(info->name, sizeof(info->name), "%s", hj_kernel_name());
    hj_kernel_version(info->version, sizeof(info->version));
    info->uptime_seconds = hj_kernel_uptime();
    return true;
}

#ifdef __cplusplus
}
#endif

#endif // HJ_KERNEL_IMPL && !HJ_KERNEL_IMPL_DONE