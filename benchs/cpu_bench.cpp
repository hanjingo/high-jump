#include <benchmark/benchmark.h>
#include <hj/hardware/cpu.h>

#include <vector>
#include <cstdint>
#include <chrono>

// Benchmark: cost of a single cpu_nop in a tight loop
static void bm_cpu_nop_loop(benchmark::State &st)
{
    const int N = 10000;
    for(auto _ : st)
    {
        for(int i = 0; i < N; ++i)
            cpu_nop();
    }
}
BENCHMARK(bm_cpu_nop_loop)->Iterations(50);

// Benchmark: cpu_pause repeated (short-yield)
static void bm_cpu_pause_loop(benchmark::State &st)
{
    const int N = 1000;
    for(auto _ : st)
    {
        for(int i = 0; i < N; ++i)
            cpu_pause();
    }
}
BENCHMARK(bm_cpu_pause_loop)->Iterations(50);

// Benchmark: cpu_delay with a small cycle count (should be fast on modern hw)
static void bm_cpu_delay_small(benchmark::State &st)
{
    const uint64_t cycles = 1000; // small delay
    for(auto _ : st)
    {
        cpu_delay(cycles);
    }
}
BENCHMARK(bm_cpu_delay_small)->Iterations(20);

// Benchmark: repeated TSC reads
static void bm_cpu_tsc_read_bulk(benchmark::State &st)
{
    const int N = 10000;
    for(auto _ : st)
    {
        uint64_t sum = 0;
        for(int i = 0; i < N; ++i)
            sum += cpu_tsc_read();
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(bm_cpu_tsc_read_bulk)->Iterations(50);

// Benchmark: repeated RDTSCP (TSCP) reads when available
static void bm_cpu_tscp_read_bulk(benchmark::State &st)
{
    const int N = 5000;
    for(auto _ : st)
    {
        uint64_t sum = 0;
        uint32_t aux = 0;
        for(int i = 0; i < N; ++i)
            sum += cpu_tscp_read(&aux);
        benchmark::DoNotOptimize(sum);
        benchmark::DoNotOptimize(aux);
    }
}
BENCHMARK(bm_cpu_tscp_read_bulk)->Iterations(50);

// Benchmark: PMU cycle counter reads
static void bm_cpu_pmu_read_bulk(benchmark::State &st)
{
    const int N = 5000;
    for(auto _ : st)
    {
        uint64_t sum = 0;
        for(int i = 0; i < N; ++i)
            sum += cpu_pmu_cycle_counter_read();
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(bm_cpu_pmu_read_bulk)->Iterations(50);

// Benchmark: cache flush over a buffer
static void bm_cpu_cache_flush(benchmark::State &st)
{
    const size_t      SZ = 4096;
    std::vector<char> buf(SZ, 0x55);
    for(auto _ : st)
    {
        for(size_t i = 0; i < SZ; i += 64)
            cpu_cache_flush(&buf[i]);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_cpu_cache_flush)->Iterations(20);

// Benchmark: prefetch read/write across a buffer
static void bm_cpu_prefetch_read_write(benchmark::State &st)
{
    const size_t     SZ = 1024 * 64;
    std::vector<int> arr(SZ, 1);
    for(auto _ : st)
    {
        int sum = 0;
        for(size_t i = 0; i < SZ; i += 16)
        {
            if(i + 64 < SZ)
                cpu_prefetch_read(&arr[i + 64]);
            sum += arr[i];
        }
        for(size_t i = 0; i < SZ; i += 16)
        {
            if(i + 64 < SZ)
                cpu_prefetch_write(&arr[i + 64]);
            arr[i] = sum & 0xFF;
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(bm_cpu_prefetch_read_write)->Iterations(20);

// Benchmark: cpu_id and cpu_core_num calls
static void bm_cpu_id_corenum(benchmark::State &st)
{
    for(auto _ : st)
    {
        uint32_t     id    = cpu_id();
        unsigned int cores = cpu_core_num();
        benchmark::DoNotOptimize(id);
        benchmark::DoNotOptimize(cores);
    }
}
BENCHMARK(bm_cpu_id_corenum)->Iterations(100);