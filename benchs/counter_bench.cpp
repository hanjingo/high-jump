#include <benchmark/benchmark.h>
#include <hj/sync/counter.hpp>

#include <thread>
#include <vector>
#include <sstream>

using hj::counter;

// Single-threaded increment microbenchmark
static void bm_counter_inc_single(benchmark::State &st)
{
    const int ops = 100000;
    for(auto _ : st)
    {
        counter<int> ct(0,
                        std::numeric_limits<int>::min(),
                        std::numeric_limits<int>::max(),
                        1);
        for(int i = 0; i < ops; ++i)
            ct.inc();
        benchmark::DoNotOptimize(ct.value());
    }
}
BENCHMARK(bm_counter_inc_single)->Iterations(5);

// Single-threaded decrement microbenchmark
static void bm_counter_dec_single(benchmark::State &st)
{
    const int ops = 100000;
    for(auto _ : st)
    {
        counter<int> ct(ops, 0, std::numeric_limits<int>::max(), 1);
        for(int i = 0; i < ops; ++i)
            ct.dec();
        benchmark::DoNotOptimize(ct.value());
    }
}
BENCHMARK(bm_counter_dec_single)->Iterations(5);

// Reset cost benchmark
static void bm_counter_reset(benchmark::State &st)
{
    for(auto _ : st)
    {
        counter<int> ct(42, 0, 1000, 1);
        for(int i = 0; i < 1000; ++i)
            ct.reset(i % 1000);
        benchmark::DoNotOptimize(ct.value());
    }
}
BENCHMARK(bm_counter_reset)->Iterations(10);

// Operator += benchmark (atomic compare_exchange loops)
static void bm_counter_operator_add_assign(benchmark::State &st)
{
    for(auto _ : st)
    {
        counter<int> a(0);
        counter<int> b(1);
        for(int i = 0; i < 10000; ++i)
            a += b;
        benchmark::DoNotOptimize(a.value());
    }
}
BENCHMARK(bm_counter_operator_add_assign)->Iterations(10);

// Multi-threaded increments: fixed number of threads, variable per-thread ops
static void bm_counter_multithread_inc(benchmark::State &st)
{
    const int per_thread = static_cast<int>(st.range(0));
    const int threads    = 4;
    for(auto _ : st)
    {
        counter<int>             ct(0, 0, threads * per_thread, 1);
        std::vector<std::thread> ths;
        for(int t = 0; t < threads; ++t)
        {
            ths.emplace_back([&ct, per_thread]() {
                for(int i = 0; i < per_thread; ++i)
                    ct.inc();
            });
        }
        for(auto &th : ths)
            th.join();

        benchmark::DoNotOptimize(ct.value());
    }
}
BENCHMARK(bm_counter_multithread_inc)->Arg(1000)->Arg(5000)->Arg(20000);

// Multi-threaded decrements: fixed number of threads, variable per-thread ops
static void bm_counter_multithread_dec(benchmark::State &st)
{
    const int per_thread = static_cast<int>(st.range(0));
    const int threads    = 4;
    for(auto _ : st)
    {
        counter<int> ct(threads * per_thread, 0, threads * per_thread, 1);
        std::vector<std::thread> ths;
        for(int t = 0; t < threads; ++t)
        {
            ths.emplace_back([&ct, per_thread]() {
                for(int i = 0; i < per_thread; ++i)
                    ct.dec();
            });
        }
        for(auto &th : ths)
            th.join();

        benchmark::DoNotOptimize(ct.value());
    }
}
BENCHMARK(bm_counter_multithread_dec)->Arg(1000)->Arg(5000)->Arg(20000);
