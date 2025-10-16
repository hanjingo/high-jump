#include <benchmark/benchmark.h>

#include <hj/encoding/fmt.hpp>
#include <fmt/core.h>
#include <string>
#include <vector>

using namespace hj;

static void bm_fmt_small(benchmark::State &state)
{
    for(auto _ : state)
    {
        auto s = fmt::format("{}-{}", "hello", "world");
        benchmark::ClobberMemory();
    }
}

static void bm_fmt_many_args(benchmark::State &state)
{
    for(auto _ : state)
    {
        auto s = fmt::format("{} {} {} {} {} {} {} {} {} {}",
                             1,
                             2,
                             3,
                             4,
                             5,
                             6,
                             7,
                             8,
                             9,
                             10);
        benchmark::ClobberMemory();
    }
}

static void bm_fmt_large_string(benchmark::State &state)
{
    std::string chunk(1024, 'x');
    for(auto _ : state)
    {
        auto s = fmt::format("prefix-{}-suffix", chunk);
        benchmark::ClobberMemory();
    }
}

static void bm_fmt_repeated(benchmark::State &state)
{
    for(auto _ : state)
    {
        for(int i = 0; i < 100; ++i)
        {
            auto s = fmt::format("val:{}", i);
            (void) s;
        }
        benchmark::ClobberMemory();
    }
}

BENCHMARK(bm_fmt_small);
BENCHMARK(bm_fmt_many_args);
BENCHMARK(bm_fmt_large_string);
BENCHMARK(bm_fmt_repeated);
