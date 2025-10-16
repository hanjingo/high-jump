#include <benchmark/benchmark.h>
#include <hj/testing/error.hpp>
#include <string>
#include <thread>
#include <vector>

enum class err1
{
    ok = -1,
    fail,
    unknown,
};

static void bm_ec_to_int(benchmark::State &st)
{
    for(auto _ : st)
    {
        volatile int a = hj::ec_to_int(err1::ok);
        volatile int b = hj::ec_to_int(err1::fail);
        volatile int c = hj::ec_to_int(err1::unknown);
        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(bm_ec_to_int)->Iterations(200000);

static void bm_ec_to_hex(benchmark::State &st)
{
    for(auto _ : st)
    {
        auto s1 = hj::ec_to_hex<int>(-1);
        auto s2 = hj::ec_to_hex<unsigned int>(0xFFFF);
        benchmark::DoNotOptimize(s1);
        benchmark::DoNotOptimize(s2);
    }
}
BENCHMARK(bm_ec_to_hex)->Iterations(100000);

static void bm_register_and_make_err(benchmark::State &st)
{
    for(auto _ : st)
    {
        hj::register_err("bench_cat", 1001, "bench error");
        auto ec = hj::make_err(1001, "bench_cat");
        benchmark::DoNotOptimize(ec);
    }
}
BENCHMARK(bm_register_and_make_err)->Iterations(50000);

static void bm_nested_make_err(benchmark::State &st)
{
    hj::register_err("net", 1, "network");
    hj::register_err("io", 2, "io");
    for(auto _ : st)
    {
        auto e1     = hj::make_err(1, "net");
        auto e2     = hj::make_err(2, "io");
        auto nested = hj::make_err(e1, e2, "bench_cat");
        benchmark::DoNotOptimize(nested.ec);
    }
}
BENCHMARK(bm_nested_make_err)->Iterations(20000);

static void bm_concurrent_register(benchmark::State &st)
{
    for(auto _ : st)
    {
        const int                N = 8;
        std::vector<std::thread> threads;
        threads.reserve(N);
        for(int i = 0; i < N; ++i)
        {
            threads.emplace_back([i]() {
                std::string cat = "bench_cat_" + std::to_string(i);
                hj::register_err(cat.c_str(), i, "desc");
                auto ec = hj::make_err(i, cat.c_str());
                benchmark::DoNotOptimize(ec);
            });
        }
        for(auto &t : threads)
            t.join();
    }
}
BENCHMARK(bm_concurrent_register)->Iterations(2000);