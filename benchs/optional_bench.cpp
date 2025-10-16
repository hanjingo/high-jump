#include <benchmark/benchmark.h>
#include <hj/types/optional.hpp>
#include <string>

using hj::optional;

static void bm_optional_construct_empty(benchmark::State &state)
{
    for(auto _ : state)
    {
        optional<int> o;
        benchmark::DoNotOptimize(o);
    }
}

static void bm_optional_construct_value(benchmark::State &state)
{
    for(auto _ : state)
    {
        optional<int> o(42);
        benchmark::DoNotOptimize(o.value());
    }
}

static void bm_optional_value_or(benchmark::State &state)
{
    optional<int> some(7);
    optional<int> none;
    for(auto _ : state)
    {
        int a = some.value_or(100);
        int b = none.value_or(100);
        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
    }
}

static void bm_optional_has_value_emplace_reset(benchmark::State &state)
{
    optional<std::string> o;
    for(auto _ : state)
    {
        o.emplace("hello");
        benchmark::DoNotOptimize(o->size());
        o = optional<std::string>();
    }
}

static void bm_optional_copy_move(benchmark::State &state)
{
    optional<std::string> base(std::string(64, 'x'));
    for(auto _ : state)
    {
        auto c = base;         // copy
        auto m = std::move(c); // move
        benchmark::DoNotOptimize(m);
    }
}

BENCHMARK(bm_optional_construct_empty)->Iterations(50000);
BENCHMARK(bm_optional_construct_value)->Iterations(50000);
BENCHMARK(bm_optional_value_or)->Iterations(50000);
BENCHMARK(bm_optional_has_value_emplace_reset)->Iterations(20000);
BENCHMARK(bm_optional_copy_move)->Iterations(20000);