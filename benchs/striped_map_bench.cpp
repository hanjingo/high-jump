#include <benchmark/benchmark.h>
#include <hj/sync/striped_map.hpp>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <cstdlib>

using namespace hj;

// Construct with capacity
static void bm_striped_map_construct(benchmark::State &state)
{
    const std::size_t capa = 16;
    for(auto _ : state)
    {
        striped_map<int, int> m(capa);
        benchmark::DoNotOptimize(m);
    }
}
BENCHMARK(bm_striped_map_construct)->Iterations(10000);

// Emplace N elements
static void bm_striped_map_emplace(benchmark::State &state)
{
    const int n = static_cast<int>(state.range(0));
    for(auto _ : state)
    {
        state.PauseTiming();
        striped_map<int, int> m(32);
        state.ResumeTiming();

        for(int i = 0; i < n; ++i)
        {
            m.emplace(i, i);
            benchmark::DoNotOptimize(m);
        }

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_striped_map_emplace)->Arg(100)->Arg(1000);

// Find existing keys
static void bm_striped_map_find(benchmark::State &state)
{
    const int n = static_cast<int>(state.range(0));
    for(auto _ : state)
    {
        state.PauseTiming();
        striped_map<int, int> m(64);
        for(int i = 0; i < n; ++i)
            m.emplace(i, i);
        state.ResumeTiming();

        for(int i = 0; i < n; ++i)
        {
            int  v  = 0;
            bool ok = m.find(i, v);
            benchmark::DoNotOptimize(ok);
            benchmark::DoNotOptimize(v);
        }

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_striped_map_find)->Arg(100)->Arg(1000);

// Replace values
static void bm_striped_map_replace(benchmark::State &state)
{
    const int n = static_cast<int>(state.range(0));
    for(auto _ : state)
    {
        state.PauseTiming();
        striped_map<int, int> m(64);
        for(int i = 0; i < n; ++i)
            m.emplace(i, i);
        state.ResumeTiming();

        for(int i = 0; i < n; ++i)
            m.replace(i, i + 1);

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_striped_map_replace)->Arg(100)->Arg(1000);

// Erase keys
static void bm_striped_map_erase(benchmark::State &state)
{
    const int n = static_cast<int>(state.range(0));
    for(auto _ : state)
    {
        state.PauseTiming();
        striped_map<int, int> m(64);
        for(int i = 0; i < n; ++i)
            m.emplace(i, i);
        state.ResumeTiming();

        for(int i = 0; i < n; ++i)
            m.erase(i);

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_striped_map_erase)->Arg(100)->Arg(1000);

// Iterator traversal
static void bm_striped_map_iterator_traversal(benchmark::State &state)
{
    const int n = 1000;
    for(auto _ : state)
    {
        state.PauseTiming();
        striped_map<int, int> m(64);
        for(int i = 0; i < n; ++i)
            m.emplace(i, i);
        state.ResumeTiming();

        std::size_t count = 0;
        for(auto it = m.begin(); it != m.end(); ++it)
            ++count;
        benchmark::DoNotOptimize(count);

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_striped_map_iterator_traversal)->Iterations(1000);

// size() hot path
static void bm_striped_map_size(benchmark::State &state)
{
    state.PauseTiming();
    striped_map<int, int> m(64);
    for(int i = 0; i < 500; ++i)
        m.emplace(i, i);
    state.ResumeTiming();

    for(auto _ : state)
    {
        auto s = m.size();
        benchmark::DoNotOptimize(s);
    }
}
BENCHMARK(bm_striped_map_size)->Iterations(10000);

// Guarded multithreaded read/write benchmark. Disabled by default in CI unless
// environment variable HJ_BENCH_ALLOW_MT=1 is set.
static void bm_striped_map_multithreaded_rw(benchmark::State &state)
{
    if(std::getenv("HJ_BENCH_ALLOW_MT") == nullptr)
    {
        state.SkipWithError("multithreaded benchmark disabled; set "
                            "HJ_BENCH_ALLOW_MT=1 to enable");
        return;
    }

    const int thread_count   = 2;
    const int ops_per_thread = 10000;

    for(auto _ : state)
    {
        state.PauseTiming();
        striped_map<int64_t, int64_t> m(64);
        std::atomic<int64_t>          write_sum{0};
        std::atomic<int64_t>          read_sum{0};
        std::vector<std::thread>      writers;
        std::vector<std::thread>      readers;
        state.ResumeTiming();

        for(int t = 0; t < thread_count; ++t)
        {
            writers.emplace_back([&, t]() {
                for(int i = 0; i < ops_per_thread; ++i)
                {
                    int64_t key = static_cast<int64_t>(t) * ops_per_thread + i;
                    m.emplace(key, key);
                    write_sum += key;
                }
            });
        }

        for(auto &th : writers)
            th.join();

        for(int t = 0; t < thread_count; ++t)
        {
            readers.emplace_back([&, t]() {
                int64_t local = 0;
                for(int i = 0; i < ops_per_thread; ++i)
                {
                    int64_t key = static_cast<int64_t>(t) * ops_per_thread + i;
                    int64_t v   = 0;
                    if(m.find(key, v))
                        local += v;
                }
                read_sum += local;
            });
        }

        for(auto &th : readers)
            th.join();

        benchmark::DoNotOptimize(write_sum.load());
        benchmark::DoNotOptimize(read_sum.load());
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_striped_map_multithreaded_rw)->Iterations(10);
