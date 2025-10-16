#include <benchmark/benchmark.h>
#include <hj/os/env.h>
#include <vector>
#include <cstdint>

// Benchmark reading compile-time macros
static void bm_compile_time_macros(benchmark::State &st)
{
    for(auto _ : st)
    {
        volatile int         y = COMPILE_YEAR;
        volatile int         m = COMPILE_MONTH;
        volatile int         d = COMPILE_DAY;
        volatile const char *t = COMPILE_TIME;
        benchmark::DoNotOptimize(y);
        benchmark::DoNotOptimize(m);
        benchmark::DoNotOptimize(d);
        benchmark::DoNotOptimize(t);
    }
}
BENCHMARK(bm_compile_time_macros)->Iterations(20000);

// Benchmark env_get for a few commonly used keys (lightweight)
static void bm_env_get_core(benchmark::State &st)
{
    for(auto _ : st)
    {
        volatile int64_t cpu = env_get(CONF_CPU_COUNT);
        volatile int64_t mem = env_get(CONF_MEMORY_TOTAL);
        volatile int64_t up  = env_get(CONF_UPTIME_SECONDS);
        benchmark::DoNotOptimize(cpu);
        benchmark::DoNotOptimize(mem);
        benchmark::DoNotOptimize(up);
    }
}
BENCHMARK(bm_env_get_core)->Iterations(5000);

// Benchmark iterating a selection of env keys
static void bm_env_get_many_keys(benchmark::State &st)
{
    const std::vector<conf_t> keys = {CONF_CPU_COUNT,
                                      CONF_MEMORY_TOTAL,
                                      CONF_UPTIME_SECONDS,
                                      CONF_PATH_MAX,
                                      CONF_OPEN_MAX,
                                      CONF_TIMEZONE_OFFSET,
                                      CONF_CLK_TCK};

    for(auto _ : st)
    {
        for(auto k : keys)
        {
            volatile int64_t v = env_get(k);
            benchmark::DoNotOptimize(v);
        }
    }
}
BENCHMARK(bm_env_get_many_keys)->Iterations(2000);

// Benchmark ENV_* convenience macros (they call env_get under the hood)
static void bm_env_macros(benchmark::State &st)
{
    for(auto _ : st)
    {
        volatile int64_t c = ENV_CPU_COUNT;
        volatile int64_t m = ENV_MEMORY_TOTAL;
        volatile int64_t p = ENV_PATH_LEN_MAX;
        benchmark::DoNotOptimize(c);
        benchmark::DoNotOptimize(m);
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(bm_env_macros)->Iterations(5000);