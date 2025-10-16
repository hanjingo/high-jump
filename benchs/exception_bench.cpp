#include <benchmark/benchmark.h>

#include <hj/testing/exception.hpp>
#include <string>
#include <vector>

using namespace hj;

static void bm_throw_if_false_no_throw(benchmark::State &state)
{
    for(auto _ : state)
    {
        // no throw path
        hj::throw_if_false(true);
        benchmark::ClobberMemory();
    }
}

static void bm_throw_if_false_throw(benchmark::State &state)
{
    for(auto _ : state)
    {
        try
        {
            hj::throw_if_false(false);
        }
        catch(const std::exception &)
        {
        }

        benchmark::ClobberMemory();
    }
}

static void bm_throw_if_not_false_no_throw(benchmark::State &state)
{
    for(auto _ : state)
    {
        hj::throw_if_not_false(false);
        benchmark::ClobberMemory();
    }
}

static void bm_throw_if_not_false_throw(benchmark::State &state)
{
    for(auto _ : state)
    {
        try
        {
            hj::throw_if_not_false(true);
        }
        catch(const std::exception &)
        {
        }

        benchmark::ClobberMemory();
    }
}

static void bm_throw_if_equal_no_throw(benchmark::State &state)
{
    int a = 1, b = 2;
    for(auto _ : state)
    {
        hj::throw_if_equal(a, b);
        benchmark::ClobberMemory();
    }
}

static void bm_throw_if_equal_throw(benchmark::State &state)
{
    int a = 1, b = 1;
    for(auto _ : state)
    {
        try
        {
            hj::throw_if_equal(a, b);
        }
        catch(const std::exception &)
        {
        }

        benchmark::ClobberMemory();
    }
}

static void bm_throw_if_empty_no_throw(benchmark::State &state)
{
    std::string s{"hello"};
    for(auto _ : state)
    {
        hj::throw_if_empty(s);
        benchmark::ClobberMemory();
    }
}

static void bm_throw_if_empty_throw(benchmark::State &state)
{
    std::string s{};
    for(auto _ : state)
    {
        try
        {
            hj::throw_if_empty(s);
        }
        catch(const std::exception &)
        {
        }

        benchmark::ClobberMemory();
    }
}

static void bm_throw_if_not_empty_no_throw(benchmark::State &state)
{
    std::string s{};
    for(auto _ : state)
    {
        hj::throw_if_not_empty(s);
        benchmark::ClobberMemory();
    }
}

static void bm_throw_if_not_empty_throw(benchmark::State &state)
{
    std::string s{"x"};
    for(auto _ : state)
    {
        try
        {
            hj::throw_if_not_empty(s);
        }
        catch(const std::exception &)
        {
        }

        benchmark::ClobberMemory();
    }
}

static void bm_throw_if_null_no_throw(benchmark::State &state)
{
    int x = 0;
    for(auto _ : state)
    {
        hj::throw_if_null(&x);
        benchmark::ClobberMemory();
    }
}

static void bm_throw_if_null_throw(benchmark::State &state)
{
    for(auto _ : state)
    {
        try
        {
            hj::throw_if_null((void *) nullptr);
        }
        catch(const std::exception &)
        {
        }

        benchmark::ClobberMemory();
    }
}

static void bm_throw_if_exists_no_throw(benchmark::State &state)
{
    std::vector<int> v{1, 2, 3};
    for(auto _ : state)
    {
        hj::throw_if_exists(v, 0);
        benchmark::ClobberMemory();
    }
}

static void bm_throw_if_exists_throw(benchmark::State &state)
{
    std::vector<int> v{1, 2, 3};
    for(auto _ : state)
    {
        try
        {
            hj::throw_if_exists(v, 2);
        }
        catch(const std::exception &)
        {
        }

        benchmark::ClobberMemory();
    }
}

// register benchmarks
BENCHMARK(bm_throw_if_false_no_throw);
BENCHMARK(bm_throw_if_false_throw);
BENCHMARK(bm_throw_if_not_false_no_throw);
BENCHMARK(bm_throw_if_not_false_throw);
BENCHMARK(bm_throw_if_equal_no_throw);
BENCHMARK(bm_throw_if_equal_throw);
BENCHMARK(bm_throw_if_empty_no_throw);
BENCHMARK(bm_throw_if_empty_throw);
BENCHMARK(bm_throw_if_not_empty_no_throw);
BENCHMARK(bm_throw_if_not_empty_throw);
BENCHMARK(bm_throw_if_null_no_throw);
BENCHMARK(bm_throw_if_null_throw);
BENCHMARK(bm_throw_if_exists_no_throw);
BENCHMARK(bm_throw_if_exists_throw);