#include <benchmark/benchmark.h>

#include <hj/algo/fill.hpp>
#include <vector>
#include <string>
#include <stdexcept>

using namespace hj;

static void bm_fill_vector_small(benchmark::State &state)
{
    for(auto _ : state)
    {
        std::vector<int> v(64);
        hj::fill(v, [](size_t idx) { return static_cast<int>(idx % 7); });
        benchmark::ClobberMemory();
    }
}

static void bm_fill_vector_large(benchmark::State &state)
{
    for(auto _ : state)
    {
        std::vector<int> v(100000);
        hj::fill(v, [](size_t idx) { return static_cast<int>(idx % 256); });
        benchmark::ClobberMemory();
    }
}

static void bm_fill_iterator_range(benchmark::State &state)
{
    for(auto _ : state)
    {
        std::vector<int> v(1000);
        hj::fill(v.begin(), v.end(), [](size_t idx) {
            return static_cast<int>(idx % 13);
        });
        benchmark::ClobberMemory();
    }
}

static void bm_fill_partial_n(benchmark::State &state)
{
    for(auto _ : state)
    {
        std::vector<int> v(1000, 0);
        hj::fill(v, [](size_t idx) { return 1; }, 10);
        benchmark::ClobberMemory();
    }
}

static void bm_fill_string_type(benchmark::State &state)
{
    for(auto _ : state)
    {
        std::vector<std::string> v(1000);
        hj::fill(v, [](size_t idx) { return std::to_string(idx); });
        benchmark::ClobberMemory();
    }
}

static void bm_fill_exception_path(benchmark::State &state)
{
    for(auto _ : state)
    {
        std::vector<int> v(16);
        try
        {
            hj::fill(v, [](size_t idx) -> int {
                if(idx == 8)
                    throw std::runtime_error("boom");
                return 1;
            });
        }
        catch(const std::exception &)
        {
            // swallow - we measure cost of throwing/catching
        }
        benchmark::ClobberMemory();
    }
}

BENCHMARK(bm_fill_vector_small);
BENCHMARK(bm_fill_vector_large);
BENCHMARK(bm_fill_iterator_range);
BENCHMARK(bm_fill_partial_n);
BENCHMARK(bm_fill_string_type);
BENCHMARK(bm_fill_exception_path);
