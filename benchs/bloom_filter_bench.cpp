#include <benchmark/benchmark.h>
#include <hj/algo/bloom_filter.hpp>
#include <random>
#include <string>

using namespace hj;

static void bm_bloom_filter_add(benchmark::State &state)
{
    size_t       n = state.range(0);
    bloom_filter bf(n, 0.01);
    std::mt19937 rng(42);
    for(auto _ : state)
    {
        for(size_t i = 0; i < n; ++i)
        {
            bf.add(std::to_string(i));
        }
    }
    state.SetItemsProcessed(n * state.iterations());
}

static void bm_bloom_filter_contains(benchmark::State &state)
{
    size_t       n = state.range(0);
    bloom_filter bf(n, 0.01);
    for(size_t i = 0; i < n; ++i)
    {
        bf.add(std::to_string(i));
    }
    std::mt19937 rng(42);
    for(auto _ : state)
    {
        for(size_t i = 0; i < n; ++i)
        {
            benchmark::DoNotOptimize(bf.contains(std::to_string(i)));
        }
    }
    state.SetItemsProcessed(n * state.iterations());
}

static void bm_bloom_filter_serialize(benchmark::State &state)
{
    size_t       n = state.range(0);
    bloom_filter bf(n, 0.01);
    for(size_t i = 0; i < n; ++i)
    {
        bf.add(std::to_string(i));
    }
    std::string data;
    for(auto _ : state)
    {
        bf.serialize(data);
        benchmark::DoNotOptimize(data);
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK(bm_bloom_filter_add)->Arg(100)->Arg(1000)->Arg(10000);
BENCHMARK(bm_bloom_filter_contains)->Arg(100)->Arg(1000)->Arg(10000);
BENCHMARK(bm_bloom_filter_serialize)->Arg(100)->Arg(1000)->Arg(10000);

BENCHMARK_MAIN();