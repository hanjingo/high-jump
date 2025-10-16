#include <benchmark/benchmark.h>
#include <hj/sync/safe_vector.hpp>
#include <vector>
#include <thread>
#include <atomic>

// CI-friendly microbench for hj::safe_vector

static void bm_safe_vector_emplace(benchmark::State &state)
{
    const int N = static_cast<int>(state.range(0));
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::safe_vector<int> vec;
        state.ResumeTiming();

        for(int i = 0; i < N; ++i)
            vec.emplace(i);

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_safe_vector_emplace)->Arg(100)->Arg(1000);

static void bm_safe_vector_emplace_bulk(benchmark::State &state)
{
    const int N = static_cast<int>(state.range(0));
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::safe_vector<int> vec;
        std::vector<int>     vals;
        vals.reserve(N);
        for(int i = 0; i < N; ++i)
            vals.push_back(i);
        state.ResumeTiming();

        vec.emplace_bulk(vals);

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_safe_vector_emplace_bulk)->Arg(100)->Arg(1000);

static void bm_safe_vector_at(benchmark::State &state)
{
    const int N = static_cast<int>(state.range(0));
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::safe_vector<int> vec;
        for(int i = 0; i < N; ++i)
            vec.emplace(i);
        state.ResumeTiming();

        int sum = 0;
        for(int i = 0; i < N; ++i)
            sum += vec.at(i);

        benchmark::DoNotOptimize(sum);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_safe_vector_at)->Arg(100)->Arg(1000);

static void bm_safe_vector_range(benchmark::State &state)
{
    const int N = static_cast<int>(state.range(0));
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::safe_vector<int> vec;
        for(int i = 0; i < N; ++i)
            vec.emplace(i);
        state.ResumeTiming();

        int sum = 0;
        vec.range([&sum](int &v) {
            sum += v;
            return true;
        });

        benchmark::DoNotOptimize(sum);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_safe_vector_range)->Arg(100)->Arg(1000);

static void bm_safe_vector_sort(benchmark::State &state)
{
    const int N = static_cast<int>(state.range(0));
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::safe_vector<int> vec;
        for(int i = N; i > 0; --i)
            vec.emplace(i);
        state.ResumeTiming();

        vec.sort([](const int &a, const int &b) { return a < b; });

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_safe_vector_sort)->Arg(100)->Arg(1000);

static void bm_safe_vector_multithread_emplace(benchmark::State &state)
{
    const int threads    = static_cast<int>(state.range(0));
    const int per_thread = static_cast<int>(state.range(1));

    for(auto _ : state)
    {
        state.PauseTiming();
        hj::safe_vector<int> vec;
        state.ResumeTiming();

        std::vector<std::thread> ths;
        ths.reserve(threads);
        for(int t = 0; t < threads; ++t)
        {
            ths.emplace_back([&vec, t, per_thread]() {
                int base = t * per_thread;
                for(int i = 0; i < per_thread; ++i)
                    vec.emplace(base + i);
            });
        }

        for(auto &th : ths)
            th.join();

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_safe_vector_multithread_emplace)->Args({4, 250})->Args({8, 125});
