#include <benchmark/benchmark.h>

#include <hj/types/variant.hpp>
#include <string>

// Construct and get benchmark
static void bm_variant_construct_get(benchmark::State &state)
{
    for(auto _ : state)
    {
        hj::variant<int, double, std::string> v = 42;
        benchmark::DoNotOptimize(hj::get<int>(v));

        v = 3.14;
        benchmark::DoNotOptimize(hj::get<double>(v));

        v = std::string("hello");
        benchmark::DoNotOptimize(hj::get<std::string>(v));

        benchmark::ClobberMemory();
    }
}

// Const get benchmark
static void bm_variant_const_get(benchmark::State &state)
{
    const hj::variant<int, std::string> v = std::string("abc");
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(hj::get<std::string>(v));
        benchmark::ClobberMemory();
    }
}

// Move get benchmark
static void bm_variant_move_get(benchmark::State &state)
{
    for(auto _ : state)
    {
        hj::variant<int, std::string> v = std::string("move");
        auto                          s = hj::get<std::string>(std::move(v));
        benchmark::DoNotOptimize(s);
        benchmark::ClobberMemory();
    }
}

// Type switch benchmark: assign different types repeatedly
static void bm_variant_type_switch(benchmark::State &state)
{
    hj::variant<int, double> v = 1;
    for(auto _ : state)
    {
        v = 1;
        benchmark::DoNotOptimize(hj::get<int>(v));
        v = 2.5;
        benchmark::DoNotOptimize(hj::get<double>(v));
        benchmark::ClobberMemory();
    }
}

// Bad access path: expect an exception when getting wrong type
static void bm_variant_bad_access_throws(benchmark::State &state)
{
    for(auto _ : state)
    {
        hj::variant<int, std::string> v = 123;
        try
        {
            auto s = hj::get<std::string>(v);
            benchmark::DoNotOptimize(s);
        }
        catch(...) // measure throw/catch cost
        {
            benchmark::ClobberMemory();
        }
    }
}

BENCHMARK(bm_variant_construct_get)->Unit(benchmark::kMicrosecond);
BENCHMARK(bm_variant_const_get)->Unit(benchmark::kMicrosecond);
BENCHMARK(bm_variant_move_get)->Unit(benchmark::kMicrosecond);
BENCHMARK(bm_variant_type_switch)->Unit(benchmark::kMicrosecond);
BENCHMARK(bm_variant_bad_access_throws)->Unit(benchmark::kMicrosecond);
