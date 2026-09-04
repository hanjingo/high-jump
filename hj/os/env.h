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

#ifndef ENV_H
#define ENV_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HJ_ENV_API
#if defined(HJ_ENV_STATIC)
#define HJ_ENV_API static inline
#else
#define HJ_ENV_API extern
#endif
#endif

/* -------------------------------------------------------------------------
 * TLS (Thread Local Storage) 
 * ------------------------------------------------------------------------- */
#ifndef HJ_TLS
#if defined(__cplusplus) && __cplusplus >= 201103L
#define HJ_TLS thread_local
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L                 \
    && !defined(__STDC_NO_THREADS__)
#define HJ_TLS _Thread_local
#elif defined(_MSC_VER)
#define HJ_TLS __declspec(thread)
#elif defined(__GNUC__) || defined(__clang__)
#define HJ_TLS __thread
#else
#define HJ_TLS
#endif
#endif

/* -------------------------------------------------------------------------
 * Compile time macros and handling
 * ------------------------------------------------------------------------- */
#define HJ_COMPILE_YEAR                                                        \
    ((__DATE__[7] - '0') * 1000 + (__DATE__[8] - '0') * 100                    \
     + (__DATE__[9] - '0') * 10 + (__DATE__[10] - '0'))

#define HJ_COMPILE_MONTH                                                       \
    (__DATE__[0] == 'J' && __DATE__[1] == 'a'   ? 1                            \
     : __DATE__[0] == 'F'                       ? 2                            \
     : __DATE__[0] == 'M' && __DATE__[2] == 'r' ? 3                            \
     : __DATE__[0] == 'A' && __DATE__[1] == 'p' ? 4                            \
     : __DATE__[0] == 'M' && __DATE__[2] == 'y' ? 5                            \
     : __DATE__[0] == 'J' && __DATE__[2] == 'n' ? 6                            \
     : __DATE__[0] == 'J' && __DATE__[2] == 'l' ? 7                            \
     : __DATE__[0] == 'A' && __DATE__[1] == 'u' ? 8                            \
     : __DATE__[0] == 'S'                       ? 9                            \
     : __DATE__[0] == 'O'                       ? 10                           \
     : __DATE__[0] == 'N'                       ? 11                           \
                                                : 12)

#define HJ_COMPILE_DAY                                                         \
    ((__DATE__[4] == ' ' ? 0 : __DATE__[4] - '0') * 10 + (__DATE__[5] - '0'))

#define HJ_COMPILE_HOUR ((__TIME__[0] - '0') * 10 + (__TIME__[1] - '0'))
#define HJ_COMPILE_MINUTE ((__TIME__[3] - '0') * 10 + (__TIME__[4] - '0'))
#define HJ_COMPILE_SECOND ((__TIME__[6] - '0') * 10 + (__TIME__[7] - '0'))

#define HJ_COMPILE_TIME_FMT_ISO8601 "%04d-%02d-%02d %02d:%02d:%02d"
#ifndef HJ_COMPILE_TIME_FMT
#define HJ_COMPILE_TIME_FMT HJ_COMPILE_TIME_FMT_ISO8601
#endif
#ifndef HJ_COMPILE_TIME_LEN
#define HJ_COMPILE_TIME_LEN 20
#endif

static inline const char *_HJ_COMPILE_TIME(void)
{
    static char _date_time_buf[HJ_COMPILE_TIME_LEN] = {0};
    if(_date_time_buf[0] == '\0')
    {
        snprintf(_date_time_buf,
                 HJ_COMPILE_TIME_LEN,
                 HJ_COMPILE_TIME_FMT,
                 HJ_COMPILE_YEAR,
                 HJ_COMPILE_MONTH,
                 HJ_COMPILE_DAY,
                 HJ_COMPILE_HOUR,
                 HJ_COMPILE_MINUTE,
                 HJ_COMPILE_SECOND);
    }
    return _date_time_buf;
}
#define HJ_COMPILE_TIME _HJ_COMPILE_TIME()

/* -------------------------------------------------------------------------
 * Operating system and architecture definitions
 * ------------------------------------------------------------------------- */
#if defined(_WIN32) || defined(_WIN64)
#define HJ_OS "windows"
#elif defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#define HJ_OS "ios"
#else
#define HJ_OS "macos"
#endif
#elif defined(__linux__)
#define HJ_OS "linux"
#elif defined(__ANDROID__)
#define HJ_OS "android"
#else
#define HJ_OS "unknown"
#endif

#if defined(_M_IX86) || defined(__i386__)
#define HJ_ARCH "x86"
#elif defined(_M_X64) || defined(__x86_64__)
#define HJ_ARCH "x64"
#elif defined(_M_ARM64) || defined(__aarch64__)
#define HJ_ARCH "arm64"
#elif defined(_M_ARM) || defined(__arm__)
#define HJ_ARCH "arm"
#elif defined(__loongarch__)
#define HJ_ARCH "loong64"
#else
#define HJ_ARCH "unknown"
#endif

/* -------------------------------------------------------------------------
 * Qt environment support
 * ------------------------------------------------------------------------- */
#if defined(QT_VERSION) && defined(QT_CORE_LIB)
#define HJ_QT_ENVIRONMENT 1
#define HJ_QT_VERSION QT_VERSION
#define HJ_QT_VERSION_MAJOR QT_VERSION_MAJOR
#define HJ_QT_VERSION_MINOR QT_VERSION_MINOR
#define HJ_QT_VERSION_PATCH QT_VERSION_PATCH
#define HJ_QT_VERSION_CHECK(major, minor, patch)                               \
    QT_VERSION_CHECK(major, minor, patch)
#else
#define HJ_QT_ENVIRONMENT 0
#endif

// ------------------------ ENV API Declarations ------------------------
typedef enum
{
    HJ_CONF_AIO_MAX,
    HJ_CONF_ARG_MAX,
    HJ_CONF_CLK_TCK,

    HJ_CONF_CPU_COUNT,

    HJ_CONF_FILE_SIZE_MAX,
    HJ_CONF_NAME_MAX,
    HJ_CONF_PRIMARY_GROUP_ID,
    HJ_CONF_HEAP_SIZE_MAX,
    HJ_CONF_HOST_NAME_MAX,
    HJ_CONF_LOGIN_NAME_MAX,
    HJ_CONF_MEMORY_PAGE_SIZE,
    HJ_CONF_MSG_QUEUE_SIZE_MAX,
    HJ_CONF_MSG_MAX,
    HJ_CONF_NGROUPS_MAX,
    HJ_CONF_OPEN_MAX,
    HJ_CONF_PATH_MAX,
    HJ_CONF_PIPE_BUF,
    HJ_CONF_SEM_NSEMS_MAX,
    HJ_CONF_SEM_VALUE_MAX,
    HJ_CONF_STACK_SIZE_MAX,
    HJ_CONF_SYMLOOP_MAX,
    HJ_CONF_TTY_NAME_MAX,
    HJ_CONF_USER_ID,
    HJ_CONF_VIRTUAL_MEMORY_MAX,

    HJ_CONF_MAX_COUNT
} hj_conf_t;

HJ_ENV_API void    hj_env_init(void);
HJ_ENV_API int64_t hj_env_get(hj_conf_t conf);

/* -------------------------------------------------------------------------
 * macros for convenient access to environment variables
 * ------------------------------------------------------------------------- */
#define HJ_ENV_AIO_MAX hj_env_get(HJ_CONF_AIO_MAX)
#define HJ_ENV_ARG_LEN_MAX hj_env_get(HJ_CONF_ARG_MAX)
#define HJ_ENV_CLOCK_TICKS_PER_SEC hj_env_get(HJ_CONF_CLK_TCK)

#define HJ_ENV_CPU_COUNT hj_env_get(HJ_CONF_CPU_COUNT)

#define HJ_ENV_FILE_SIZE_MAX hj_env_get(HJ_CONF_FILE_SIZE_MAX)
#define HJ_ENV_FILENAME_LEN_MAX hj_env_get(HJ_CONF_NAME_MAX)

#define HJ_ENV_PRIMARY_GROUP_ID hj_env_get(HJ_CONF_PRIMARY_GROUP_ID)
#define HJ_ENV_HEAP_SIZE_MAX hj_env_get(HJ_CONF_HEAP_SIZE_MAX)
#define HJ_ENV_HOSTNAME_LEN_MAX hj_env_get(HJ_CONF_HOST_NAME_MAX)
#define HJ_ENV_LOGIN_NAME_MAX hj_env_get(HJ_CONF_LOGIN_NAME_MAX)

#define HJ_ENV_MEMORY_PAGE_SIZE hj_env_get(HJ_CONF_MEMORY_PAGE_SIZE)

#define HJ_ENV_MSG_QUEUE_SIZE_MAX hj_env_get(HJ_CONF_MSG_QUEUE_SIZE_MAX)
#define HJ_ENV_MSG_MAX hj_env_get(HJ_CONF_MSG_MAX)

#define HJ_ENV_NGROUPS_MAX hj_env_get(HJ_CONF_NGROUPS_MAX)
#define HJ_ENV_OPEN_FILES_MAX hj_env_get(HJ_CONF_OPEN_MAX)
#define HJ_ENV_PATH_LEN_MAX hj_env_get(HJ_CONF_PATH_MAX)
#define HJ_ENV_PIPE_BUF hj_env_get(HJ_CONF_PIPE_BUF)

#define HJ_ENV_SEM_NSEMS_MAX hj_env_get(HJ_CONF_SEM_NSEMS_MAX)
#define HJ_ENV_SEM_VALUE_MAX hj_env_get(HJ_CONF_SEM_VALUE_MAX)

#define HJ_ENV_STACK_SIZE_MAX hj_env_get(HJ_CONF_STACK_SIZE_MAX)
#define HJ_ENV_SYMLINK_MAX hj_env_get(HJ_CONF_SYMLOOP_MAX)
#define HJ_ENV_TTY_NAME_MAX hj_env_get(HJ_CONF_TTY_NAME_MAX)
#define HJ_ENV_USER_ID hj_env_get(HJ_CONF_USER_ID)
#define HJ_ENV_VIRTUAL_MEMORY_MAX hj_env_get(HJ_CONF_VIRTUAL_MEMORY_MAX)

// --------------------- Implementation -------------------------
#if (defined(HJ_ENV_IMPL) || defined(HJ_ENV_STATIC))                           \
    && !defined(HJ_ENV_IMPL_DONE)
#define HJ_ENV_IMPL_DONE

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#if defined(_WIN32) || defined(_WIN64)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <psapi.h>
#include <Lmcons.h>
#include <io.h>
#include <stdio.h>
#pragma comment(lib, "psapi.lib")

#elif defined(__linux__)
#include <unistd.h>
#include <sys/resource.h>
#include <semaphore.h>

#elif defined(__APPLE__)
#include <unistd.h>
#include <sys/resource.h>
#include <sys/sysctl.h>

#else
#pragma message("Unknown OS, some env function will be disabled")

#endif

static HJ_TLS int64_t tls_hj_env_cache[HJ_CONF_MAX_COUNT];
static HJ_TLS int     tls_hj_env_initialized = 0;

HJ_ENV_API int64_t hj_env_fetch_uncached(hj_conf_t conf)
{
    if(conf < 0 || conf >= HJ_CONF_MAX_COUNT)
        return -1;

    switch(conf)
    {
        case HJ_CONF_AIO_MAX: {
#if defined(_WIN32) || defined(_WIN64)
            return -1;
#elif defined(__linux__) || defined(__APPLE__)
#ifdef _SC_AIO_MAX
            return sysconf(_SC_AIO_MAX);
#else
            return -1;
#endif
#else
            return -1;
#endif
        }

        case HJ_CONF_ARG_MAX: {
#if defined(_WIN32) || defined(_WIN64)
#ifdef ARG_MAX
            return ARG_MAX;
#elif defined(UNICODE_STRING_MAX_CHARS)
            return UNICODE_STRING_MAX_CHARS;
#else
            return -1;
#endif
#elif defined(__linux__) || defined(__APPLE__)
            return sysconf(_SC_ARG_MAX);
#else
            return -1;
#endif
        }

        case HJ_CONF_CLK_TCK: {
#if defined(_WIN32) || defined(_WIN64)
            LARGE_INTEGER freq;
            if(QueryPerformanceFrequency(&freq))
                return (int64_t) freq.QuadPart;
            return -1;
#elif defined(__linux__) || defined(__APPLE__)
            return sysconf(_SC_CLK_TCK);
#else
            return -1;
#endif
        }

        case HJ_CONF_CPU_COUNT: {
#if defined(_WIN32) || defined(_WIN64)
            SYSTEM_INFO sys_info;
            GetSystemInfo(&sys_info);
            return (int64_t) sys_info.dwNumberOfProcessors;
#elif defined(__linux__) || defined(__APPLE__)
            return sysconf(_SC_NPROCESSORS_ONLN);
#else
            return -1;
#endif
        }

        case HJ_CONF_FILE_SIZE_MAX: {
#if defined(_WIN32) || defined(_WIN64)
            return -1;
#elif defined(__linux__) || defined(__APPLE__)
            struct rlimit rlim;
            if(getrlimit(RLIMIT_FSIZE, &rlim) == 0)
                return (int64_t) rlim.rlim_max;
            return -1;
#else
            return -1;
#endif
        }

        case HJ_CONF_NAME_MAX: {
#if defined(_WIN32) || defined(_WIN64)
            char  volume_path[] = "C:\\";
            DWORD file_system_flags;
            DWORD max_component_length;
            if(GetVolumeInformationA(volume_path,
                                     NULL,
                                     0,
                                     NULL,
                                     &max_component_length,
                                     &file_system_flags,
                                     NULL,
                                     0))
                return (int64_t) max_component_length;
            return -1;
#elif defined(__linux__) || defined(__APPLE__)
#if defined(_SC_NAME_MAX)
            return sysconf(_SC_NAME_MAX);
#elif defined(_PC_NAME_MAX)
            {
                long v = pathconf("/", _PC_NAME_MAX);
                return v > 0 ? v : -1;
            }
#else
            return -1;
#endif
#else
            return -1;
#endif
        }

        case HJ_CONF_PRIMARY_GROUP_ID: {
#if defined(_WIN32) || defined(_WIN64)
            HANDLE               hToken;
            DWORD                dwLength = 0;
            PTOKEN_PRIMARY_GROUP pPrimaryGroup;
            if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
                return -1;
            GetTokenInformation(hToken, TokenPrimaryGroup, NULL, 0, &dwLength);
            if(dwLength <= 0)
            {
                CloseHandle(hToken);
                return -1;
            }
            pPrimaryGroup = (PTOKEN_PRIMARY_GROUP) malloc(dwLength);
            if(pPrimaryGroup
               && GetTokenInformation(hToken,
                                      TokenPrimaryGroup,
                                      pPrimaryGroup,
                                      dwLength,
                                      &dwLength))
            {
                PSID  pSid             = pPrimaryGroup->PrimaryGroup;
                DWORD dwSubAuthorities = *GetSidSubAuthorityCount(pSid);
                if(dwSubAuthorities > 0)
                {
                    DWORD rid = *GetSidSubAuthority(pSid, dwSubAuthorities - 1);
                    free(pPrimaryGroup);
                    CloseHandle(hToken);
                    return (int64_t) rid;
                }
            }
            if(pPrimaryGroup)
                free(pPrimaryGroup);
            CloseHandle(hToken);
            return -1;
#elif defined(__linux__) || defined(__APPLE__)
            return (int64_t) getgid();
#else
            return -1;
#endif
        }

        case HJ_CONF_HEAP_SIZE_MAX: {
#if defined(_WIN32) || defined(_WIN64)
            MEMORYSTATUSEX mem_status;
            mem_status.dwLength = sizeof(mem_status);
            if(GlobalMemoryStatusEx(&mem_status))
                return (int64_t) (mem_status.ullTotalVirtual);
            return -1;
#elif defined(__linux__) || defined(__APPLE__)
            struct rlimit rlim;
            if(getrlimit(RLIMIT_DATA, &rlim) == 0)
                return (int64_t) rlim.rlim_max;
            return -1;
#else
            return -1;
#endif
        }

        case HJ_CONF_HOST_NAME_MAX: {
#if defined(_WIN32) || defined(_WIN64)
#ifdef MAX_COMPUTERNAME_LENGTH
            return MAX_COMPUTERNAME_LENGTH + 1;
#else
            return -1;
#endif
#elif defined(__linux__) || defined(__APPLE__)
            return sysconf(_SC_HOST_NAME_MAX);
#else
            return -1;
#endif
        }

        case HJ_CONF_LOGIN_NAME_MAX: {
#if defined(_WIN32) || defined(_WIN64)
#ifdef UNLEN
            return UNLEN + 1;
#else
            return -1;
#endif
#elif defined(__linux__) || defined(__APPLE__)
            return sysconf(_SC_LOGIN_NAME_MAX);
#else
            return -1;
#endif
        }

        case HJ_CONF_MEMORY_PAGE_SIZE: {
#if defined(_WIN32) || defined(_WIN64)
            SYSTEM_INFO sys_info;
            GetSystemInfo(&sys_info);
            return (int64_t) sys_info.dwPageSize;
#elif defined(__linux__) || defined(__APPLE__)
            return sysconf(_SC_PAGESIZE);
#else
            return -1;
#endif
        }

        case HJ_CONF_MSG_QUEUE_SIZE_MAX: {
#if defined(_WIN32) || defined(_WIN64)
            return -1;
#elif defined(__linux__)
            FILE *f = fopen("/proc/sys/fs/mqueue/msg_max", "r");
            int   max_msgs;
            if(!f)
                return -1;
            if(fscanf(f, "%d", &max_msgs) == 1)
            {
                fclose(f);
                return max_msgs;
            }
            fclose(f);
            return -1;
#else
            return -1;
#endif
        }

        case HJ_CONF_MSG_MAX: {
#if defined(_WIN32) || defined(_WIN64)
            return -1;
#elif defined(__linux__)
            FILE *f = fopen("/proc/sys/fs/mqueue/msgsize_max", "r");
            int   max_size;
            if(!f)
                return -1;
            if(fscanf(f, "%d", &max_size) == 1)
            {
                fclose(f);
                return max_size;
            }
            fclose(f);
            return -1;
#else
            return -1;
#endif
        }

        case HJ_CONF_NGROUPS_MAX: {
#if defined(_WIN32) || defined(_WIN64)
            return -1;
#elif defined(__linux__) || defined(__APPLE__)
            return sysconf(_SC_NGROUPS_MAX);
#else
            return -1;
#endif
        }

        case HJ_CONF_OPEN_MAX: {
#if defined(_WIN32) || defined(_WIN64)
            return _getmaxstdio();
#elif defined(__linux__) || defined(__APPLE__)
            return sysconf(_SC_OPEN_MAX);
#else
            return -1;
#endif
        }

        case HJ_CONF_PATH_MAX: {
#if defined(_WIN32) || defined(_WIN64)
#ifdef MAX_PATH
            return MAX_PATH;
#else
            return -1;
#endif
#elif defined(__linux__) || defined(__APPLE__)
#if defined(_SC_PATH_MAX)
            return sysconf(_SC_PATH_MAX);
#elif defined(_PC_PATH_MAX)
            long v = pathconf("/", _PC_PATH_MAX);
            return v > 0 ? v : -1;
#else
            return -1;
#endif
#else
            return -1;
#endif
        }

        case HJ_CONF_PIPE_BUF: {
#if defined(_WIN32) || defined(_WIN64)
            return -1;
#elif defined(__linux__) || defined(__APPLE__)
#if defined(_SC_PIPE_BUF)
            return sysconf(_SC_PIPE_BUF);
#elif defined(_PC_PIPE_BUF)
            {
                long v = pathconf("/", _PC_PIPE_BUF);
                return v > 0 ? v : -1;
            }
#else
            return -1;
#endif
#else
            return -1;
#endif
        }

        case HJ_CONF_SEM_NSEMS_MAX: {
#if defined(_WIN32) || defined(_WIN64)
            return LONG_MAX;
#elif defined(__linux__)
            {
                FILE *f = fopen("/proc/sys/kernel/sem", "r");
                int   semmsl, semmns, semopm, semmni;
                if(!f)
                    return -1;
                if(fscanf(f, "%d %d %d %d", &semmsl, &semmns, &semopm, &semmni)
                   == 4)
                {
                    fclose(f);
                    return semmsl;
                }
                fclose(f);
                return -1;
            }
#else
            return -1;
#endif
        }

        case HJ_CONF_SEM_VALUE_MAX: {
#if defined(_WIN32) || defined(_WIN64)
            return LONG_MAX;
#elif defined(__linux__) || defined(__APPLE__)
#ifdef SEM_VALUE_MAX
            return SEM_VALUE_MAX;
#else
            return -1;
#endif
#else
            return -1;
#endif
        }

        case HJ_CONF_STACK_SIZE_MAX: {
#if defined(_WIN32) || defined(_WIN64)
            {
                MEMORY_BASIC_INFORMATION mbi;
                volatile int             stack_var = 0;
                if(VirtualQuery((LPCVOID) &stack_var, &mbi, sizeof(mbi)) != 0
                   && mbi.AllocationBase != NULL)
                {
                    MEMORY_BASIC_INFORMATION alloc_mbi;
                    if(VirtualQuery(mbi.AllocationBase,
                                    &alloc_mbi,
                                    sizeof(alloc_mbi))
                       != 0)
                    {
                        PVOID  base                = mbi.AllocationBase;
                        SIZE_T total_stack_reserve = 0;
                        while(VirtualQuery(base, &alloc_mbi, sizeof(alloc_mbi))
                              != 0)
                        {
                            if(alloc_mbi.AllocationBase != mbi.AllocationBase)
                                break;
                            total_stack_reserve += alloc_mbi.RegionSize;
                            base = (BYTE *) base + alloc_mbi.RegionSize;
                        }
                        if(total_stack_reserve > 0)
                            return (int64_t) total_stack_reserve;
                    }
                }
                return -1;
            }
#elif defined(__linux__) || defined(__APPLE__)
            {
                struct rlimit rlim;
                if(getrlimit(RLIMIT_STACK, &rlim) == 0)
                    return (int64_t) rlim.rlim_max;
                return -1;
            }
#else
            return -1;
#endif
        }

        case HJ_CONF_SYMLOOP_MAX: {
#if defined(_WIN32) || defined(_WIN64)
#ifdef MAX_PATH
            return MAX_PATH;
#else
            return -1;
#endif
#elif defined(__linux__) || defined(__APPLE__)
            return sysconf(_SC_SYMLOOP_MAX);
#else
            return -1;
#endif
        }

        case HJ_CONF_TTY_NAME_MAX: {
#if defined(_WIN32) || defined(_WIN64)
            return -1;
#elif defined(__linux__) || defined(__APPLE__)
            return sysconf(_SC_TTY_NAME_MAX);
#else
            return -1;
#endif
        }

        case HJ_CONF_USER_ID: {
#if defined(_WIN32) || defined(_WIN64)
            {
                HANDLE      hToken;
                DWORD       dwLength = 0;
                PTOKEN_USER pTokenUser;
                if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
                    return -1;
                GetTokenInformation(hToken, TokenUser, NULL, 0, &dwLength);
                if(dwLength <= 0)
                {
                    CloseHandle(hToken);
                    return -1;
                }
                pTokenUser = (PTOKEN_USER) malloc(dwLength);
                if(pTokenUser
                   && GetTokenInformation(hToken,
                                          TokenUser,
                                          pTokenUser,
                                          dwLength,
                                          &dwLength))
                {
                    PSID  pSid             = pTokenUser->User.Sid;
                    DWORD dwSubAuthorities = *GetSidSubAuthorityCount(pSid);
                    if(dwSubAuthorities > 0)
                    {
                        DWORD rid =
                            *GetSidSubAuthority(pSid, dwSubAuthorities - 1);
                        free(pTokenUser);
                        CloseHandle(hToken);
                        return (int64_t) rid;
                    }
                }
                if(pTokenUser)
                    free(pTokenUser);
                CloseHandle(hToken);
                return -1;
            }
#elif defined(__linux__) || defined(__APPLE__)
            return (int64_t) getuid();
#else
            return -1;
#endif
        }

        case HJ_CONF_VIRTUAL_MEMORY_MAX: {
#if defined(_WIN32) || defined(_WIN64)
            MEMORYSTATUSEX mem_status;
            mem_status.dwLength = sizeof(mem_status);
            if(GlobalMemoryStatusEx(&mem_status))
                return (int64_t) mem_status.ullTotalVirtual;
            return -1;
#elif defined(__linux__) || defined(__APPLE__)
            struct rlimit rlim;
            if(getrlimit(RLIMIT_AS, &rlim) == 0)
                return (int64_t) rlim.rlim_max;
            return -1;
#else
            return -1;
#endif
        }
    }

    return -1; // Invalid conf value
}

HJ_ENV_API void hj_env_init(void)
{
    if(tls_hj_env_initialized)
        return;

    for(int i = 0; i < HJ_CONF_MAX_COUNT; ++i)
    {
        tls_hj_env_cache[i] = hj_env_fetch_uncached((hj_conf_t) i);
    }

    tls_hj_env_initialized = 1;
}

HJ_ENV_API int64_t hj_env_get(hj_conf_t conf)
{
    if(conf < 0 || conf >= HJ_CONF_MAX_COUNT)
        return -1;

    if(!tls_hj_env_initialized)
        hj_env_init();

    return tls_hj_env_cache[conf];
}

#endif /* HJ_ENV_IMPL || HJ_ENV_STATIC */

#ifdef __cplusplus
}
#endif

#endif /* ENV_H */