#include <benchmark/benchmark.h>

#include <hj/algo/uuid.hpp>
#include <string>
#include <vector>
#include <thread>

// Generate UUID string (to_string) benchmark
static void bm_uuid_gen_string(benchmark::State &state)
{
    for(auto _ : state)
    {
        auto s = hj::uuid::gen();
        benchmark::DoNotOptimize(s);
        benchmark::ClobberMemory();
    }
}

// Generate 64-bit uuid (default big_endian=true)
static void bm_uuid_genu64_default(benchmark::State &state)
{
    for(auto _ : state)
    {
        auto v = hj::uuid::gen_u64();
        benchmark::DoNotOptimize(v);
        benchmark::ClobberMemory();
    }
}

// Generate 64-bit uuid (explicit big-endian)
static void bm_uuid_genu64_big_endian(benchmark::State &state)
{
    for(auto _ : state)
    {
        auto v = hj::uuid::gen_u64(true);
        benchmark::DoNotOptimize(v);
        benchmark::ClobberMemory();
    }
}

// Generate 64-bit uuid (explicit little-endian)
static void bm_uuid_genu64_little_endian(benchmark::State &state)
{
    for(auto _ : state)
    {
        auto v = hj::uuid::gen_u64(false);
        benchmark::DoNotOptimize(v);
        benchmark::ClobberMemory();
    }
}

// Small parallel generation stress: spawn N threads each generating M uuids
static void bm_uuid_gen_parallel(benchmark::State &state)
{
    const int threads    = 4;
    const int per_thread = 1000;

    for(auto _ : state)
    {
        state.PauseTiming();
        std::vector<std::thread> ths;
        ths.reserve(threads);
        state.ResumeTiming();

        for(int i = 0; i < threads; ++i)
        {
            ths.emplace_back([per_thread]() {
                for(int j = 0; j < per_thread; ++j)
                {
                    auto v = hj::uuid::gen_u64();
                    benchmark::DoNotOptimize(v);
                }
            });
        }

        for(auto &t : ths)
            t.join();

        benchmark::ClobberMemory();
    }
}

BENCHMARK(bm_uuid_gen_string)->Unit(benchmark::kMicrosecond);
BENCHMARK(bm_uuid_genu64_default)->Unit(benchmark::kMicrosecond);
BENCHMARK(bm_uuid_genu64_big_endian)->Unit(benchmark::kMicrosecond);
BENCHMARK(bm_uuid_genu64_little_endian)->Unit(benchmark::kMicrosecond);
BENCHMARK(bm_uuid_gen_parallel)->Unit(benchmark::kMillisecond);
