#include <benchmark/benchmark.h>
#include <hj/io/dbuffer.hpp>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>

using namespace std::chrono_literals;

static void bm_dbuffer_write_read_single_thread(benchmark::State &st)
{
    hj::dbuffer<std::vector<int>> dbuf{};
    std::vector<int>              src{1, 2, 3, 4, 5};
    std::vector<int>              dst(5);

    for(auto _ : st)
    {
        bool ok = dbuf.write(src);
        benchmark::DoNotOptimize(ok);
        ok = dbuf.read(dst);
        benchmark::DoNotOptimize(dst);
    }
}
BENCHMARK(bm_dbuffer_write_read_single_thread)->Iterations(20000);

static void bm_dbuffer_move_write(benchmark::State &st)
{
    hj::dbuffer<std::string> dbuf;
    for(auto _ : st)
    {
        std::string s  = "hello world";
        bool        ok = dbuf.write(std::move(s));
        benchmark::DoNotOptimize(ok);
        std::string out;
        ok = dbuf.read(out);
        benchmark::DoNotOptimize(out);
    }
}
BENCHMARK(bm_dbuffer_move_write)->Iterations(20000);

static void bm_dbuffer_custom_copy(benchmark::State &st)
{
    using V = std::vector<int>;
    hj::dbuffer<V> dbuf([](V &src, V &dst) -> bool {
        if(src.empty())
            return false;
        dst = src; // simple copy
        return true;
    });

    V v{1, 2, 3, 4};
    V out;
    for(auto _ : st)
    {
        bool ok = dbuf.write(v);
        benchmark::DoNotOptimize(ok);
        ok = dbuf.read(out);
        benchmark::DoNotOptimize(out);
    }
}
BENCHMARK(bm_dbuffer_custom_copy)->Iterations(15000);

static void bm_dbuffer_concurrent_rw(benchmark::State &st)
{
    hj::dbuffer<std::vector<int>> dbuf{};
    const int                     writer_count   = 2;
    const int                     reader_count   = 2;
    const int                     ops_per_thread = 1000;

    for(auto _ : st)
    {
        std::atomic<int>         written{0};
        std::atomic<int>         readed{0};
        std::vector<std::thread> writers;
        std::vector<std::thread> readers;

        for(int i = 0; i < writer_count; ++i)
        {
            writers.emplace_back([&]() {
                std::vector<int> v{1, 2, 3, 4, 5};
                for(int n = 0; n < ops_per_thread; ++n)
                {
                    if(dbuf.write(v))
                        ++written;
                }
            });
        }

        for(int i = 0; i < reader_count; ++i)
        {
            readers.emplace_back([&]() {
                std::vector<int> out(5);
                for(int n = 0; n < ops_per_thread; ++n)
                {
                    if(dbuf.read(out))
                        ++readed;
                }
            });
        }

        for(auto &t : writers)
            t.join();
        for(auto &t : readers)
            t.join();

        benchmark::DoNotOptimize(written);
        benchmark::DoNotOptimize(readed);
    }
}
BENCHMARK(bm_dbuffer_concurrent_rw)->Iterations(200);
