#include <benchmark/benchmark.h>
#include <hj/time/timer.hpp>
#include <atomic>
#include <thread>
#include <chrono>
#include <memory>

using namespace hj;

// Construct/destruct cost
static void bm_timer_construct(benchmark::State &state)
{
    for(auto _ : state)
    {
        timer t(1000); // 1000us
        benchmark::DoNotOptimize(t);
    }
}
BENCHMARK(bm_timer_construct)->Iterations(10000);

// Start and cancel quickly: measure overhead of scheduling + cancel
static void bm_timer_start_cancel(benchmark::State &state)
{
    for(auto _ : state)
    {
        auto             t = std::make_shared<timer>(1000);
        std::atomic<int> flag{0};
        t->start([&flag] { flag.store(1, std::memory_order_relaxed); });
        t->cancel();
        benchmark::DoNotOptimize(t);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_timer_start_cancel)->Iterations(10000);

// make() + wait for callback to execute (short wait outside timing)
static void bm_timer_make_and_wait(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        std::atomic<int> flag{0};
        state.ResumeTiming();

        auto tm = timer::make(500, [&flag] {
            flag.store(1, std::memory_order_relaxed);
        });

        // give timer time to fire (outside timing)
        state.PauseTiming();
        auto start = std::chrono::steady_clock::now();
        while(flag.load(std::memory_order_relaxed) == 0
              && std::chrono::steady_clock::now() - start
                     < std::chrono::milliseconds(50))
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        state.ResumeTiming();

        benchmark::DoNotOptimize(tm);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_timer_make_and_wait)->Iterations(50);

// reset existing timer and wait for the new callback
static void bm_timer_reset_and_wait(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        std::atomic<int> flag{0};
        auto             tm = std::make_shared<timer>(5000); // 5ms
        tm->start([] {});                                    // initial no-op
        state.ResumeTiming();

        // reset to a short timeout with a callback
        tm->reset(500, [&flag] { flag.store(1, std::memory_order_relaxed); });

        // wait outside timing for callback
        state.PauseTiming();
        auto start = std::chrono::steady_clock::now();
        while(flag.load(std::memory_order_relaxed) == 0
              && std::chrono::steady_clock::now() - start
                     < std::chrono::milliseconds(50))
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        state.ResumeTiming();

        benchmark::DoNotOptimize(tm);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_timer_reset_and_wait)->Iterations(50);

// shared_ptr copy/move overhead for timer
static void bm_timer_shared_ptr_copy_move(benchmark::State &state)
{
    for(auto _ : state)
    {
        auto tm1 = std::make_shared<timer>(1000);
        state.PauseTiming();
        state.ResumeTiming();
        auto tm2 = tm1;            // copy
        auto tm3 = std::move(tm2); // move
        benchmark::DoNotOptimize(tm3);
    }
}
BENCHMARK(bm_timer_shared_ptr_copy_move)->Iterations(10000);
