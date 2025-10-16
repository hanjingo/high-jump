#include <benchmark/benchmark.h>
#include <hj/os/kernel.h>
#include <string>
#include <cstring>

static void bm_kernel_name(benchmark::State &state)
{
    for(auto _ : state)
    {
        const char *n = kernel_name();
        benchmark::DoNotOptimize(n);
    }
}

static void bm_kernel_version(benchmark::State &state)
{
    for(auto _ : state)
    {
        char        buf[128];
        const char *v = kernel_version(buf, sizeof(buf));
        benchmark::DoNotOptimize(v);
        benchmark::ClobberMemory();
    }
}

static void bm_kernel_uptime(benchmark::State &state)
{
    for(auto _ : state)
    {
        uint64_t up = kernel_uptime();
        benchmark::DoNotOptimize(up);
    }
}

static void bm_kernel_uptime_str(benchmark::State &state)
{
    const char *fmt = "%llu days, %llu hours, %llu minutes, %llu seconds";
    for(auto _ : state)
    {
        char        buf[128];
        const char *s = kernel_uptime_str(buf, sizeof(buf), fmt);
        benchmark::DoNotOptimize(s);
    }
}

static void bm_kernel_info(benchmark::State &state)
{
    for(auto _ : state)
    {
        kernel_info_t info;
        bool          ok = kernel_info(&info);
        benchmark::DoNotOptimize(ok);
        benchmark::DoNotOptimize(info.name[0]);
        benchmark::DoNotOptimize(info.version[0]);
        benchmark::DoNotOptimize(info.uptime_seconds);
    }
}

BENCHMARK(bm_kernel_name)->Iterations(100000);
BENCHMARK(bm_kernel_version)->Iterations(20000);
BENCHMARK(bm_kernel_uptime)->Iterations(20000);
BENCHMARK(bm_kernel_uptime_str)->Iterations(20000);
BENCHMARK(bm_kernel_info)->Iterations(5000);