#include <benchmark/benchmark.h>
#include <hj/sync/object_pool.hpp>
#include <vector>
#include <thread>
#include <atomic>
#include <string>
#include <chrono>

// lightweight worker used in tests
struct Worker
{
    Worker() = default;
    Worker(const std::string &n, int a, float s, const std::string &e)
        : name(n)
        , age(a)
        , salary(s)
        , email(e)
    {
    }
    std::string name;
    int         age;
    float       salary;
    std::string email;
};

static void bm_object_pool_create(benchmark::State &state)
{
    const int N = 500;
    for(auto _ : state)
    {
        hj::object_pool<Worker> pool;
        for(int i = 0; i < N; ++i)
            pool.create(std::string("u") + std::to_string(i),
                        20 + i,
                        1000.0f + i,
                        "x@x.com");
        benchmark::DoNotOptimize(pool.size());
    }
}

static void bm_object_pool_acquire_release(benchmark::State &state)
{
    const int N = 1000;
    for(auto _ : state)
    {
        hj::object_pool<Worker> pool;
        for(int i = 0; i < N; ++i)
            pool.create(std::string("a") + std::to_string(i), i, 0.0f, "a@b");

        // acquire all then release
        std::vector<Worker *> objs;
        objs.reserve(N);
        for(int i = 0; i < N; ++i)
        {
            auto p = pool.acquire();
            objs.push_back(p);
            benchmark::DoNotOptimize(p);
        }

        for(auto p : objs)
            pool.release(p);

        benchmark::DoNotOptimize(pool.size());
    }
}

static void bm_object_pool_bulk_acquire_release(benchmark::State &state)
{
    const int total = 500;
    const int bulk  = 50;
    for(auto _ : state)
    {
        hj::object_pool<Worker> pool;
        for(int i = 0; i < total; ++i)
            pool.create(std::string("b") + std::to_string(i), i, 0.0f, "b@b");

        std::vector<Worker *> out;
        pool.acquire_bulk(out, bulk);
        benchmark::DoNotOptimize(out.size());
        pool.release_bulk(out);
        benchmark::DoNotOptimize(pool.size());
    }
}

static void bm_object_pool_multithread_acquire_release(benchmark::State &state)
{
    const int total   = 256;
    const int threads = 4;

    for(auto _ : state)
    {
        hj::object_pool<Worker> pool;
        for(int i = 0; i < total; ++i)
            pool.create(std::string("mt") + std::to_string(i), i, 0.0f, "mt@x");

        std::vector<std::thread>           ths;
        std::atomic<int>                   idx{0};
        std::vector<std::vector<Worker *>> results(threads);
        for(int t = 0; t < threads; ++t)
        {
            ths.emplace_back([&pool, &results, t, &idx, total]() {
                while(true)
                {
                    int i = idx++;
                    if(i >= total)
                        break;
                    Worker *p = pool.acquire();
                    if(p)
                        results[t].push_back(p);
                }
            });
        }

        for(auto &th : ths)
            th.join();

        // release
        for(auto &vec : results)
            pool.release_bulk(vec);

        benchmark::DoNotOptimize(pool.size());
    }
}

static void bm_object_pool_size(benchmark::State &state)
{
    for(auto _ : state)
    {
        hj::object_pool<Worker> pool;
        benchmark::DoNotOptimize(pool.size());
        pool.create("s", 1, 1.0f, "e");
        benchmark::DoNotOptimize(pool.size());
    }
}

BENCHMARK(bm_object_pool_create)->Iterations(200);
BENCHMARK(bm_object_pool_acquire_release)->Iterations(200);
BENCHMARK(bm_object_pool_bulk_acquire_release)->Iterations(500);
BENCHMARK(bm_object_pool_multithread_acquire_release)->Iterations(50);
BENCHMARK(bm_object_pool_size)->Iterations(10000);