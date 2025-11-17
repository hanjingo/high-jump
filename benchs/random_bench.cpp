#include <benchmark/benchmark.h>
#include <type_traits>
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
        hj::random::engine rng; // construct outside timing
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
        hj::random::engine rng;
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
        hj::random::engine rng;
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
        hj::random::engine rng;
        state.ResumeTiming();

        double v = rng.normal<double>(100.0, 10.0);
        benchmark::DoNotOptimize(v);
    }
}

// Detection for free-function NTTP overload: hj::random::range<Min,Max>()
namespace
{
template <int Min, int Max, typename = void>
struct has_range_nttp : std::false_type
{
};

template <int Min, int Max>
struct has_range_nttp<Min,
                      Max,
                      std::void_t<decltype(hj::random::range<Min, Max>())>>
    : std::true_type
{
};

} // namespace

// Benchmark the compile-time (NTTP) integer range if available; otherwise
// fall back to the runtime engine call.
static void bm_random_range_nttp(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::random::engine rng;
        state.ResumeTiming();

        int v;
        if constexpr(has_range_nttp<1, 5>::value)
            v = hj::random::range<1, 5>();
        else
            v = rng.range<int>(1, 5);

        benchmark::DoNotOptimize(v);
    }
}

} // namespace

BENCHMARK(bm_random_range_int)->Iterations(200000);
BENCHMARK(bm_random_range_real)->Iterations(200000);
BENCHMARK(bm_random_range_bulk)->Iterations(5000);
BENCHMARK(bm_random_normal)->Iterations(200000);
