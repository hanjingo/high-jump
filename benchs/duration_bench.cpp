#include <benchmark/benchmark.h>
#include <hj/time/duration.hpp>

static void bm_time_start_overhead(benchmark::State &st)
{
    for(auto _ : st)
    {
        TIME_START();
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_time_start_overhead)->Iterations(100000);

static void bm_time_passed_ms_overhead(benchmark::State &st)
{
    TIME_START();
    for(auto _ : st)
    {
        volatile auto ms = TIME_PASSED_MS();
        benchmark::DoNotOptimize(ms);
    }
}
BENCHMARK(bm_time_passed_ms_overhead)->Iterations(200000);

static void bm_time_passed_ns_overhead(benchmark::State &st)
{
    TIME_START();
    for(auto _ : st)
    {
        volatile auto ns = TIME_PASSED_NS();
        benchmark::DoNotOptimize(ns);
    }
}
BENCHMARK(bm_time_passed_ns_overhead)->Iterations(200000);

static void bm_time_passed_seconds_overhead(benchmark::State &st)
{
    TIME_START();
    for(auto _ : st)
    {
        volatile auto s = TIME_PASSED_SEC();
        benchmark::DoNotOptimize(s);
    }
}
BENCHMARK(bm_time_passed_seconds_overhead)->Iterations(200000);

// Simulate repeated start/passed cycle to measure combined cost
static void bm_start_then_passed_cycle(benchmark::State &st)
{
    for(auto _ : st)
    {
        TIME_START();
        for(int i = 0; i < 8; ++i)
        {
            volatile auto ns = TIME_PASSED_NS();
            benchmark::DoNotOptimize(ns);
        }
    }
}
BENCHMARK(bm_start_then_passed_cycle)->Iterations(50000);
