#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32) || defined(_WIN64)
#include <intrin.h>
#include <windows.h>

#elif defined(__APPLE__)
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <unistd.h>

#include <libkern/OSCacheControl.h>
#include <mach/mach_time.h>
#include <sys/param.h>
#include <sys/sysctl.h>

#elif defined(__linux__)
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#ifdef __GLIBC__
#include <sched.h>
#endif

#else
#pragma message("Unknown OS, some functions will be disabled")
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------- API define
// ------------------------------------
unsigned int cpu_core_num (void);
bool cpu_core_bind (const unsigned int core);
void cpu_core_list (unsigned int *buf, unsigned int *len);
uint32_t cpu_id (void);
void cpu_pause (void);
void cpu_nop (void);
void cpu_delay (uint64_t cycles);
void cpu_cache_flush (const void *addr);
void cpu_prefetch_read (const void *addr);
void cpu_prefetch_write (const void *addr);
uint64_t cpu_tsc_read (void);
uint64_t cpu_tscp_read (uint32_t *aux);
uint64_t cpu_pmu_cycle_counter_read (void);

// ------------------------ API implementation
// ---------------------------------
unsigned int cpu_core_num (void)
{
#if defined(_WIN32) || defined(_WIN64)
    SYSTEM_INFO sysinfo;
    GetSystemInfo (&sysinfo);
    return sysinfo.dwNumberOfProcessors;

#elif defined(__APPLE__)
    int nm[2];
    size_t len = 4;
    unsigned int count;

    nm[0] = CTL_HW;
    nm[1] = HW_AVAILCPU;
    sysctl (nm, 2, &count, &len, NULL, 0);

    if (count < 1) {
        nm[1] = HW_NCPU;
        sysctl (nm, 2, &count, &len, NULL, 0);
        if (count < 1)
            count = 1;
    }
    return count;

#elif defined(__linux__)
    long result = sysconf (_SC_NPROCESSORS_ONLN);
    return (result > 0) ? (unsigned int) result : 1;

#else
    return 1;
#endif
}

bool cpu_core_bind (const unsigned int core)
{
#if defined(_WIN32) || defined(_WIN64)
    HANDLE hThread = GetCurrentThread ();
    DWORD_PTR mask =
      SetThreadAffinityMask (hThread, (DWORD_PTR) (1LLU << core));
    return (mask != 0);

#elif defined(__linux__)
    cpu_set_t mask;
    CPU_ZERO (&mask);
    CPU_SET (core, &mask);
    return (pthread_setaffinity_np (pthread_self (), sizeof (mask), &mask)
            == 0);

#elif defined(__APPLE__)
    (void) core;
    return false;

#else
    (void) core;
    return false;
#endif
}

void cpu_core_list (unsigned int *buf, unsigned int *len)
{
    if (!buf || !len) {
        if (len)
            *len = 0;
        return;
    }

#if defined(_WIN32) || defined(_WIN64)
    DWORD returnLength = 0;
    GetLogicalProcessorInformation (NULL, &returnLength);
    if (returnLength == 0) {
        *len = 0;
        return;
    }

    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer =
      (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION) malloc (returnLength);
    if (!buffer) {
        *len = 0;
        return;
    }

    if (!GetLogicalProcessorInformation (buffer, &returnLength)) {
        free (buffer);
        *len = 0;
        return;
    }

    unsigned int count = 0;
    DWORD n = returnLength / sizeof (SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
    DWORD i;
    unsigned int j;
    for (i = 0; i < n && count < *len; ++i) {
        if (buffer[i].Relationship != RelationProcessorCore)
            continue;

        DWORD_PTR mask = buffer[i].ProcessorMask;
        for (j = 0; j < sizeof (DWORD_PTR) * 8 && count < *len; ++j) {
            if (mask & ((DWORD_PTR) 1 << j))
                buf[count++] = j;
        }
    }

    *len = count;
    free (buffer);
    return;

#elif defined(__linux__)
    long ncpu = sysconf (_SC_NPROCESSORS_ONLN);
    if (ncpu <= 0) {
        *len = 0;
        return;
    }

    unsigned int count = 0;
    unsigned int max_cpus = (unsigned int) ncpu;
    unsigned int i;
    for (i = 0; i < max_cpus && count < *len; ++i)
        buf[count++] = i;

    *len = count;
    return;

#elif defined(__APPLE__)
    int nm[2];
    size_t len_cpu = sizeof (unsigned int);
    unsigned int ncpu = 0;

    nm[0] = CTL_HW;
    nm[1] = HW_NCPU;
    if (sysctl (nm, 2, &ncpu, &len_cpu, NULL, 0) != 0 || ncpu == 0) {
        *len = 0;
        return;
    }

    unsigned int count = 0;
    unsigned int i;
    for (i = 0; i < ncpu && count < *len; ++i)
        buf[count++] = i;

    *len = count;
    return;

#else
    *len = 0;
    return;

#endif
}

uint32_t cpu_id (void)
{
#if defined(_WIN32) || defined(_WIN64)
    return GetCurrentProcessorNumber ();

#elif defined(__linux__)
#ifdef __GLIBC__
    int cpu = sched_getcpu ();
    return (cpu >= 0) ? (uint32_t) cpu : 0;
#else
    FILE *file = fopen ("/proc/self/stat", "r");
    if (!file)
        return 0;

    char buffer[1024];
    if (!fgets (buffer, sizeof (buffer), file)) {
        fclose (file);
        return 0;
    }

    unsigned int cpu = 0;
    char *ptr = buffer;
    int field_count = 0;
    while (*ptr && field_count < 38) {
        if (*ptr == ' ')
            field_count++;

        ptr++;
    }

    if (field_count != 38 || sscanf (ptr, "%u", &cpu) != 1)
        cpu = 0;

    fclose (file);
    return cpu;
#endif

#elif defined(__APPLE__)
    pthread_t thread_id = pthread_self ();
#if defined(__LP64__)
    return (uint32_t) ((uintptr_t) thread_id % 1024);

#else
    return (uint32_t) ((uintptr_t) thread_id % 1024);

#endif

#else
    return 0;

#endif
}

void cpu_pause (void)
{
#if defined(_WIN32) || defined(_WIN64)
#if defined(_M_IX86) || defined(_M_X64)
    _mm_pause ();
#else
    Sleep (0);
#endif

#elif defined(__linux__)
    sched_yield ();

#elif defined(__APPLE__)
    sched_yield ();

#else
    (void) 0;

#endif
}

void cpu_nop (void)
{
#if defined(_WIN32) || defined(_WIN64)
#if defined(_M_IX86) || defined(_M_X64)
    __nop ();
#else
    volatile int dummy = 0;
    (void) dummy;
#endif

#elif defined(__GNUC__) || defined(__clang__)
#if defined(__i386__) || defined(__x86_64__)
    __asm__ volatile ("nop" ::: "memory");
#else
    __asm__ volatile ("" ::: "memory");
#endif

#else
    volatile int dummy = 0;
    (void) dummy;

#endif
}

void cpu_delay (uint64_t cycles)
{
    if (cycles == 0)
        return;

#if defined(_WIN32) || defined(_WIN64)
    LARGE_INTEGER frequency, start, current;

    if (QueryPerformanceFrequency (&frequency)
        && QueryPerformanceCounter (&start)) {
        // if cpu frequency is 3GHz, 1 cycle is about 0.33 nanoseconds
        // 1000 cycles is about 0.33 microseconds
        uint64_t target_microseconds = cycles / 3000;
        if (target_microseconds == 0)
            target_microseconds = 1;

        uint64_t target_ticks =
          (target_microseconds * frequency.QuadPart) / 1000000;
        do {
            cpu_pause ();
            QueryPerformanceCounter (&current);
        } while ((uint64_t) (current.QuadPart - start.QuadPart) < target_ticks);
    } else {
        if (cycles > 3000000) {
            Sleep ((DWORD) (cycles / 3000000));
        } else {
            volatile uint64_t i;
            for (i = 0; i < cycles / 100; ++i)
                cpu_pause ();
        }
    }

#elif defined(__linux__) || defined(__APPLE__)

    struct timespec start, current;

    if (clock_gettime (CLOCK_MONOTONIC, &start) == 0) {
        uint64_t target_nanoseconds = (cycles * 1000) / 3000;
        if (target_nanoseconds == 0)
            target_nanoseconds = 1;

        uint64_t target_ns = start.tv_nsec + target_nanoseconds;
        time_t target_sec = start.tv_sec;
        if (target_ns >= 1000000000) {
            target_sec += target_ns / 1000000000;
            target_ns %= 1000000000;
        }

        do {
            cpu_pause ();
            clock_gettime (CLOCK_MONOTONIC, &current);
        } while (current.tv_sec < target_sec
                 || (current.tv_sec == target_sec
                     && (uint64_t) current.tv_nsec < target_ns));
    } else {
        if (cycles > 3000) {
            usleep ((useconds_t) (cycles / 3000));
        } else {
            volatile uint64_t i;
            for (i = 0; i < cycles / 100; ++i)
                cpu_pause ();
        }
    }

#else

    volatile uint64_t i;
    for (i = 0; i < cycles; ++i)
        cpu_nop ();

#endif
}

void cpu_cache_flush (const void *addr)
{
    if (!addr)
        return;

#if defined(_WIN32) || defined(_WIN64)
#if defined(_M_IX86) || defined(_M_X64)
    _mm_clflush (addr);
#else
    MemoryBarrier ();
    (void) addr;
#endif

#elif defined(__linux__)
#if defined(__GNUC__) || defined(__clang__)
#if defined(__i386__) || defined(__x86_64__)
    __builtin_ia32_clflush (addr);
#else
    __builtin___clear_cache ((char *) addr, (char *) addr + 64);
#endif
#else
    __sync_synchronize ();
    (void) addr;
#endif

#elif defined(__APPLE__)
#if defined(__GNUC__) || defined(__clang__)
    __builtin___clear_cache ((char *) addr, (char *) addr + 64);
#else
    __sync_synchronize ();
    (void) addr;
#endif

#else
#if defined(__GNUC__) || defined(__clang__)
    __sync_synchronize ();
#endif
    (void) addr;

#endif
}

void cpu_prefetch_read (const void *addr)
{
    if (!addr)
        return;

#if defined(_WIN32) || defined(_WIN64)
#if defined(_M_IX86) || defined(_M_X64)
    _mm_prefetch ((const char *) addr, _MM_HINT_T0);
#else
    (void) addr;
#endif

#elif defined(__GNUC__) || defined(__clang__)
    __builtin_prefetch (addr, 0, 3);

#else
    {
        volatile const char *ptr = (const char *) addr;
        (void) *ptr;
    }
#endif
}

void cpu_prefetch_write (const void *addr)
{
    if (!addr)
        return;

#if defined(_WIN32) || defined(_WIN64)
#if defined(_M_IX86) || defined(_M_X64)
    _mm_prefetch ((const char *) addr, _MM_HINT_T0);
#else
    (void) addr;
#endif

#elif defined(__GNUC__) || defined(__clang__)
    __builtin_prefetch (addr, 1, 3);

#else
    {
        volatile char *ptr = (char *) addr;
        *ptr = *ptr;
    }
#endif
}

uint64_t cpu_tsc_read (void)
{
#if defined(_WIN32) || defined(_WIN64)
#if defined(_M_IX86) || defined(_M_X64)
    return __rdtsc ();
#else
    LARGE_INTEGER counter;
    if (QueryPerformanceCounter (&counter))
        return (uint64_t) counter.QuadPart;

    return 0;
#endif

#elif defined(__linux__)
#if defined(__GNUC__) || defined(__clang__)
#if defined(__i386__) || defined(__x86_64__)
    return __builtin_ia32_rdtsc ();
#else
    {
        struct timespec ts;
        if (clock_gettime (CLOCK_MONOTONIC_RAW, &ts) == 0)
            return (uint64_t) ts.tv_sec * 1000000000ULL + (uint64_t) ts.tv_nsec;

        return 0;
    }
#endif
#else
    {
        struct timespec ts;
        if (clock_gettime (CLOCK_MONOTONIC_RAW, &ts) == 0)
            return (uint64_t) ts.tv_sec * 1000000000ULL + (uint64_t) ts.tv_nsec;

        return 0;
    }
#endif

#elif defined(__APPLE__)
    return mach_absolute_time ();

#else
    {
        clock_t c = clock ();
        return (uint64_t) c;
    }

#endif
}

uint64_t cpu_tscp_read (uint32_t *aux)
{
#if defined(_WIN32) || defined(_WIN64)
#if defined(_M_IX86) || defined(_M_X64)
    if (aux) {
        return __rdtscp (aux);
    } else {
        uint32_t dummy;
        return __rdtscp (&dummy);
    }

#else
    if (aux)
        *aux = GetCurrentProcessorNumber ();

    return cpu_tsc_read ();
#endif

#elif defined(__linux__)
#if defined(__GNUC__) || defined(__clang__)
#if defined(__i386__) || defined(__x86_64__)
    {
        uint32_t lo, hi, cpu_id;
        __asm__ volatile ("rdtscp"
                          : "=a"(lo), "=d"(hi), "=c"(cpu_id)::"memory");
        if (aux)
            *aux = cpu_id;

        return ((uint64_t) hi << 32) | lo;
    }
#else
    if (aux)
        *aux = cpu_id ();

    return cpu_tsc_read ();
#endif
#else
    if (aux)
        *aux = cpu_id ();

    return cpu_tsc_read ();
#endif

#elif defined(__APPLE__)
    if (aux)
        *aux = cpu_id ();

    return cpu_tsc_read ();

#else
    if (aux)
        *aux = cpu_id ();

    return cpu_tsc_read ();
#endif
}

uint64_t cpu_pmu_cycle_counter_read (void)
{
#if defined(_WIN32) || defined(_WIN64)
#if defined(_M_IX86) || defined(_M_X64)
    return __rdtsc ();
#else
    LARGE_INTEGER counter;
    if (QueryPerformanceCounter (&counter))
        return (uint64_t) counter.QuadPart;

    return 0;
#endif

#elif defined(__linux__)
#if defined(__i386__) || defined(__x86_64__)
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_ia32_rdtsc ();
#else
    {
        struct timespec ts;
        if (clock_gettime (CLOCK_MONOTONIC_RAW, &ts) == 0)
            return (uint64_t) ts.tv_sec * 1000000000ULL + (uint64_t) ts.tv_nsec;

        return 0;
    }
#endif
#else
    {
        struct timespec ts;
        if (clock_gettime (CLOCK_MONOTONIC_RAW, &ts) == 0)
            return (uint64_t) ts.tv_sec * 1000000000ULL + (uint64_t) ts.tv_nsec;

        return 0;
    }
#endif

#elif defined(__APPLE__)
    return mach_absolute_time ();

#else
    return cpu_tsc_read ();

#endif
}

#ifdef __cplusplus
}
#endif

#endif // CPU_H