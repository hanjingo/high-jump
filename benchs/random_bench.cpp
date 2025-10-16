#include <benchmark/benchmark.h>
#include <hj/math/random.hpp>
#include <vector>
#include <string>

// CI-friendly microbenchmarks for hj::random
namespace
{

// Measure integer range generation
static void bm_random_range_int(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::random rng; // construct outside timing
        state.ResumeTiming();

        int v = rng.range<int>(1, 5);
        benchmark::DoNotOptimize(v);
    }
}

// Measure real range generation
static void bm_random_range_real(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::random rng;
        state.ResumeTiming();

        double v = rng.range_real<double>(0.0, 1.0);
        benchmark::DoNotOptimize(v);
    }
}

// Measure bulk generation (produce N values)
static void bm_random_range_bulk(benchmark::State &state)
{
    const size_t N = 100;
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::random rng;
        state.ResumeTiming();

        auto vec = rng.range_bulk<int>(10, 20, N);
        benchmark::DoNotOptimize(vec.size());
        if(!vec.empty())
            benchmark::DoNotOptimize(vec[0]);
    }
}

// Measure normal distribution sampling
static void bm_random_normal(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::random rng;
        state.ResumeTiming();

        double v = rng.normal<double>(100.0, 10.0);
        benchmark::DoNotOptimize(v);
    }
}

} // namespace

BENCHMARK(bm_random_range_int)->Iterations(200000);
BENCHMARK(bm_random_range_real)->Iterations(200000);
BENCHMARK(bm_random_range_bulk)->Iterations(5000);
BENCHMARK(bm_random_normal)->Iterations(200000);
