#include <benchmark/benchmark.h>
#include <optional>
#include <utility>
#include <hj/sync/safe_map.hpp>
#include <string>
#include <vector>
#include <thread>

// CI-friendly microbench for hj::safe_map

static void bm_safe_map_insert(benchmark::State &state)
{
    const int N = static_cast<int>(state.range(0));
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::safe_map<int, std::string> map;
        state.ResumeTiming();

        for(int i = 0; i < N; ++i)
        {
            map.insert(i, std::to_string(i));
            benchmark::DoNotOptimize(map);
        }

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_safe_map_insert)->Arg(100)->Arg(1000);

static void bm_safe_map_find(benchmark::State &state)
{
    const int N = static_cast<int>(state.range(0));
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::safe_map<int, std::string> map;
        for(int i = 0; i < N; ++i)
            map.insert(i, std::to_string(i));
        state.ResumeTiming();

        for(int i = 0; i < N; ++i)
        {
            auto opt = map.find(i);
            if(opt)
                benchmark::DoNotOptimize(opt.value());
            else
                benchmark::DoNotOptimize(0);
        }

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_safe_map_find)->Arg(100)->Arg(1000);

static void bm_safe_map_replace(benchmark::State &state)
{
    const int N = static_cast<int>(state.range(0));
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::safe_map<int, std::string> map;
        for(int i = 0; i < N; ++i)
            map.insert(i, std::to_string(i));
        state.ResumeTiming();

        for(int i = 0; i < N; ++i)
        {
            map.replace(i, std::to_string(i + 1));
            benchmark::DoNotOptimize(map);
        }

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_safe_map_replace)->Arg(100)->Arg(1000);

static void bm_safe_map_erase(benchmark::State &state)
{
    const int N = static_cast<int>(state.range(0));
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::safe_map<int, std::string> map;
        for(int i = 0; i < N; ++i)
            map.insert(i, std::to_string(i));
        state.ResumeTiming();

        for(int i = 0; i < N; ++i)
        {
            map.erase(std::move(i));
        }

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_safe_map_erase)->Arg(100)->Arg(1000);

static void bm_safe_map_range(benchmark::State &state)
{
    const int N = static_cast<int>(state.range(0));
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::safe_map<int, int> map;
        for(int i = 0; i < N; ++i)
            map.insert(i, i);
        state.ResumeTiming();

        int sum = 0;
        map.range([&sum](const int &k, int &v) {
            (void) k;
            sum += v;
            return true;
        });

        benchmark::DoNotOptimize(sum);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_safe_map_range)->Arg(100)->Arg(1000);

// Multithreaded insert: spawn threads per iteration (simple model, measures concurrent insert throughput)
static void bm_safe_map_paralle_insert(benchmark::State &state)
{
    const int threads    = static_cast<int>(state.range(0));
    const int per_thread = static_cast<int>(state.range(1));

    for(auto _ : state)
    {
        state.PauseTiming();
        hj::safe_map<int, int> map;
        state.ResumeTiming();

        std::vector<std::thread> ths;
        ths.reserve(threads);
        for(int t = 0; t < threads; ++t)
        {
            ths.emplace_back([&map, t, per_thread]() {
                int base = t * per_thread;
                for(int i = 0; i < per_thread; ++i)
                    map.insert(base + i, base + i);
            });
        }

        for(auto &th : ths)
            th.join();

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_safe_map_paralle_insert)->Args({2, 500})->Args({4, 250});
