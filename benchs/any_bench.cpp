#include <benchmark/benchmark.h>
#include <hj/types/any.hpp>
#include <string>
#include <vector>
#include <memory>

static void bm_any_construct_int(benchmark::State &state)
{
    for(auto _ : state)
    {
        hj::any a = 42;
        benchmark::DoNotOptimize(a);
    }
}
BENCHMARK(bm_any_construct_int);

static void bm_any_cast_int(benchmark::State &state)
{
    hj::any a = 123456;
    for(auto _ : state)
    {
        int v = hj::any_cast<int>(a);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(bm_any_cast_int);

static void bm_any_construct_string(benchmark::State &state)
{
    std::string s(256, 'x');
    for(auto _ : state)
    {
        hj::any a = s;
        benchmark::DoNotOptimize(a);
    }
}
BENCHMARK(bm_any_construct_string);

static void bm_any_cast_pointer(benchmark::State &state)
{
    int     x = 999;
    hj::any a = &x;
    for(auto _ : state)
    {
        int *p = hj::any_cast<int *>(a);
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(bm_any_cast_pointer);

static void bm_any_move_semantics(benchmark::State &state)
{
    for(auto _ : state)
    {
        hj::any a = std::string("move_test_string");
        hj::any b = std::move(a);
        benchmark::DoNotOptimize(b);
    }
}
BENCHMARK(bm_any_move_semantics);

static void bm_any_bad_cast_exception(benchmark::State &state)
{
    hj::any a = 1;
    for(auto _ : state)
    {
        try
        {
            (void) hj::any_cast<std::string>(a);
        }
        catch(...)
        {
            // expected
        }
    }
}
BENCHMARK(bm_any_bad_cast_exception);
