#include <benchmark/benchmark.h>
#include <hj/db/db_conn_pool.hpp>
#include <thread>
#include <vector>
#include <memory>
#include <atomic>
#include <chrono>

using namespace std::chrono_literals;

// Use a trivial connection type (int) for benching to avoid sqlite dependency
using conn_t = int;

static std::shared_ptr<conn_t> make_int_conn()
{
    return std::shared_ptr<conn_t>(new conn_t(42));
}

static void bm_create_pool(benchmark::State &st)
{
    for(auto _ : st)
    {
        hj::db_conn_pool<conn_t> pool(100, []() { return make_int_conn(); });
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_create_pool)->Iterations(1000);

static void bm_acquire_release_single_thread(benchmark::State &st)
{
    hj::db_conn_pool<conn_t> pool(10, []() { return make_int_conn(); });
    for(auto _ : st)
    {
        auto c = pool.acquire();
        benchmark::DoNotOptimize(c);
        // release by resetting
        c.reset();
    }
}
BENCHMARK(bm_acquire_release_single_thread)->Iterations(20000);

static void bm_acquire_timeout(benchmark::State &st)
{
    hj::db_conn_pool<conn_t> pool(1, []() { return make_int_conn(); });
    for(auto _ : st)
    {
        auto c1 = pool.acquire();
        auto c2 = pool.acquire(1); // should timeout quickly
        benchmark::DoNotOptimize(c1);
        benchmark::DoNotOptimize(c2);
        c1.reset();
    }
}
BENCHMARK(bm_acquire_timeout)->Iterations(2000);

static void bm_concurrent_acquire_release(benchmark::State &st)
{
    const std::size_t        pool_size     = 5;
    const int                threads_count = 20;
    hj::db_conn_pool<conn_t> pool(pool_size, []() { return make_int_conn(); });

    for(auto _ : st)
    {
        std::atomic<int>         success{0};
        std::vector<std::thread> threads;
        threads.reserve(threads_count);
        for(int i = 0; i < threads_count; ++i)
        {
            threads.emplace_back([&]() {
                auto c = pool.acquire(100);
                if(c)
                {
                    // simulate short work
                    std::this_thread::sleep_for(1ms);
                    ++success;
                }
            });
        }

        for(auto &t : threads)
            t.join();

        benchmark::DoNotOptimize(success);
    }
}
BENCHMARK(bm_concurrent_acquire_release)->Iterations(200);
