/*
 *  This file is part of hj.
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #include <psapi.h>
    #include <pdh.h>
    #include <Lmcons.h>
    #include <tlhelp32.h>
    #include <iphlpapi.h>
    #include <io.h>
    #include <intrin.h>
    #pragma comment(lib, "psapi.lib")
    #pragma comment(lib, "pdh.lib")
    #pragma comment(lib, "iphlpapi.lib")
#elif defined(__linux__)
    #include <unistd.h>
    #include <sys/resource.h>
    #include <sys/sysinfo.h>
    #include <sys/statvfs.h>
    #include <sys/utsname.h>
    #include <sys/time.h>
    #include <sys/socket.h>
    #include <ifaddrs.h>
    #include <semaphore.h>
    #include <mqueue.h>
    #include <aio.h>
    #include <pthread.h>
    #include <sys/syscall.h>
#elif defined(__APPLE__)
    #include <unistd.h>
    #include <sys/resource.h>
    #include <sys/sysctl.h>
    #include <sys/mount.h>
    #include <sys/time.h>
    #include <sys/socket.h>
    #include <mach/mach.h>
    #include <mach/vm_statistics.h>
    #include <mach/mach_types.h>
    #include <mach/mach_init.h>
    #include <mach/mach_host.h>
    #include <ifaddrs.h>
    #include <pthread.h>
#else
    #pragma message("Unknown OS, some env function will be disabled")
#endif

#ifdef __cplusplus
extern "C" {
#endif

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

static inline const char* _COMPILE_TIME() {
    static char _date_time_buf[COMPILE_TIME_LEN] = {0};
    snprintf(_date_time_buf, COMPILE_TIME_LEN, COMPILE_TIME_FMT,
             COMPILE_YEAR, COMPILE_MONTH, COMPILE_DAY,
             COMPILE_HOUR, COMPILE_MINUTE, COMPILE_SECOND);
    return _date_time_buf;
}
#define COMPILE_TIME _COMPILE_TIME()


// QT
#if defined(QT_VERSION) && defined(QT_CORE_LIB)
    #define ENV_QT_ENVIRONMENT   1
    #define ENV_QT_VERSION       QT_VERSION
    #define ENV_QT_VERSION_MAJOR QT_VERSION_MAJOR
    #define ENV_QT_VERSION_MINOR QT_VERSION_MINOR
    #define ENV_QT_VERSION_PATCH QT_VERSION_PATCH
    #define ENV_QT_VERSION_CHECK(major, minor, patch) \
        QT_VERSION_CHECK(major, minor, patch)
#else
    #define ENV_QT_ENVIRONMENT 0
#endif


// env var
typedef enum 
{
    CONF_AIO_MAX,

    CONF_ARG_MAX,

    CONF_BOOT_TIME,

    CONF_CLK_TCK,

    CONF_CPU_COUNT,
    CONF_CPU_CACHE_LINE_SIZE,
    CONF_CPU_FREQUENCY,
    CONF_CPU_L1_CACHE_SIZE,
    CONF_CPU_L2_CACHE_SIZE,
    CONF_CPU_L3_CACHE_SIZE,

    CONF_FILE_SIZE_MAX,
    CONF_NAME_MAX,

    CONF_GETGR_R_SIZE_MAX,
    CONF_GETPW_R_SIZE_MAX,

    CONF_GROUP_ID,

    CONF_HEAP_SIZE_MAX,

    CONF_HOST_NAME_MAX,

    CONF_LOGIN_NAME_MAX,

    CONF_MEMORY_TOTAL,
    CONF_MEMORY_PAGE_SIZE,

    CONF_MSG_QUEUE_SIZE_MAX,
    CONF_MSG_MAX,

    CONF_NETWORK_INTERFACES,

    CONF_NGROUPS_MAX,

    CONF_OPEN_MAX,

    CONF_PATH_MAX,

    CONF_PIPE_BUF,

    CONF_PROCESS_CHILD_MAX,

    CONF_SEM_NSEMS_MAX,
    CONF_SEM_VALUE_MAX,

    CONF_STACK_SIZE_MAX,

    CONF_SWAP_SIZE_TOTAL,

    CONF_SYMLOOP_MAX,

    CONF_THREADS_MAX,

    CONF_TIMEZONE_OFFSET, // (minute)

    CONF_TTY_NAME_MAX,

    CONF_UPTIME_SECONDS,
    
    CONF_USER_ID,
    
    CONF_VIRTUAL_MEMORY_MAX
} conf_t;

static int64_t env_get(conf_t conf)
{
#if defined(_WIN32) || defined(_WIN64)
    SYSTEM_INFO sys_info;
    MEMORYSTATUSEX mem_status;
    DWORD result;
    switch (conf) 
    {
        case CONF_AIO_MAX:
        {
            return 1024;
        }
        case CONF_ARG_MAX:
        {
            return 32767;
        }
        case CONF_BOOT_TIME:
        {
            return (int64_t)((GetTickCount64() / 1000) - (time(NULL) - GetTickCount64() / 1000));
        }
        case CONF_CLK_TCK:
        {
            LARGE_INTEGER freq;
            if (QueryPerformanceFrequency(&freq))
                return (int64_t)freq.QuadPart;
            return 1000;
        }
        case CONF_CPU_COUNT:
        {
            GetSystemInfo(&sys_info);
            return (int64_t)sys_info.dwNumberOfProcessors;
        }
        case CONF_CPU_CACHE_LINE_SIZE:
        {
            GetSystemInfo(&sys_info);
            return (int64_t)sys_info.dwPageSize;
        }
        case CONF_CPU_FREQUENCY:
        {
            HKEY hKey;
            DWORD dwMHz = 0;
            DWORD dwSize = sizeof(dwMHz);
            if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
                             "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                             0, 
                             KEY_READ, 
                             &hKey) == ERROR_SUCCESS) 
            {
                RegQueryValueExA(hKey, "~MHz", NULL, NULL, (LPBYTE)&dwMHz, &dwSize);
                RegCloseKey(hKey);
                return (int64_t)dwMHz;
            }
            return -1;
        }
        case CONF_CPU_L1_CACHE_SIZE:
        {
            int cpuInfo[4];
            __cpuid(cpuInfo, 0x80000005);
            if (cpuInfo[0] >= 0x80000005)
                return (int64_t)((cpuInfo[2] >> 24) * 1024);
            return -1;
        }
        case CONF_CPU_L2_CACHE_SIZE:
        {
            int cpuInfo[4];
            __cpuid(cpuInfo, 0x80000006);
            if (cpuInfo[0] >= 0x80000006) 
                return (int64_t)((cpuInfo[2] >> 16) * 1024);
            return -1;
        }
        case CONF_CPU_L3_CACHE_SIZE:
        {
            int cpuInfo[4];
            __cpuid(cpuInfo, 0x80000006);
            if (cpuInfo[0] >= 0x80000006)
                return (int64_t)((cpuInfo[3] >> 18) * 512 * 1024);
            return -1;
        }
        case CONF_FILE_SIZE_MAX: 
        {
            return (int64_t)(16ULL * 1024 * 1024 * 1024 * 1024); // 16TB
        }
        case CONF_NAME_MAX:
        {
            char volume_path[] = "C:\\";
            DWORD file_system_flags;
            DWORD max_component_length;
            if (GetVolumeInformationA(volume_path, 
                                      NULL, 
                                      0, 
                                      NULL, 
                                      &max_component_length, 
                                      &file_system_flags, 
                                      NULL, 
                                      0))
                return (int64_t)max_component_length;
            return -1;
        }
        case CONF_GETGR_R_SIZE_MAX:
        {
            return 4096;
        }
        case CONF_GETPW_R_SIZE_MAX:
        {
            return 4096;
        }
        case CONF_GROUP_ID:
        {
            HANDLE hToken;
            if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) 
                return -1;
            DWORD dwLength = 0;
            GetTokenInformation(hToken, TokenGroups, NULL, 0, &dwLength);
            if (dwLength <= 0)
            {
                CloseHandle(hToken);
                return -1;
            }
            PTOKEN_GROUPS pTokenGroups = (PTOKEN_GROUPS)malloc(dwLength);
            if (pTokenGroups 
                    && GetTokenInformation(hToken, TokenGroups, pTokenGroups, dwLength, &dwLength)) 
            {
                if (pTokenGroups->GroupCount <= 0)
                {
                    PSID pSid = pTokenGroups->Groups[0].Sid;
                    DWORD dwSubAuthorities = *GetSidSubAuthorityCount(pSid);
                    if (dwSubAuthorities > 0) 
                    {
                        DWORD rid = *GetSidSubAuthority(pSid, dwSubAuthorities - 1);
                        free(pTokenGroups);
                        CloseHandle(hToken);
                        return (int64_t)rid;
                    }
                }
            }
            if (pTokenGroups) 
                free(pTokenGroups);
            CloseHandle(hToken);
            return -1;
        }
        case CONF_HEAP_SIZE_MAX: 
        {
            mem_status.dwLength = sizeof(mem_status);
            if (GlobalMemoryStatusEx(&mem_status)) 
                return (int64_t)(mem_status.ullTotalVirtual);
            return -1;
        }
        case CONF_HOST_NAME_MAX:
        {
            return MAX_COMPUTERNAME_LENGTH + 1;
        }
        case CONF_LOGIN_NAME_MAX:
        {
            return UNLEN + 1;
        }
        case CONF_MEMORY_TOTAL:
        {
            mem_status.dwLength = sizeof(mem_status);
            if (GlobalMemoryStatusEx(&mem_status))
                return (int64_t)(mem_status.ullTotalPhys);
            return -1;
        }
        case CONF_MEMORY_PAGE_SIZE:
        {
            GetSystemInfo(&sys_info);
            return (int64_t)sys_info.dwPageSize;
        }
        case CONF_MSG_QUEUE_SIZE_MAX:
        {
            return 1024 * 1024;
        }
        case CONF_MSG_MAX:
        {
            return 65536;
        }
        case CONF_NETWORK_INTERFACES:
        {
            DWORD dwSize = 0;
            if (GetAdaptersInfo(NULL, &dwSize) != ERROR_BUFFER_OVERFLOW) 
                return -1;
            PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO*)malloc(dwSize);
            if (pAdapterInfo && GetAdaptersInfo(pAdapterInfo, &dwSize) == NO_ERROR) 
            {
                int count = 0;
                PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
                while (pAdapter) 
                {
                    count++;
                    pAdapter = pAdapter->Next;
                }
                free(pAdapterInfo);
                return count;
            }
            if (pAdapterInfo) 
                free(pAdapterInfo);
            return -1;
        }
        case CONF_NGROUPS_MAX:
        {
            return 1024;
        }
        case CONF_OPEN_MAX:
        {
            return _getmaxstdio();
        }
        case CONF_PATH_MAX:
        {
            return MAX_PATH;
        }
        case CONF_PIPE_BUF:
        {
            return 4096;
        }
        case CONF_PROCESS_CHILD_MAX:
        {
            HANDLE hProcess = GetCurrentProcess();
            PROCESS_MEMORY_COUNTERS pmc;
            if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
                return 32768;
            return 2048;
        }
        case CONF_SEM_NSEMS_MAX:
        {
            return 32767;
        }
        case CONF_SEM_VALUE_MAX:
        {
            return LONG_MAX;
        }
        case CONF_STACK_SIZE_MAX:
        {
            HANDLE hThread = GetCurrentThread();
            MEMORY_BASIC_INFORMATION mbi;
            if (VirtualQuery(&mbi, &mbi, sizeof(mbi)) != 0)
                return (int64_t)mbi.RegionSize;
            return 1024 * 1024; // 1MB
        }
        case CONF_SWAP_SIZE_TOTAL:
        {
            mem_status.dwLength = sizeof(mem_status);
            if (GlobalMemoryStatusEx(&mem_status))
                return (int64_t)(mem_status.ullTotalPageFile - mem_status.ullTotalPhys);
            return -1;
        }
        case CONF_SYMLOOP_MAX:
        {
            return MAX_PATH;
        }
        case CONF_THREADS_MAX:
        {
            GetSystemInfo(&sys_info);
            mem_status.dwLength = sizeof(mem_status);
            if (GlobalMemoryStatusEx(&mem_status))
                return (int64_t)(mem_status.ullTotalVirtual / (1024 * 1024));
            return 2048;
        }
        case CONF_TIMEZONE_OFFSET:
        {
            TIME_ZONE_INFORMATION tzi;
            if (GetTimeZoneInformation(&tzi) != TIME_ZONE_ID_INVALID)
                return (int64_t)(-tzi.Bias);
            return 0;
        }
        case CONF_TTY_NAME_MAX:
        {
            return 32;
        }
        case CONF_UPTIME_SECONDS:
        {
            return GetTickCount64() / 1000;
        }
        case CONF_USER_ID:
        {
            HANDLE hToken;
            if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
                return -1;
            DWORD dwLength = 0;
            GetTokenInformation(hToken, TokenUser, NULL, 0, &dwLength);
            if (dwLength <= 0)
            {
                CloseHandle(hToken);
                return -1;
            }
            PTOKEN_USER pTokenUser = (PTOKEN_USER)malloc(dwLength);
            if (pTokenUser && GetTokenInformation(hToken, TokenUser, pTokenUser, dwLength, &dwLength)) 
            {
                PSID pSid = pTokenUser->User.Sid;
                DWORD dwSubAuthorities = *GetSidSubAuthorityCount(pSid);
                if (dwSubAuthorities > 0) 
                {
                    DWORD rid = *GetSidSubAuthority(pSid, dwSubAuthorities - 1);
                    free(pTokenUser);
                    CloseHandle(hToken);
                    return (int64_t)rid;
                }
            }
            if (pTokenUser) 
                free(pTokenUser);
            CloseHandle(hToken);
            return -1;
        }
        case CONF_VIRTUAL_MEMORY_MAX:
        {
            mem_status.dwLength = sizeof(mem_status);
            if (GlobalMemoryStatusEx(&mem_status)) 
                return (int64_t)mem_status.ullTotalVirtual;
            return -1;
        }
        default:
            return -1;
    }

#elif defined(__linux__) || defined(__APPLE__)
    switch (conf)
    {
        case CONF_AIO_MAX:
        {
#ifdef _SC_AIO_MAX
            return sysconf(_SC_AIO_MAX);
#else
            return 1024;
#endif
        }
        case CONF_ARG_MAX: 
        {
            return sysconf(_SC_ARG_MAX);
        }
        case CONF_BOOT_TIME:
        {
#ifdef __linux__
            struct sysinfo info;
            if (sysinfo(&info) == 0)
                return (int64_t)(time(NULL) - info.uptime);
#elif defined(__APPLE__)
            struct timeval boottime;
            size_t size = sizeof(boottime);
            int mib[2] = {CTL_KERN, KERN_BOOTTIME};
            if (sysctl(mib, 2, &boottime, &size, NULL, 0) == 0) 
                return (int64_t)boottime.tv_sec;
#endif
            return -1;
        }
        case CONF_CLK_TCK: 
        {
            return sysconf(_SC_CLK_TCK);
        }
        case CONF_CPU_COUNT: 
        {
            return sysconf(_SC_NPROCESSORS_ONLN);
        }
        case CONF_CPU_CACHE_LINE_SIZE:
        {
            #ifdef __linux__
            FILE *f = fopen("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");
            if (!f) 
                return -1;
            int size;
            if (fscanf(f, "%d", &size) == 1)
            {
                fclose(f);
                return (int64_t)size;
            }
            fclose(f);
#elif defined(__APPLE__)
            size_t size = sizeof(int);
            int cache_line_size;
            if (sysctlbyname("hw.cachelinesize", &cache_line_size, &size, NULL, 0) == 0)
                return (int64_t)cache_line_size;
#endif
            return -1;
        }
        case CONF_CPU_FREQUENCY:
        {
#ifdef __linux__
            FILE *f = fopen("/proc/cpuinfo", "r");
            if (!f)
                return -1;
            double mhz;
            char line[256];
            while (fgets(line, sizeof(line), f)) 
            {
                if (strncmp(line, "cpu MHz", 7) != 0)
                    continue;
                if (sscanf(line, "cpu MHz : %lf", &mhz) != 1)
                    continue;
                fclose(f);
                return (int64_t)mhz;
            }
            fclose(f);
#elif defined(__APPLE__)
            uint64_t freq;
            size_t size = sizeof(freq);
            if (sysctlbyname("hw.cpufrequency", &freq, &size, NULL, 0) == 0)
                return (int64_t)(freq / 1000000);
#endif
            return -1;
        }
        case CONF_CPU_L1_CACHE_SIZE:
        {
#ifdef __linux__
            FILE *f = fopen("/sys/devices/system/cpu/cpu0/cache/index0/size", "r");
            if (!f)
                return -1;
            char size_str[32];
            if (fgets(size_str, sizeof(size_str), f)) 
            {
                int size = atoi(size_str);
                char unit = size_str[strlen(size_str) - 2];
                fclose(f);
                if (unit == 'K') 
                    return (int64_t)(size * 1024);
                if (unit == 'M') 
                    return (int64_t)(size * 1024 * 1024);
                return (int64_t)size;
            }
            fclose(f);
#elif defined(__APPLE__)
            uint64_t l1_cache_size;
            size_t size = sizeof(l1_cache_size);
            if (sysctlbyname("hw.l1dcachesize", &l1_cache_size, &size, NULL, 0) == 0)
                return (int64_t)l1_cache_size;
#endif
            return -1;
        }
        case CONF_CPU_L2_CACHE_SIZE:
        {
#ifdef __linux__
            FILE *f = fopen("/sys/devices/system/cpu/cpu0/cache/index2/size", "r");
            if (!f) 
                return -1;
            char size_str[32];
            if (fgets(size_str, sizeof(size_str), f)) 
            {
                int size = atoi(size_str);
                char unit = size_str[strlen(size_str) - 2];
                fclose(f);
                if (unit == 'K') 
                    return (int64_t)(size * 1024);
                if (unit == 'M') 
                    return (int64_t)(size * 1024 * 1024);
                return (int64_t)size;
            }
            fclose(f);
#elif defined(__APPLE__)
            uint64_t l2_cache_size;
            size_t size = sizeof(l2_cache_size);
            if (sysctlbyname("hw.l2cachesize", &l2_cache_size, &size, NULL, 0) == 0)
                return (int64_t)l2_cache_size;
#endif
            return -1;
        }
        case CONF_CPU_L3_CACHE_SIZE:
        {
#ifdef __linux__
            FILE *f = fopen("/sys/devices/system/cpu/cpu0/cache/index3/size", "r");
            if (!f)
                return -1;
            char size_str[32];
            if (fgets(size_str, sizeof(size_str), f)) 
            {
                int size = atoi(size_str);
                char unit = size_str[strlen(size_str) - 2];
                fclose(f);
                if (unit == 'K') 
                    return (int64_t)(size * 1024);
                if (unit == 'M') 
                    return (int64_t)(size * 1024 * 1024);
                return (int64_t)size;
            }
            fclose(f);
#elif defined(__APPLE__)
            uint64_t l3_cache_size;
            size_t size = sizeof(l3_cache_size);
            if (sysctlbyname("hw.l3cachesize", &l3_cache_size, &size, NULL, 0) == 0)
                return (int64_t)l3_cache_size;
#endif
            return -1;
        }
        case CONF_FILE_SIZE_MAX:
        {
            struct rlimit rlim;
            if (getrlimit(RLIMIT_FSIZE, &rlim) == 0)
                return (int64_t)rlim.rlim_max;
            return -1;
        }
        case CONF_NAME_MAX: 
        {
#if defined(_SC_NAME_MAX)
            return sysconf(_SC_NAME_MAX);
#elif defined(_PC_NAME_MAX)
            long v = pathconf("/", _PC_NAME_MAX);
            return v > 0 ? v : 255;
#else
            return 255;
#endif
        }
        case CONF_GETGR_R_SIZE_MAX: 
        {
            return sysconf(_SC_GETGR_R_SIZE_MAX);
        }
        case CONF_GETPW_R_SIZE_MAX: 
        {
            return sysconf(_SC_GETPW_R_SIZE_MAX);
        }
        case CONF_GROUP_ID:
        {
            return (int64_t)getgid();
        }
        case CONF_HEAP_SIZE_MAX:
        {
            struct rlimit rlim;
            if (getrlimit(RLIMIT_AS, &rlim) == 0)
                return (int64_t)rlim.rlim_max;
            return -1;
        }
        case CONF_HOST_NAME_MAX: 
        {
            return sysconf(_SC_HOST_NAME_MAX);
        }
        case CONF_LOGIN_NAME_MAX: 
        {
            return sysconf(_SC_LOGIN_NAME_MAX);
        }
        case CONF_MEMORY_TOTAL:
        {
#ifdef __linux__
            struct sysinfo info;
            if (sysinfo(&info) == 0)
                return (int64_t)(info.totalram * info.mem_unit);
#elif defined(__APPLE__)
            uint64_t physical_memory;
            size_t length = sizeof(uint64_t);
            int mib[2] = {CTL_HW, HW_MEMSIZE};
            if (sysctl(mib, 2, &physical_memory, &length, NULL, 0) == 0)
                return (int64_t)physical_memory;
#endif
            return -1;
        }
        case CONF_MEMORY_PAGE_SIZE: 
        {
            return sysconf(_SC_PAGESIZE);
        }
        case CONF_MSG_QUEUE_SIZE_MAX:
        {
#ifdef __linux__
            FILE *f = fopen("/proc/sys/fs/mqueue/msg_max", "r");
            if (!f) 
                return -1;
            int max_msgs;
            if (fscanf(f, "%d", &max_msgs) == 1)
            {
                fclose(f);
                return max_msgs;
            }
            fclose(f);
#endif
            return -1;
        }
        case CONF_MSG_MAX:
        {
#ifdef __linux__
            FILE *f = fopen("/proc/sys/fs/mqueue/msgsize_max", "r");
            if (!f)
                return -1;

            int max_size;
            if (fscanf(f, "%d", &max_size) == 1) 
            {
                fclose(f);
                return max_size;
            }
            fclose(f);
#endif
            return -1;
        }
        case CONF_NETWORK_INTERFACES:
        {
            struct ifaddrs *ifap, *ifa;
            int count = 0;
            if (getifaddrs(&ifap) != 0)
                return -1;
            for (ifa = ifap; ifa != NULL; ifa = ifa->ifa_next) 
            {
                if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) 
                    count++;
            }
            freeifaddrs(ifap);
            return count;
        }
        case CONF_NGROUPS_MAX: 
        {
            return sysconf(_SC_NGROUPS_MAX);
        }
        case CONF_OPEN_MAX: 
        {
            return sysconf(_SC_OPEN_MAX);
        }
        case CONF_PATH_MAX: 
        {
#if defined(_SC_PATH_MAX)
            return sysconf(_SC_PATH_MAX);
#elif defined(_PC_PATH_MAX)
            long v = pathconf("/", _PC_PATH_MAX);
            return v > 0 ? v : 4096;
#else
            return 4096;
#endif
        }
        case CONF_PIPE_BUF: 
        {
#if defined(_SC_PIPE_BUF)
            return sysconf(_SC_PIPE_BUF);
#elif defined(_PC_PIPE_BUF)
            long v = pathconf("/", _PC_PIPE_BUF);
            return v > 0 ? v : 4096;
#else
            return 4096;
#endif
        }
        case CONF_PROCESS_CHILD_MAX: 
        {
            return sysconf(_SC_CHILD_MAX);
        }
        case CONF_SEM_NSEMS_MAX:
        {
#ifdef __linux__
            FILE *f = fopen("/proc/sys/kernel/sem", "r");
            if (!f) 
                return -1;
            int semmsl, semmns, semopm, semmni;
            if (fscanf(f, "%d %d %d %d", &semmsl, &semmns, &semopm, &semmni) == 4) 
            {
                fclose(f);
                return semmsl;
            }
            fclose(f);
#endif
            return -1;
        }
        case CONF_SEM_VALUE_MAX:
        {
#ifdef SEM_VALUE_MAX
            return SEM_VALUE_MAX;
#else
            return -1;
#endif
        }
        case CONF_STACK_SIZE_MAX:
        {
            struct rlimit rlim;
            if (getrlimit(RLIMIT_STACK, &rlim) == 0)
                return (int64_t)rlim.rlim_max;
            return -1;
        }
        case CONF_SWAP_SIZE_TOTAL:
        {
#ifdef __linux__
            struct sysinfo info;
            if (sysinfo(&info) == 0)
                return (int64_t)(info.totalswap * info.mem_unit);
#elif defined(__APPLE__)
            struct statfs fs;
            if (statfs("/", &fs) == 0)
                return (int64_t)(fs.f_bavail * fs.f_bsize);
#endif
            return -1;
        }
        case CONF_SYMLOOP_MAX: 
        {
            return sysconf(_SC_SYMLOOP_MAX);
        }
        case CONF_THREADS_MAX:
        {
#ifdef _SC_THREAD_THREADS_MAX
            return sysconf(_SC_THREAD_THREADS_MAX);
#else
            return -1;
#endif
        }
        case CONF_TIMEZONE_OFFSET:
        {
            time_t now = time(NULL);
            struct tm *local_tm = localtime(&now);
            struct tm *utc_tm = gmtime(&now);
            return (int64_t)((mktime(local_tm) - mktime(utc_tm)) / 60); // minute
        }
        case CONF_TTY_NAME_MAX: 
        {
            return sysconf(_SC_TTY_NAME_MAX);
        }
        case CONF_UPTIME_SECONDS:
        {
#if defined(__linux__)
            struct sysinfo info;
            if (sysinfo(&info) == 0) 
                return info.uptime;
            return -1;
#elif defined(__APPLE__)
            struct timeval boottime;
            size_t size = sizeof(boottime);
            int mib[2] = {CTL_KERN, KERN_BOOTTIME};
            if (sysctl(mib, 2, &boottime, &size, NULL, 0) == 0) 
            {
                struct timeval now;
                gettimeofday(&now, NULL);
                return (int64_t)(now.tv_sec - boottime.tv_sec);
            }
            return -1;
#else
            return -1;
#endif
        }
        case CONF_USER_ID:
        {
            return (int64_t)getuid();
        }
        case CONF_VIRTUAL_MEMORY_MAX:
        {
            struct rlimit rlim;
            if (getrlimit(RLIMIT_AS, &rlim) == 0)
                return (int64_t)rlim.rlim_max;
            return -1;
        }
        default:
            return -1;
    }
#else
    (void)conf;
    return -1;
#endif
}

#define ENV_AIO_MAX             env_get(CONF_AIO_MAX)

#define ENV_ARG_LEN_MAX         env_get(CONF_ARG_MAX)

#define ENV_BOOT_TIME           env_get(CONF_BOOT_TIME)

#define ENV_CLOCK_TICKS_PER_SEC env_get(CONF_CLK_TCK)

#define ENV_CPU_COUNT           env_get(CONF_CPU_COUNT)
#define ENV_CPU_CACHE_LINE_SIZE env_get(CONF_CPU_CACHE_LINE_SIZE)
#define ENV_CPU_FREQUENCY       env_get(CONF_CPU_FREQUENCY)
#define ENV_CPU_L1_CACHE_SIZE   env_get(CONF_CPU_L1_CACHE_SIZE)
#define ENV_CPU_L2_CACHE_SIZE   env_get(CONF_CPU_L2_CACHE_SIZE)
#define ENV_CPU_L3_CACHE_SIZE   env_get(CONF_CPU_L3_CACHE_SIZE)

#define ENV_FILE_SIZE_MAX       env_get(CONF_FILE_SIZE_MAX)
#define ENV_FILENAME_LEN_MAX    env_get(CONF_NAME_MAX)

#define ENV_GETGR_R_SIZE_MAX    env_get(CONF_GETGR_R_SIZE_MAX)
#define ENV_GETPW_R_SIZE_MAX    env_get(CONF_GETPW_R_SIZE_MAX)

#define ENV_GROUP_ID            env_get(CONF_GROUP_ID)

#define ENV_HEAP_SIZE_MAX       env_get(CONF_HEAP_SIZE_MAX)

#define ENV_HOSTNAME_LEN_MAX    env_get(CONF_HOST_NAME_MAX)

#define ENV_LOGIN_NAME_MAX      env_get(CONF_LOGIN_NAME_MAX)

#define ENV_MEMORY_TOTAL        env_get(CONF_MEMORY_TOTAL)
#define ENV_MEMORY_PAGE_SIZE    env_get(CONF_MEMORY_PAGE_SIZE)

#define ENV_MSG_QUEUE_SIZE_MAX  env_get(CONF_MSG_QUEUE_SIZE_MAX)
#define ENV_MSG_MAX             env_get(CONF_MSG_MAX)

#define ENV_NETWORK_INTERFACES  env_get(CONF_NETWORK_INTERFACES)

#define ENV_NGROUPS_MAX         env_get(CONF_NGROUPS_MAX)

#define ENV_OPEN_FILES_MAX      env_get(CONF_OPEN_MAX)

#define ENV_PATH_LEN_MAX        env_get(CONF_PATH_MAX)

#define ENV_PIPE_BUF            env_get(CONF_PIPE_BUF)

#define ENV_PROCESS_CHILD_MAX   env_get(CONF_PROCESS_CHILD_MAX)

#define ENV_SEM_NSEMS_MAX       env_get(CONF_SEM_NSEMS_MAX)
#define ENV_SEM_VALUE_MAX       env_get(CONF_SEM_VALUE_MAX)

#define ENV_STACK_SIZE_MAX      env_get(CONF_STACK_SIZE_MAX)

#define ENV_SWAP_SIZE_TOTAL     env_get(CONF_SWAP_SIZE_TOTAL)

#define ENV_SYMLINK_MAX         env_get(CONF_SYMLOOP_MAX)

#define ENV_THREADS_MAX         env_get(CONF_THREADS_MAX)

#define ENV_TIMEZONE_OFFSET     env_get(CONF_TIMEZONE_OFFSET)

#define ENV_TTY_NAME_MAX        env_get(CONF_TTY_NAME_MAX)

#define ENV_UPTIME_SECONDS      env_get(CONF_UPTIME_SECONDS)

#define ENV_USER_ID             env_get(CONF_USER_ID)

#define ENV_VIRTUAL_MEMORY_MAX  env_get(CONF_VIRTUAL_MEMORY_MAX)

#ifdef __cplusplus
}
#endif

#endif // ENV_H