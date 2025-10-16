#include <benchmark/benchmark.h>
#include <hj/io/ring_buffer.hpp>
#include <vector>
#include <numeric>

// Benchmarks for hj::ring_buffer (boost::circular_buffer alias)
namespace
{

static void bm_ring_push_back_overwrite(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::ring_buffer<int> buf{3};
        state.ResumeTiming();

        buf.push_back(1);
        buf.push_back(2);
        buf.push_back(3);
        // overwrite
        buf.push_back(4);
        benchmark::DoNotOptimize(buf);
    }
}

static void bm_ring_front_back_access(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::ring_buffer<int> buf{100};
        for(int i = 0; i < 100; ++i)
            buf.push_back(i);
        state.ResumeTiming();

        int f = buf.front();
        int b = buf.back();
        benchmark::DoNotOptimize(f);
        benchmark::DoNotOptimize(b);
    }
}

static void bm_ring_at_and_index(benchmark::State &state)
{
    const int N = 64;
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::ring_buffer<int> buf{N};
        for(int i = 0; i < N; ++i)
            buf.push_back(i);
        state.ResumeTiming();

        int v1 = buf.at(0);
        int v2 = buf[0];
        benchmark::DoNotOptimize(v1);
        benchmark::DoNotOptimize(v2);
    }
}

static void bm_ring_iterator_sum(benchmark::State &state)
{
    const int N = 256;
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::ring_buffer<int> buf{N};
        for(int i = 0; i < N; ++i)
            buf.push_back(i);
        state.ResumeTiming();

        long long sum = 0;
        for(auto v : buf)
            sum += v;
        benchmark::DoNotOptimize(sum);
    }
}

static void bm_ring_pop_front_pop_back(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::ring_buffer<int> buf{8};
        for(int i = 0; i < 8; ++i)
            buf.push_back(i);
        state.ResumeTiming();

        buf.pop_front();
        buf.pop_back();
        benchmark::DoNotOptimize(buf.size());
    }
}

static void bm_ring_size_capacity_clear(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::ring_buffer<int> buf{16};
        for(int i = 0; i < 8; ++i)
            buf.push_back(i);
        state.ResumeTiming();

        auto s = buf.size();
        auto c = buf.capacity();
        buf.clear();
        benchmark::DoNotOptimize(s);
        benchmark::DoNotOptimize(c);
    }
}

} // namespace

BENCHMARK(bm_ring_push_back_overwrite)->Iterations(2000);
BENCHMARK(bm_ring_front_back_access)->Iterations(5000);
BENCHMARK(bm_ring_at_and_index)->Iterations(5000);
BENCHMARK(bm_ring_iterator_sum)->Iterations(2000);
BENCHMARK(bm_ring_pop_front_pop_back)->Iterations(5000);
BENCHMARK(bm_ring_size_capacity_clear)->Iterations(5000);
