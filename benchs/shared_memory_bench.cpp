#include <benchmark/benchmark.h>
#include <hj/sync/shared_memory.hpp>
#include <chrono>
#include <sstream>
#include <thread>
#include <vector>

static std::string make_shm_name(const char *base = "hj_shm")
{
    auto now =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    auto tid = std::hash<std::thread::id>()(std::this_thread::get_id());
    std::ostringstream oss;
    oss << base << "_" << now << "_" << tid;
    return oss.str();
}

// Map/unmap single mapping per iteration
static void bm_shared_memory_map_unmap(benchmark::State &state)
{
    const std::size_t sz = static_cast<std::size_t>(state.range(0));
    for(auto _ : state)
    {
        state.PauseTiming();
        auto name = make_shm_name("bench_mapunmap").c_str();
        // ensure clean
        hj::shared_memory::remove(name);
        hj::shared_memory shm(name, sz);
        state.ResumeTiming();

        void *ptr = shm.map();
        benchmark::DoNotOptimize(ptr);
        shm.unmap();

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_shared_memory_map_unmap)->Arg(4096)->Arg(65536);

// Write/read a few bytes in the mapped region
static void bm_shared_memory_write_read(benchmark::State &state)
{
    const std::size_t sz = static_cast<std::size_t>(state.range(0));
    for(auto _ : state)
    {
        state.PauseTiming();
        auto name = make_shm_name("bench_writeread").c_str();
        hj::shared_memory::remove(name);
        hj::shared_memory shm(name, sz);
        void             *ptr = shm.map();
        state.ResumeTiming();

        char *c = static_cast<char *>(ptr);
        // write a pattern every 64 bytes
        for(std::size_t i = 0; i < sz; i += 64)
            c[i] = static_cast<char>(i & 0xFF);

        // read back a simple checksum
        volatile unsigned long sum = 0;
        for(std::size_t i = 0; i < sz; i += 64)
            sum += static_cast<unsigned char>(c[i]);

        benchmark::DoNotOptimize(sum);

        shm.unmap();
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_shared_memory_write_read)->Arg(4096)->Arg(65536);

// Concurrent map/unmap from multiple threads
static void bm_shared_memory_concurrent_map_unmap(benchmark::State &state)
{
    const int threads    = static_cast<int>(state.range(0));
    const int per_thread = static_cast<int>(state.range(1));

    for(auto _ : state)
    {
        state.PauseTiming();
        auto name = make_shm_name("bench_concurrent").c_str();
        hj::shared_memory::remove(name);
        hj::shared_memory shm(name, 4096);
        state.ResumeTiming();

        std::vector<std::thread> ths;
        ths.reserve(threads);
        for(int t = 0; t < threads; ++t)
        {
            ths.emplace_back([&shm, per_thread]() {
                for(int i = 0; i < per_thread; ++i)
                {
                    void *p = shm.map();
                    if(p)
                        shm.unmap();
                }
            });
        }

        for(auto &th : ths)
            th.join();

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_shared_memory_concurrent_map_unmap)->Args({4, 50})->Args({8, 25});
