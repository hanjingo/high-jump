#include <benchmark/benchmark.h>
#include <hj/algo/skiplist.hpp>
#include <vector>
#include <thread>
#include <random>

using hj::skiplist;

static std::vector<std::pair<double, int>> make_test_data(size_t   n,
                                                          uint32_t seed = 42)
{
    std::mt19937                           gen(seed);
    std::uniform_real_distribution<double> dist(0.0, 100000.0);
    std::uniform_int_distribution<int>     val(0, 1000000);
    std::vector<std::pair<double, int>>    out;
    out.reserve(n);
    for(size_t i = 0; i < n; ++i)
        out.emplace_back(dist(gen), val(gen));
    return out;
}

static void bm_skiplist_insert(benchmark::State &state)
{
    const size_t N    = static_cast<size_t>(state.range(0));
    auto         data = make_test_data(N);
    for(auto _ : state)
    {
        state.PauseTiming();
        skiplist<int> sl;
        state.ResumeTiming();

        for(size_t i = 0; i < N; ++i)
            sl.insert(data[i].first, data[i].second);

        benchmark::DoNotOptimize(sl.size());
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_skiplist_insert)->Arg(100)->Arg(1000);

static void bm_skiplist_get_by_rank(benchmark::State &state)
{
    const size_t N    = static_cast<size_t>(state.range(0));
    auto         data = make_test_data(N);
    for(auto _ : state)
    {
        state.PauseTiming();
        skiplist<int> sl;
        for(size_t i = 0; i < N; ++i)
            sl.insert(data[i].first, data[i].second);
        state.ResumeTiming();

        for(size_t i = 0; i < std::min<size_t>(1000, N); ++i)
        {
            auto *node = sl.get_element_by_rank(i % sl.size());
            benchmark::DoNotOptimize(node);
        }

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_skiplist_get_by_rank)->Arg(100)->Arg(1000);

static void bm_skiplist_get_rank(benchmark::State &state)
{
    const size_t N    = static_cast<size_t>(state.range(0));
    auto         data = make_test_data(N);
    for(auto _ : state)
    {
        state.PauseTiming();
        skiplist<int> sl;
        for(size_t i = 0; i < N; ++i)
            sl.insert(data[i].first, data[i].second);
        state.ResumeTiming();

        for(int i = 0; i < 1000 && i < (int) N; ++i)
        {
            auto r = sl.get_rank(data[i].first, data[i].second);
            benchmark::DoNotOptimize(r);
        }

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_skiplist_get_rank)->Arg(100)->Arg(1000);

static void bm_skiplist_range_by_score(benchmark::State &state)
{
    const size_t N    = static_cast<size_t>(state.range(0));
    auto         data = make_test_data(N);
    for(auto _ : state)
    {
        state.PauseTiming();
        skiplist<int> sl;
        for(size_t i = 0; i < N; ++i)
            sl.insert(data[i].first, data[i].second);
        state.ResumeTiming();

        double min_s = data[0].first;
        double max_s = data[std::min<size_t>(N - 1, 10)].first;
        auto   res   = hj::range_by_score(sl, min_s, max_s);
        benchmark::DoNotOptimize(res.nodes.size());

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_skiplist_range_by_score)->Arg(100)->Arg(1000);

static void bm_skiplist_delete_range_by_score(benchmark::State &state)
{
    const size_t N    = static_cast<size_t>(state.range(0));
    auto         data = make_test_data(N);
    for(auto _ : state)
    {
        state.PauseTiming();
        skiplist<int> sl;
        for(size_t i = 0; i < N; ++i)
            sl.insert(data[i].first, data[i].second);
        state.ResumeTiming();

        double min_s   = data[0].first;
        double max_s   = data[std::min<size_t>(N - 1, 10)].first;
        auto   removed = sl.delete_range_by_score(min_s, max_s);
        benchmark::DoNotOptimize(removed);

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_skiplist_delete_range_by_score)->Arg(100)->Arg(1000);

static void bm_skiplist_validate(benchmark::State &state)
{
    const size_t N    = static_cast<size_t>(state.range(0));
    auto         data = make_test_data(N);
    for(auto _ : state)
    {
        state.PauseTiming();
        skiplist<int> sl;
        for(size_t i = 0; i < N; ++i)
            sl.insert(data[i].first, data[i].second);
        state.ResumeTiming();

        bool ok = sl.validate();
        benchmark::DoNotOptimize(ok);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_skiplist_validate)->Arg(100)->Arg(1000);

static void bm_skiplist_parallel_insert(benchmark::State &state)
{
    const int threads    = static_cast<int>(state.range(0));
    const int per_thread = static_cast<int>(state.range(1));

    for(auto _ : state)
    {
        state.PauseTiming();
        skiplist<int> sl;
        state.ResumeTiming();

        std::vector<std::thread> ths;
        ths.reserve(threads);
        for(int t = 0; t < threads; ++t)
        {
            ths.emplace_back([&sl, t, per_thread]() {
                std::mt19937                           gen(t);
                std::uniform_real_distribution<double> dist(0.0, 1000000.0);
                for(int i = 0; i < per_thread; ++i)
                    sl.insert(dist(gen), i + t * per_thread);
            });
        }

        for(auto &th : ths)
            th.join();

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_skiplist_parallel_insert)->Args({2, 100})->Args({4, 50});
