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

#ifndef CPU_H
#define CPU_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HJ_CPU_API
#if defined(HJ_CPU_STATIC)
#define HJ_CPU_API static inline
#else
#define HJ_CPU_API extern
#endif
#endif

typedef enum
{
    HJ_CPU_OK                           = 0,
    HJ_CPU_ERR_UNKNOWN                  = -1,
    HJ_CPU_ERR_NOT_SUPPORTED            = -2,
    HJ_CPU_ERR_INVALID_ARG              = -3,
    HJ_CPU_ERR_INTERNAL                 = -4,
    HJ_CPU_ERR_ALLOCATION_MEMORY_FAILED = -5,
    HJ_CPU_ERR_OPEN_FILE_FAILED         = -6,
    HJ_CPU_ERR_READ_INFO_FAILED         = -7,
    HJ_CPU_ERR_SYSCTL_FAILED            = -8,
    HJ_CPU_ERR_SYSCONF_FAILED           = -9,
    HJ_CPU_ERR_INVALID_CPU              = -10,
    HJ_CPU_ERR_PERMISSION               = -11,
    HJ_CPU_ERR_THREAD_NOT_FOUND         = -12
} hj_cpu_err_t;

typedef enum
{
    HJ_CPU_FEATURE_TSC,
    HJ_CPU_FEATURE_RDTSCP,
    HJ_CPU_FEATURE_CLFLUSH,
    HJ_CPU_FEATURE_PREFETCH,
    HJ_CPU_FEATURE_CPU_AFFINITY,
    HJ_CPU_FEATURE_PMU,
    HJ_CPU_FEATURE_PAUSE
} hj_cpu_feature_t;

// ------------------------ CPU API Declarations ------------------------

HJ_CPU_API bool         hj_cpu_has_feature(hj_cpu_feature_t feature);
HJ_CPU_API hj_cpu_err_t hj_cpu_brand(char *buf, size_t size);
HJ_CPU_API hj_cpu_err_t hj_cpu_vendor(char *buf, size_t size);
HJ_CPU_API hj_cpu_err_t hj_cpu_logical_core_num(unsigned int *num);
HJ_CPU_API hj_cpu_err_t hj_cpu_core_bind(const unsigned int core);
HJ_CPU_API hj_cpu_err_t hj_cpu_logical_core_list(unsigned int *buf,
                                                 unsigned int *len);
HJ_CPU_API hj_cpu_err_t hj_cpu_id(uint32_t *cpu_id);
HJ_CPU_API void         hj_cpu_pause(void);
HJ_CPU_API void         hj_cpu_nop(void);
HJ_CPU_API void         hj_cpu_delay_ticks(uint64_t ticks);
HJ_CPU_API void         hj_cpu_cache_flush(const void *addr);
HJ_CPU_API void         hj_cpu_prefetch_read(const void *addr);
HJ_CPU_API void         hj_cpu_prefetch_write(const void *addr);
HJ_CPU_API uint64_t     hj_cpu_tsc_start(void);
HJ_CPU_API uint64_t     hj_cpu_tsc_end(uint32_t *aux);
HJ_CPU_API uint64_t     hj_cpu_tsc_read(void);
HJ_CPU_API uint64_t     hj_cpu_tsc_frequency();
HJ_CPU_API uint64_t     hj_cpu_tscp_read(uint32_t *aux);

#ifdef __cplusplus
}
#endif

#endif // CPU_H


// --------------------- Implementation -------------------------
// To include implementation, define HJ_CPU_IMPL before including
// this header in ONE C/C++ source file.
#if (defined(HJ_CPU_IMPL) || defined(HJ_CPU_STATIC))                           \
    && !defined(HJ_CPU_IMPL_DONE)
#define HJ_CPU_IMPL_DONE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <intrin.h>
#include <immintrin.h>

#elif defined(__APPLE__)
#include <sys/param.h>
#include <sys/sysctl.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <unistd.h>
#include <libkern/OSCacheControl.h>
#include <mach/mach_time.h>

#elif defined(__linux__)
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#ifdef __GLIBC__
#include <sched.h>
#endif

#else
#pragma message("Unknown OS, some functions will be disabled")
#endif

#ifdef __cplusplus
extern "C" {
#endif

HJ_CPU_API bool hj_cpu_has_feature(hj_cpu_feature_t feature)
{
#if defined(_M_IX86) || defined(_M_X64) || defined(__i386__)                   \
    || defined(__x86_64__)
    int cpuInfo[4] = {0};
    switch(feature)
    {
        case HJ_CPU_FEATURE_TSC:
            __cpuid(cpuInfo, 1);
            return (cpuInfo[3] & (1 << 4)) != 0; // EDX bit 4
        case HJ_CPU_FEATURE_RDTSCP:
            __cpuid(cpuInfo, 0x80000000);
            if((unsigned int) cpuInfo[0] >= 0x80000001)
            {
                __cpuid(cpuInfo, 0x80000001);
                return (cpuInfo[3] & (1 << 27)) != 0; // EDX bit 27
            }
            return false;
        case HJ_CPU_FEATURE_CLFLUSH:
            __cpuid(cpuInfo, 1);
            return (cpuInfo[3] & (1 << 19)) != 0; // EDX bit 19
        case HJ_CPU_FEATURE_PREFETCH:
            return true;
        case HJ_CPU_FEATURE_CPU_AFFINITY:
#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
            return true;
#else
            return false;
#endif
        case HJ_CPU_FEATURE_PMU:
            return true;
        case HJ_CPU_FEATURE_PAUSE:
            return true;
        default:
            return false;
    }
#elif defined(__aarch64__) || defined(__arm__) || defined(_M_ARM)              \
    || defined(_M_ARM64)
    switch(feature)
    {
        case HJ_CPU_FEATURE_TSC:
            return true; // ARM generic timer (cntvct_el0)
        case HJ_CPU_FEATURE_RDTSCP:
            return false; // x86 specific
        case HJ_CPU_FEATURE_CLFLUSH:
            return true; // Supported via cache maintenance instructions
        case HJ_CPU_FEATURE_PREFETCH:
            return true; // Supported via compiler builtins
        case HJ_CPU_FEATURE_CPU_AFFINITY:
#if defined(__linux__) || defined(_WIN32) || defined(_WIN64)
            return true;
#else
            return false;
#endif
        case HJ_CPU_FEATURE_PMU:
            return true;
        case HJ_CPU_FEATURE_PAUSE:
            return true; // yield instruction
        default:
            return false;
    }
#else
    switch(feature)
    {
        case HJ_CPU_FEATURE_TSC:
        case HJ_CPU_FEATURE_PREFETCH:
        case HJ_CPU_FEATURE_PMU:
        case HJ_CPU_FEATURE_PAUSE:
            return true;
        case HJ_CPU_FEATURE_RDTSCP:
            return false;
        case HJ_CPU_FEATURE_CLFLUSH:
            return true;
        case HJ_CPU_FEATURE_CPU_AFFINITY:
#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
            return true;
#else
            return false;
#endif
        default:
            return false;
    }
#endif
}

HJ_CPU_API hj_cpu_err_t hj_cpu_brand(char *buf, size_t size)
{
    if(!buf || size == 0)
        return HJ_CPU_ERR_INVALID_ARG;

#if defined(_WIN32) || defined(_WIN64)
    int  cpuInfo[4] = {0};
    char brand[65]  = {0};
    __cpuid(cpuInfo, 0x80000000);
    unsigned int nExIds = (unsigned int) cpuInfo[0];
    if(nExIds >= 0x80000004)
    {
        __cpuid((int *) cpuInfo, 0x80000002);
        memcpy(brand, cpuInfo, 16);
        __cpuid((int *) cpuInfo, 0x80000003);
        memcpy(brand + 16, cpuInfo, 16);
        __cpuid((int *) cpuInfo, 0x80000004);
        memcpy(brand + 32, cpuInfo, 16);

        size_t len = strlen(brand);
        if(len >= size)
            len = size - 1;

        memcpy(buf, brand, len);
        buf[len] = '\0';
        return HJ_CPU_OK;
    } else
    {
        buf[0] = '\0';
        return HJ_CPU_ERR_NOT_SUPPORTED;
    }

#elif defined(__linux__)
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if(!fp)
    {
        buf[0] = '\0';
        return HJ_CPU_ERR_OPEN_FILE_FAILED;
    }

    char *p = NULL;
    char  line[256];
    while(fgets(line, sizeof(line), fp))
    {
        if(!(strncmp(line, "model name", 10) == 0
             || strncmp(line, "Hardware", 8) == 0
             || strncmp(line, "Processor", 9) == 0))
            continue;

        p = strchr(line, ':');
        if(!p)
            continue;

        p++;
        while(*p == ' ' || *p == '\t')
            ++p;

        size_t len = strlen(p);
        while(len > 0 && (p[len - 1] == '\n' || p[len - 1] == '\r'))
            p[--len] = '\0';

        if(len >= size)
            len = size - 1;

        memcpy(buf, p, len);
        buf[len] = '\0';
        fclose(fp);
        return HJ_CPU_OK;
    }

    fclose(fp);
    buf[0] = '\0';
    return HJ_CPU_ERR_READ_INFO_FAILED;

#elif defined(__APPLE__)
    char   temp_buf[256] = {0};
    size_t temp_len      = sizeof(temp_buf);
    if(sysctlbyname("machdep.cpu.brand_string", temp_buf, &temp_len, NULL, 0)
       == 0)
    {
        size_t len = strlen(temp_buf);
        if(len >= size)
            len = size - 1;

        memcpy(buf, temp_buf, len);
        buf[len] = '\0';
        return HJ_CPU_OK;
    }

    buf[0] = '\0';
    return HJ_CPU_ERR_SYSCTL_FAILED;

#else
    buf[0] = '\0';
    return HJ_CPU_ERR_NOT_SUPPORTED;

#endif
}

HJ_CPU_API hj_cpu_err_t hj_cpu_vendor(char *buf, size_t size)
{
    if(!buf || size == 0)
        return HJ_CPU_ERR_INVALID_ARG;

#if defined(_WIN32) || defined(_WIN64)
    int  cpuInfo[4] = {0};
    char vendor[13] = {0};
    __cpuid(cpuInfo, 0);
    memcpy(vendor, &cpuInfo[1], 4);
    memcpy(vendor + 4, &cpuInfo[3], 4);
    memcpy(vendor + 8, &cpuInfo[2], 4);
    vendor[12] = '\0';

    size_t len = strlen(vendor);
    if(len >= size)
        len = size - 1;

    memcpy(buf, vendor, len);
    buf[len] = '\0';
    return HJ_CPU_OK;

#elif defined(__linux__)
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if(!fp)
    {
        buf[0] = '\0';
        return HJ_CPU_ERR_OPEN_FILE_FAILED;
    }

    char *p = NULL;
    char  line[256];
    while(fgets(line, sizeof(line), fp))
    {
        if(!(strncmp(line, "vendor_id", 9) == 0
             || strncmp(line, "CPU implementer", 15) == 0))
            continue;

        p = strchr(line, ':');
        if(!p)
            continue;

        p++;
        while(*p == ' ' || *p == '\t')
            ++p;

        size_t len = strlen(p);
        while(len > 0 && (p[len - 1] == '\n' || p[len - 1] == '\r'))
            p[--len] = '\0';

        if(len >= size)
            len = size - 1;

        memcpy(buf, p, len);
        buf[len] = '\0';
        fclose(fp);
        return HJ_CPU_OK;
    }

    fclose(fp);
    buf[0] = '\0';
    return HJ_CPU_ERR_READ_INFO_FAILED;

#elif defined(__APPLE__)
    char   temp_buf[64] = {0};
    size_t temp_len     = sizeof(temp_buf);

    if(sysctlbyname("machdep.cpu.vendor", temp_buf, &temp_len, NULL, 0) == 0)
    {
        size_t len = strlen(temp_buf);
        if(len >= size)
            len = size - 1;

        memcpy(buf, temp_buf, len);
        buf[len] = '\0';
        return HJ_CPU_OK;
    }

    buf[0] = '\0';
    return HJ_CPU_ERR_SYSCTL_FAILED;

#else
    buf[0] = '\0';
    return HJ_CPU_ERR_NOT_SUPPORTED;

#endif
}

HJ_CPU_API hj_cpu_err_t hj_cpu_logical_core_num(unsigned int *num)
{
    if(!num)
        return HJ_CPU_ERR_INVALID_ARG;

#if defined(_WIN32) || defined(_WIN64)
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    if(sysinfo.dwNumberOfProcessors > 0)
    {
        *num = sysinfo.dwNumberOfProcessors;
        return HJ_CPU_OK;
    }
    return HJ_CPU_ERR_INTERNAL;

#elif defined(__APPLE__)
    int          nm[2] = {CTL_HW, HW_AVAILCPU};
    size_t       len   = sizeof(unsigned int);
    unsigned int count = 0;
    if(sysctl(nm, 2, &count, &len, NULL, 0) == 0 && count >= 1)
    {
        *num = count;
        return HJ_CPU_OK;
    }

    nm[1] = HW_NCPU;
    if(sysctl(nm, 2, &count, &len, NULL, 0) == 0 && count >= 1)
    {
        *num = count;
        return HJ_CPU_OK;
    }

    return HJ_CPU_ERR_SYSCTL_FAILED;

#elif defined(__linux__)
    long result = sysconf(_SC_NPROCESSORS_ONLN);
    if(result > 0)
    {
        *num = (unsigned int) result;
        return HJ_CPU_OK;
    }
    return HJ_CPU_ERR_SYSCONF_FAILED;

#else
    return HJ_CPU_ERR_NOT_SUPPORTED;

#endif
}

HJ_CPU_API hj_cpu_err_t hj_cpu_core_bind(const unsigned int core)
{
#if defined(_WIN32) || defined(_WIN64)
    WORD         group_count   = GetActiveProcessorGroupCount();
    WORD         target_group  = 0;
    unsigned int current_base  = 0;
    unsigned int relative_core = 0;
    BOOL         found         = FALSE;
    for(WORD g = 0; g < group_count; ++g)
    {
        DWORD count_in_group = GetActiveProcessorCount(g);
        if(core < current_base + count_in_group)
        {
            target_group  = g;
            relative_core = core - current_base;
            found         = TRUE;
            break;
        }
        current_base += count_in_group;
    }

    if(!found || relative_core >= (sizeof(KAFFINITY) * 8))
        return HJ_CPU_ERR_INVALID_ARG;

    GROUP_AFFINITY group_affinity;
    memset(&group_affinity, 0, sizeof(group_affinity));
    group_affinity.Group = target_group;
    group_affinity.Mask  = ((KAFFINITY) 1) << relative_core;

    HANDLE hThread = GetCurrentThread();
    if(SetThreadGroupAffinity(hThread, &group_affinity, NULL))
        return HJ_CPU_OK;

    return HJ_CPU_ERR_INTERNAL;

#elif defined(__linux__)
    if(core >= CPU_SETSIZE)
        return HJ_CPU_ERR_INVALID_CPU;

    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(core, &mask);
    int ret = pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask);
    if(ret == 0)
    {
        return HJ_CPU_OK;
    }

    if(ret == EINVAL)
        return HJ_CPU_ERR_INVALID_CPU;
    else if(ret == EPERM)
        return HJ_CPU_ERR_PERMISSION;
    else if(ret == ESRCH)
        return HJ_CPU_ERR_THREAD_NOT_FOUND;
    else
        return HJ_CPU_ERR_INTERNAL;

#elif defined(__APPLE__)
    (void) core;
    return HJ_CPU_ERR_NOT_SUPPORTED;

#else
    (void) core;
    return HJ_CPU_ERR_NOT_SUPPORTED;

#endif
}

HJ_CPU_API hj_cpu_err_t hj_cpu_logical_core_list(unsigned int *buf,
                                                 unsigned int *len)
{
    if(!len)
        return HJ_CPU_ERR_INVALID_ARG;

    unsigned int required_len = 0;

#if defined(_WIN32) || defined(_WIN64)
    DWORD returnLength = 0;
    GetLogicalProcessorInformationEx(RelationProcessorCore,
                                     NULL,
                                     &returnLength);
    if(returnLength == 0)
        return HJ_CPU_ERR_INTERNAL;

    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX buffer =
        (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX) malloc(returnLength);
    if(!buffer)
        return HJ_CPU_ERR_ALLOCATION_MEMORY_FAILED;

    if(!GetLogicalProcessorInformationEx(RelationProcessorCore,
                                         buffer,
                                         &returnLength))
    {
        free(buffer);
        return HJ_CPU_ERR_INTERNAL;
    }

    WORD          group_count = GetActiveProcessorGroupCount();
    unsigned int *group_bases =
        (unsigned int *) malloc(group_count * sizeof(unsigned int));
    if(!group_bases)
    {
        free(buffer);
        return HJ_CPU_ERR_ALLOCATION_MEMORY_FAILED;
    }

    unsigned int running_sum = 0;
    WORD         sg;
    for(sg = 0; sg < group_count; ++sg)
    {
        group_bases[sg] = running_sum;
        running_sum += GetActiveProcessorCount(sg);
    }

    BYTE *ptr = (BYTE *) buffer;
    BYTE *end = ptr + returnLength;

    while(ptr < end)
    {
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX info =
            (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX) ptr;
        if(info->Relationship == RelationProcessorCore)
        {
            WORD g;
            for(g = 0; g < info->Processor.GroupCount; ++g)
            {
                KAFFINITY mask = info->Processor.GroupMask[g].Mask;
                int       j;
                for(j = 0; j < (int) (sizeof(KAFFINITY) * 8); ++j)
                {
                    if(mask & ((KAFFINITY) 1 << j))
                    {
                        required_len++;
                    }
                }
            }
        }
        ptr += info->Size;
    }

    if(!buf)
    {
        *len = required_len;
        free(group_bases);
        free(buffer);
        return HJ_CPU_OK;
    }

    unsigned int max_capacity = *len;
    unsigned int count        = 0;

    ptr = (BYTE *) buffer;
    while(ptr < end && count < max_capacity)
    {
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX info =
            (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX) ptr;
        if(info->Relationship == RelationProcessorCore)
        {
            WORD g;
            for(g = 0; g < info->Processor.GroupCount && count < max_capacity;
                ++g)
            {
                WORD         group_num = info->Processor.GroupMask[g].Group;
                KAFFINITY    mask      = info->Processor.GroupMask[g].Mask;
                unsigned int base      = (group_num < group_count)
                                             ? group_bases[group_num]
                                             : running_sum;

                int j;
                for(j = 0;
                    j < (int) (sizeof(KAFFINITY) * 8) && count < max_capacity;
                    ++j)
                {
                    if(mask & ((KAFFINITY) 1 << j))
                    {
                        if(count < max_capacity)
                        {
                            buf[count++] = base + j;
                        }
                    }
                }
            }
        }
        ptr += info->Size;
    }

    *len = count;
    free(group_bases);
    free(buffer);
    return HJ_CPU_OK;

#elif defined(__linux__)
    long ncpu = sysconf(_SC_NPROCESSORS_ONLN);
    if(ncpu <= 0)
        return HJ_CPU_ERR_SYSCONF_FAILED;

    required_len = (unsigned int) ncpu;
    if(!buf)
    {
        *len = required_len;
        return HJ_CPU_OK;
    }

    unsigned int max_capacity = *len;
    unsigned int count        = 0;
    unsigned int max_cpus     = required_len;
    unsigned int i;
    for(i = 0; i < max_cpus && count < max_capacity; ++i)
        buf[count++] = i;

    *len = count;
    return HJ_CPU_OK;

#elif defined(__APPLE__)
    int          nm[2];
    size_t       len_cpu = sizeof(unsigned int);
    unsigned int ncpu    = 0;
    nm[0]                = CTL_HW;
    nm[1]                = HW_AVAILCPU;
    if(sysctl(nm, 2, &ncpu, &len_cpu, NULL, 0) != 0 || ncpu == 0)
    {
        nm[1] = HW_NCPU;
        if(sysctl(nm, 2, &ncpu, &len_cpu, NULL, 0) != 0 || ncpu == 0)
            return HJ_CPU_ERR_SYSCTL_FAILED;
    }

    required_len = ncpu;
    if(!buf)
    {
        *len = required_len;
        return HJ_CPU_OK;
    }

    unsigned int max_capacity = *len;
    unsigned int count        = 0;
    unsigned int i;
    for(i = 0; i < ncpu && count < max_capacity; ++i)
        buf[count++] = i;

    *len = count;
    return HJ_CPU_OK;

#else
    *len = 0;
    return HJ_CPU_ERR_NOT_SUPPORTED;

#endif
}

HJ_CPU_API hj_cpu_err_t hj_cpu_id(uint32_t *cpu_id)
{
    if(!cpu_id)
        return HJ_CPU_ERR_INVALID_ARG;

#if defined(_WIN32) || defined(_WIN64)
    *cpu_id = (uint32_t) GetCurrentProcessorNumber();
    return HJ_CPU_OK;

#elif defined(__linux__)
#ifdef __GLIBC__
    int cpu = sched_getcpu();
    if(cpu >= 0)
    {
        *cpu_id = (uint32_t) cpu;
        return HJ_CPU_OK;
    }
    return HJ_CPU_ERR_INTERNAL;
#else
    FILE *file = fopen("/proc/self/stat", "r");
    if(!file)
        return HJ_CPU_ERR_OPEN_FILE_FAILED;

    char buffer[1024];
    if(!fgets(buffer, sizeof(buffer), file))
    {
        fclose(file);
        return HJ_CPU_ERR_READ_INFO_FAILED;
    }
    fclose(file);

    char *ptr = strrchr(buffer, ')');
    if(!ptr)
        return HJ_CPU_ERR_READ_INFO_FAILED;

    int space_count = 0;
    while(*ptr && space_count < 37)
    {
        if(*ptr == ' ')
            space_count++;
        ptr++;
    }

    unsigned int cpu = 0;
    if(space_count == 37 && sscanf(ptr, "%u", &cpu) == 1)
    {
        *cpu_id = (uint32_t) cpu;
        return HJ_CPU_OK;
    }

    return HJ_CPU_ERR_READ_INFO_FAILED;
#endif

#elif defined(__APPLE__)
    return HJ_CPU_ERR_NOT_SUPPORTED;

#else
    return HJ_CPU_ERR_NOT_SUPPORTED;

#endif
}

HJ_CPU_API void hj_cpu_pause(void)
{
#if defined(_WIN32) || defined(_WIN64)
#if defined(_M_IX86) || defined(_M_X64)
    _mm_pause();
#elif defined(_M_ARM) || defined(_M_ARM64)
    __yield();
#else
#if defined(_MSC_VER)
    _ReadWriteBarrier();
#else
    (void) 0;
#endif
#endif

#elif defined(__GNUC__) || defined(__clang__)
#if defined(__i386__) || defined(__x86_64__)
    __asm__ volatile("pause" ::: "memory");
#elif defined(__arm__) || defined(__aarch64__)
    __asm__ volatile("yield" ::: "memory");
#else
    __asm__ volatile("" ::: "memory");
#endif

#else
    (void) 0;

#endif
}

HJ_CPU_API void hj_cpu_nop(void)
{
#if defined(_WIN32) || defined(_WIN64)
#if defined(_M_IX86) || defined(_M_X64)
    __nop();
#else
    volatile int dummy = 0;
    (void) dummy;
#endif

#elif defined(__GNUC__) || defined(__clang__)
#if defined(__i386__) || defined(__x86_64__)
    __asm__ volatile("nop" ::: "memory");
#elif defined(__arm__) || defined(__aarch64__)
    __asm__ volatile("nop" ::: "memory");
#else
    __asm__ volatile("" ::: "memory");
#endif

#else
    volatile int dummy = 0;
    (void) dummy;

#endif
}

HJ_CPU_API void hj_cpu_delay_ticks(uint64_t ticks)
{
    if(ticks == 0)
        return;

#if defined(_M_IX86) || defined(_M_X64) || defined(__i386__)                   \
    || defined(__x86_64__)
    uint64_t start = hj_cpu_tsc_read();
    while((hj_cpu_tsc_read() - start) < ticks)
    {
        hj_cpu_pause();
    }

#elif defined(__aarch64__) || defined(_M_ARM64)
#if defined(__GNUC__) || defined(__clang__)
    uint64_t start, current;
    __asm__ volatile("mrs %0, cntvct_el0" : "=r"(start));
    do
    {
        hj_cpu_pause();
        __asm__ volatile("mrs %0, cntvct_el0" : "=r"(current));
    } while((current - start) < ticks);
#elif defined(_MSC_VER)
    uint64_t start = _ReadStatusReg(ARM64_CNTVCT);
    while((_ReadStatusReg(ARM64_CNTVCT) - start) < ticks)
    {
        hj_cpu_pause();
    }
#else
    volatile uint64_t i;
    for(i = 0; i < ticks; ++i)
        hj_cpu_pause();
#endif

#else
    volatile uint64_t i;
    for(i = 0; i < ticks; ++i)
        hj_cpu_pause();

#endif
}

HJ_CPU_API void hj_cpu_cache_flush(const void *addr)
{
    if(!addr)
        return;

#if defined(_WIN32) || defined(_WIN64)
#if defined(_M_IX86) || defined(_M_X64)
    _mm_clflush(addr);
#elif defined(_M_ARM64)
    // Windows on ARM64: MSVC does not support inline asm.
    // Issue a Data Synchronization Barrier to ensure memory visibility.
    __dsb(_ARM64_BARRIER_SY);
    (void) addr;
#else
    MemoryBarrier();
    (void) addr;
#endif

#elif defined(__APPLE__)
#if defined(__aarch64__) || defined(__arm__)
    // Apple Silicon / iOS: Use Apple's official system data cache flush API
    // Flushes 64 bytes (standard cache line size on Apple Silicon)
    sys_dcache_flush((void *) addr, 64);
#elif defined(__i386__) || defined(__x86_64__)
    __builtin_ia32_clflush(addr);
#else
    __sync_synchronize();
    (void) addr;
#endif

#elif defined(__linux__)
#if defined(__i386__) || defined(__x86_64__)
    __builtin_ia32_clflush(addr);
#elif defined(__aarch64__)
    // ARM64 Linux: Clean and Invalidate Data Cache line by VA to PoC
    __asm__ volatile("dc civac, %0\n\t"
                     "dsb ish\n\t"
                     :
                     : "r"(addr)
                     : "memory");
#elif defined(__arm__)
    // 32-bit ARM Linux: DCCIMVAC (Clean and Invalidate Data Cache line by MVA to PoC)
    __asm__ volatile("mcr p15, 0, %0, c7, c14, 1\n\t"
                     "dsb\n\t"
                     :
                     : "r"(addr)
                     : "memory");
#else
    __sync_synchronize();
    (void) addr;
#endif

#else
#if defined(__GNUC__) || defined(__clang__)
    __sync_synchronize();
#endif
    (void) addr;
#endif
}

HJ_CPU_API void hj_cpu_prefetch_read(const void *addr)
{
    if(!addr)
        return;

#if defined(_WIN32) || defined(_WIN64)
#if defined(_M_IX86) || defined(_M_X64)
    _mm_prefetch((const char *) addr, _MM_HINT_T0);
#else
    (void) addr;
#endif

#elif defined(__GNUC__) || defined(__clang__)
    __builtin_prefetch(addr, 0, 3);

#else
    (void) addr;
#endif
}

HJ_CPU_API void hj_cpu_prefetch_write(const void *addr)
{
    if(!addr)
        return;

#if defined(_WIN32) || defined(_WIN64)
#if defined(_M_IX86) || defined(_M_X64)
    _m_prefetchw((void *) addr);
#else
    (void) addr;
#endif

#elif defined(__GNUC__) || defined(__clang__)
    __builtin_prefetch(addr, 1, 3);

#else
    (void) addr;
#endif
}

HJ_CPU_API uint64_t hj_cpu_tsc_start(void)
{
#if defined(_WIN32) || defined(_WIN64)
#if defined(_M_IX86) || defined(_M_X64)
    _mm_lfence();
    return __rdtsc();
#else
    return hj_cpu_tsc_read();
#endif

#elif defined(__GNUC__) || defined(__clang__)
#if defined(__i386__) || defined(__x86_64__)
    __asm__ volatile("lfence" ::: "memory");
    return __builtin_ia32_rdtsc();
#else
    return hj_cpu_tsc_read();
#endif

#else
    return hj_cpu_tsc_read();

#endif
}

HJ_CPU_API uint64_t hj_cpu_tsc_end(uint32_t *aux)
{
#if defined(_WIN32) || defined(_WIN64)
#if defined(_M_IX86) || defined(_M_X64)
    uint32_t     dummy;
    uint32_t    *paux = aux ? aux : &dummy;
    unsigned int tsc_aux;
    uint64_t     tsc = __rdtscp(&tsc_aux);
    _mm_lfence();
    *paux = tsc_aux;
    return tsc;
#else
    if(aux)
        hj_cpu_id(aux);
    return hj_cpu_tsc_read();
#endif

#elif defined(__GNUC__) || defined(__clang__)
#if defined(__i386__) || defined(__x86_64__)
    uint32_t lo, hi, cpu_id_val;
    __asm__ volatile("rdtscp\n\t"
                     "lfence"
                     : "=a"(lo), "=d"(hi), "=c"(cpu_id_val)
                     :
                     : "memory");
    if(aux)
        *aux = cpu_id_val;
    return ((uint64_t) hi << 32) | lo;
#else
    if(aux)
        hj_cpu_id(aux);
    return hj_cpu_tsc_read();
#endif

#else
    if(aux)
        hj_cpu_id(aux);
    return hj_cpu_tsc_read();

#endif
}

HJ_CPU_API uint64_t hj_cpu_tsc_read(void)
{
#if defined(_WIN32) || defined(_WIN64)
#if defined(_M_IX86) || defined(_M_X64)
    return __rdtsc();
#else
    LARGE_INTEGER counter;
    if(QueryPerformanceCounter(&counter))
        return (uint64_t) counter.QuadPart;

    return 0;
#endif

#elif defined(__linux__)
#if defined(__GNUC__) || defined(__clang__)
#if defined(__i386__) || defined(__x86_64__)
    return __builtin_ia32_rdtsc();
#else
    struct timespec ts;
    if(clock_gettime(CLOCK_MONOTONIC_RAW, &ts) == 0)
        return (uint64_t) ts.tv_sec * 1000000000ULL + (uint64_t) ts.tv_nsec;

    return 0;
#endif
#else
    struct timespec ts;
    if(clock_gettime(CLOCK_MONOTONIC_RAW, &ts) == 0)
        return (uint64_t) ts.tv_sec * 1000000000ULL + (uint64_t) ts.tv_nsec;

    return 0;
#endif

#elif defined(__APPLE__)
    return mach_absolute_time();

#else
    clock_t c = clock();
    return (uint64_t) c;

#endif
}

HJ_CPU_API uint64_t hj_cpu_tsc_frequency()
{
#if defined(__APPLE__)
    mach_timebase_info_data_t tb;
    if(mach_timebase_info(&tb) == KERN_SUCCESS && tb.num != 0)
    {
        return 1000000000ULL * tb.denom / tb.num;
    }
    return 0;
#elif defined(_WIN32) || defined(_WIN64)
    LARGE_INTEGER freq;
    if(QueryPerformanceFrequency(&freq))
    {
        return (uint64_t) freq.QuadPart;
    }
    return 0;
#elif defined(__linux__) || defined(__unix__)
#if defined(__i386__) || defined(__x86_64__)
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if(fp)
    {
        char line[256];
        while(fgets(line, sizeof(line), fp))
        {
            if(strncmp(line, "cpu MHz", 7) == 0)
            {
                char *p = strchr(line, ':');
                if(p)
                {
                    double mhz = atof(p + 1);
                    fclose(fp);
                    if(mhz > 0.0)
                    {
                        return (uint64_t) (mhz * 1000000.0);
                    }
                }
            }
        }
        fclose(fp);
    }
#elif defined(__aarch64__) || defined(__arm__)
    return 1000000000ULL;
#endif

    struct timespec ts_start, ts_end;
    uint64_t        tsc_start = hj_cpu_tsc_read();
    if(clock_gettime(CLOCK_MONOTONIC, &ts_start) != 0)
    {
        return 0;
    }

    struct timespec req = {0, 10000000};
    nanosleep(&req, NULL);

    uint64_t tsc_end = hj_cpu_tsc_read();
    if(clock_gettime(CLOCK_MONOTONIC, &ts_end) != 0)
    {
        return 0;
    }

    uint64_t ns_elapsed =
        (uint64_t) (ts_end.tv_sec - ts_start.tv_sec) * 1000000000ULL
        + (uint64_t) (ts_end.tv_nsec - ts_start.tv_nsec);
    if(ns_elapsed == 0)
    {
        return 0;
    }

    return (tsc_end - tsc_start) * 1000000000ULL / ns_elapsed;
#else
    return 0;
#endif
}

HJ_CPU_API uint64_t hj_cpu_tscp_read(uint32_t *aux)
{
#if defined(_WIN32) || defined(_WIN64)
#if defined(_M_IX86) || defined(_M_X64)
    if(!aux)
    {
        uint32_t dummy;
        return __rdtscp(&dummy);
    }

    return __rdtscp(aux);
#else
    if(aux)
        *aux = GetCurrentProcessorNumber();

    return hj_cpu_tsc_read();
#endif

#elif defined(__linux__)
#if defined(__GNUC__) || defined(__clang__)
#if defined(__i386__) || defined(__x86_64__)
    uint32_t lo, hi, cpu_id_val;
    __asm__ volatile("rdtscp" : "=a"(lo), "=d"(hi), "=c"(cpu_id_val)::"memory");
    if(aux)
        *aux = cpu_id_val;

    return ((uint64_t) hi << 32) | lo;
#else
    if(aux)
    {
        uint32_t id = 0;
        if(hj_cpu_id(&id) == HJ_CPU_OK)
            *aux = id;
        else
            *aux = 0;
    }

    return hj_cpu_tsc_read();
#endif
#else
    if(aux)
    {
        uint32_t id = 0;
        if(hj_cpu_id(&id) == HJ_CPU_OK)
            *aux = id;
        else
            *aux = 0;
    }

    return hj_cpu_tsc_read();
#endif

#elif defined(__APPLE__)
    if(aux)
    {
        uint32_t id = 0;
        if(hj_cpu_id(&id) == HJ_CPU_OK)
            *aux = id;
        else
            *aux = 0;
    }

    return hj_cpu_tsc_read();

#else
    if(aux)
    {
        uint32_t id = 0;
        if(hj_cpu_id(&id) == HJ_CPU_OK)
            *aux = id;
        else
            *aux = 0;
    }

    return hj_cpu_tsc_read();
#endif
}

#ifdef __cplusplus
}
#endif

#endif // HJ_CPU_IMPL && !HJ_CPU_IMPL_DONE