#include <benchmark/benchmark.h>
#include <hj/sync/thread_pool.hpp>
#include <atomic>
#include <future>
#include <vector>
#include <unordered_set>
#include <thread>
#include <chrono>

using namespace hj;

// Construct default thread_pool
static void bm_thread_pool_construct_default(benchmark::State &state)
{
    for(auto _ : state)
    {
        thread_pool tp;
        benchmark::DoNotOptimize(tp);
    }
}
BENCHMARK(bm_thread_pool_construct_default)->Iterations(10000);

// Construct with N threads
static void bm_thread_pool_construct_n(benchmark::State &state)
{
    const unsigned long n = static_cast<unsigned long>(state.range(0));
    for(auto _ : state)
    {
        thread_pool tp(n);
        benchmark::DoNotOptimize(tp);
    }
}
BENCHMARK(bm_thread_pool_construct_n)->Arg(1)->Arg(2)->Arg(4);

// Enqueue a task that returns a value
static void bm_thread_pool_enqueue_return(benchmark::State &state)
{
    for(auto _ : state)
    {
        thread_pool tp(2);
        auto        fut = tp.enqueue([]() -> int { return 42; });
        state.PauseTiming();
        int v = fut.get();
        state.ResumeTiming();
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(bm_thread_pool_enqueue_return)->Iterations(1000);

// Clear: enqueue many small tasks then clear
static void bm_thread_pool_clear(benchmark::State &state)
{
    const int n = 100;
    for(auto _ : state)
    {
        thread_pool tp(2);
        for(int i = 0; i < n; ++i)
            tp.enqueue([i]() { (void) i; });

        state.PauseTiming();
        tp.clear();
        state.ResumeTiming();

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_thread_pool_clear)->Iterations(200);

// Multi-threaded enqueue: many futures from multiple producers
static void bm_thread_pool_multi_thread_enqueue(benchmark::State &state)
{
    const int tasks = static_cast<int>(state.range(0));
    for(auto _ : state)
    {
        thread_pool                    tp(4);
        std::atomic<int>               sum{0};
        std::vector<std::future<void>> futs;

        for(int i = 0; i < tasks; ++i)
            futs.push_back(tp.enqueue([&sum]() { sum++; }));

        state.PauseTiming();
        for(auto &f : futs)
            f.get();
        state.ResumeTiming();

        benchmark::DoNotOptimize(sum.load());
    }
}
BENCHMARK(bm_thread_pool_multi_thread_enqueue)->Arg(100)->Arg(1000);

// Enqueue tasks that throw; measure enqueue path, handle exception outside timed region
static void bm_thread_pool_enqueue_exception(benchmark::State &state)
{
    for(auto _ : state)
    {
        thread_pool tp(1);
        state.PauseTiming();
        auto fut =
            tp.enqueue([]() -> int { throw std::runtime_error("fail"); });
        state.ResumeTiming();

        state.PauseTiming();
        try
        {
            fut.get();
        }
        catch(...)
        {
        }
        state.ResumeTiming();
    }
}
BENCHMARK(bm_thread_pool_enqueue_exception)->Iterations(200);

// Affinity constructor: try to bind to a core; skip if affinity not supported/permission denied
static void bm_thread_pool_affinity_ctor(benchmark::State &state)
{
    // pick core 0 for test
    for(auto _ : state)
    {
        try
        {
            std::unordered_set<unsigned int> cores{0};
            thread_pool                      tp(cores);
            benchmark::DoNotOptimize(tp);
        }
        catch(const std::runtime_error &e)
        {
            state.SkipWithError(e.what());
            return;
        }
    }
}
BENCHMARK(bm_thread_pool_affinity_ctor)->Iterations(50);
