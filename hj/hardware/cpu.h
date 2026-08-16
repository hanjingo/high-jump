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
    HJ_CPU_ERR_SYSCONF_FAILED           = -9
} hj_cpu_err_t;

// ------------------------ CPU API Declarations ------------------------

HJ_CPU_API hj_cpu_err_t hj_cpu_brand(char *buf, size_t size);
HJ_CPU_API hj_cpu_err_t hj_cpu_vendor(char *buf, size_t size);
HJ_CPU_API hj_cpu_err_t hj_cpu_core_num(unsigned int *num);
HJ_CPU_API hj_cpu_err_t hj_cpu_core_bind(const unsigned int core);
HJ_CPU_API hj_cpu_err_t hj_cpu_core_list(unsigned int *buf, unsigned int *len);
HJ_CPU_API hj_cpu_err_t hj_cpu_id(uint32_t *cpu_id);
HJ_CPU_API void         hj_cpu_pause(void);
HJ_CPU_API void         hj_cpu_nop(void);
HJ_CPU_API void         hj_cpu_delay(uint64_t cycles);
HJ_CPU_API void         hj_cpu_cache_flush(const void *addr);
HJ_CPU_API void         hj_cpu_prefetch_read(const void *addr);
HJ_CPU_API void         hj_cpu_prefetch_write(const void *addr);
HJ_CPU_API uint64_t     hj_cpu_tsc_read(void);
HJ_CPU_API uint64_t     hj_cpu_tscp_read(uint32_t *aux);
HJ_CPU_API uint64_t     hj_cpu_pmu_cycle_counter_read(void);

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
#ifdef __GLIBC__
#include <sched.h>
#endif

#else
#pragma message("Unknown OS, some functions will be disabled")
#endif

#ifdef __cplusplus
extern "C" {
#endif

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
        if(!strstr(line, "model name") && !strstr(line, "Hardware")
           && !strstr(line, "Processor"))
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
        if(!strstr(line, "vendor_id") && !strstr(line, "CPU implementer"))
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

HJ_CPU_API hj_cpu_err_t hj_cpu_core_num(unsigned int *num)
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
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(core, &mask);
    if(pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) == 0)
        return HJ_CPU_OK;
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

HJ_CPU_API hj_cpu_err_t hj_cpu_core_list(unsigned int *buf, unsigned int *len)
{
    if(!buf || !len)
    {
        if(len)
            *len = 0;

        return HJ_CPU_ERR_INVALID_ARG;
    }

    unsigned int max_capacity = *len;

#if defined(_WIN32) || defined(_WIN64)
    DWORD returnLength = 0;
    GetLogicalProcessorInformation(NULL, &returnLength);
    if(returnLength == 0)
    {
        *len = 0;
        return HJ_CPU_ERR_INTERNAL;
    }

    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer =
        (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION) malloc(returnLength);
    if(!buffer)
    {
        *len = 0;
        return HJ_CPU_ERR_ALLOCATION_MEMORY_FAILED;
    }

    if(!GetLogicalProcessorInformation(buffer, &returnLength))
    {
        free(buffer);
        *len = 0;
        return HJ_CPU_ERR_INTERNAL;
    }

    unsigned int count = 0;
    DWORD n = returnLength / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
    DWORD i;
    unsigned int j;
    for(i = 0; i < n && count < max_capacity; ++i)
    {
        if(buffer[i].Relationship != RelationProcessorCore)
            continue;

        DWORD_PTR mask = buffer[i].ProcessorMask;
        for(j = 0; j < sizeof(DWORD_PTR) * 8 && count < max_capacity; ++j)
        {
            if(mask & ((DWORD_PTR) 1 << j))
                buf[count++] = j;
        }
    }

    *len = count;
    free(buffer);
    return HJ_CPU_OK;

#elif defined(__linux__)
    long ncpu = sysconf(_SC_NPROCESSORS_ONLN);
    if(ncpu <= 0)
    {
        *len = 0;
        return HJ_CPU_ERR_SYSCONF_FAILED;
    }

    unsigned int count    = 0;
    unsigned int max_cpus = (unsigned int) ncpu;
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
        {
            *len = 0;
            return HJ_CPU_ERR_SYSCTL_FAILED;
        }
    }

    unsigned int count = 0;
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
    Sleep(0);
#endif

#elif defined(__GNUC__) || defined(__clang__)
#if defined(__i386__) || defined(__x86_64__)
    __asm__ volatile("pause" ::: "memory");
#elif defined(__arm__) || defined(__aarch64__)
    __asm__ volatile("yield" ::: "memory");
#else
    sched_yield();
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

HJ_CPU_API void hj_cpu_delay(uint64_t cycles)
{
    if(cycles == 0)
        return;

#if defined(_M_IX86) || defined(_M_X64) || defined(__i386__)                   \
    || defined(__x86_64__)
    uint64_t start = hj_cpu_tsc_read();
    while((hj_cpu_tsc_read() - start) < cycles)
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
    } while((current - start) < cycles);
#elif defined(_MSC_VER)
    uint64_t start = _ReadStatusReg(ARM64_CNTVCT);
    while((_ReadStatusReg(ARM64_CNTVCT) - start) < cycles)
    {
        hj_cpu_pause();
    }
#else
    volatile uint64_t i;
    for(i = 0; i < cycles; ++i)
        hj_cpu_pause();
#endif

#else
    volatile uint64_t i;
    for(i = 0; i < cycles; ++i)
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
#else
    MemoryBarrier();
    (void) addr;
#endif

#elif defined(__linux__)
#if defined(__GNUC__) || defined(__clang__)
#if defined(__i386__) || defined(__x86_64__)
    __builtin_ia32_clflush(addr);
#else
    __builtin___clear_cache((char *) addr, (char *) addr + 64);
#endif
#else
    __sync_synchronize();
    (void) addr;
#endif

#elif defined(__APPLE__)
#if defined(__GNUC__) || defined(__clang__)
    __builtin___clear_cache((char *) addr, (char *) addr + 64);
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
    volatile const char *ptr = (const char *) addr;
    (void) *ptr;
#endif
}

HJ_CPU_API void hj_cpu_prefetch_write(const void *addr)
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
    __builtin_prefetch(addr, 1, 3);

#else
    volatile char *ptr = (char *) addr;
    *ptr               = *ptr;

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

HJ_CPU_API uint64_t hj_cpu_pmu_cycle_counter_read(void)
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
#if defined(__i386__) || defined(__x86_64__)
#if defined(__GNUC__) || defined(__clang__)
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
    return hj_cpu_tsc_read();

#endif
}

#ifdef __cplusplus
}
#endif

#endif // HJ_CPU_IMPL && !HJ_CPU_IMPL_DONE