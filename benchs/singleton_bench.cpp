#include <benchmark/benchmark.h>
#include <hj/types/singleton.hpp>
#include <thread>
#include <vector>

// Benchmarks for hj::singleton (boost::serialization::singleton wrapper)

class A
{
  public:
    static void static_inc() { /* noop for bench */ }
    void        inc() { _tmp += 2; }
    int         value() const { return _tmp; }

  private:
    int _tmp = 0;
};

static void bm_singleton_get_const(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        // no setup
        state.ResumeTiming();

        int v = hj::singleton<A>::get_const_instance().value();
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(bm_singleton_get_const)->Iterations(100000);

static void bm_singleton_get_mutable(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        state.ResumeTiming();

        // call mutable accessor but do not modify singleton state to keep bench idempotent
        int v = hj::singleton<A>::get_mutable_instance().value();
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(bm_singleton_get_mutable)->Iterations(100000);

static void bm_singleton_concurrent_read(benchmark::State &state)
{
    const int threads    = static_cast<int>(state.range(0));
    const int per_thread = static_cast<int>(state.range(1));

    for(auto _ : state)
    {
        state.PauseTiming();
        std::vector<std::thread> ths;
        ths.reserve(threads);
        state.ResumeTiming();

        for(int t = 0; t < threads; ++t)
        {
            ths.emplace_back([per_thread]() {
                for(int i = 0; i < per_thread; ++i)
                {
                    int v = hj::singleton<A>::get_const_instance().value();
                    benchmark::DoNotOptimize(v);
                }
            });
        }

        for(auto &th : ths)
            th.join();

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_singleton_concurrent_read)->Args({2, 1000})->Args({4, 500});
