
#include <gtest/gtest.h>
#include <hj/testing/benchmark.hpp>

TEST(benchmark, header_compiles)
{
    SUCCEED();
}

static void bm_simple_loop(benchmark::State &state)
{
    for(auto _ : state)
    {
        int sum = 0;
        for(int i = 0; i < 1000; ++i)
            sum += i;
        benchmark::DoNotOptimize(sum);
    }
}

BENCHMARK(bm_simple_loop);
