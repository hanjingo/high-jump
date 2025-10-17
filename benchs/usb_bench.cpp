#include <benchmark/benchmark.h>

#include <hj/hardware/usb.h>
#include <atomic>
#include <cstdlib>

static bool bench_allow_usb()
{
    return std::getenv("HJ_BENCH_ALLOW_USB") != nullptr;
}

static std::atomic<int> bench_usb_range_count{0};

// C-style callback used by usb_device_range. Keep C-linkage compatibility.
static bool bench_usb_range_cb(usb_info_t *info)
{
    (void) info;
    bench_usb_range_count.fetch_add(1, std::memory_order_relaxed);
    return true; // continue iteration
}

// Measure usb_device_count (fast path for enumeration)
static void bm_usb_device_count(benchmark::State &state)
{
    if(!bench_allow_usb())
        state.SkipWithError("HJ_BENCH_ALLOW_USB not set");

    for(auto _ : state)
    {
        state.PauseTiming();
        // nothing heavy to prepare
        state.ResumeTiming();

        int count = usb_device_count(default_usb_device_filter);
        benchmark::DoNotOptimize(count);
        benchmark::ClobberMemory();
    }
}

// Measure usb_device_range enumeration cost
static void bm_use_device_range(benchmark::State &state)
{
    if(!bench_allow_usb())
        state.SkipWithError("HJ_BENCH_ALLOW_USB not set");

    for(auto _ : state)
    {
        state.PauseTiming();
        bench_usb_range_count.store(0, std::memory_order_relaxed);
        state.ResumeTiming();

        usb_device_range(bench_usb_range_cb, default_usb_device_filter);

        int seen = bench_usb_range_count.load(std::memory_order_relaxed);
        benchmark::DoNotOptimize(seen);
        benchmark::ClobberMemory();
    }
}

BENCHMARK(bm_usb_device_count)->Unit(benchmark::kMillisecond);
BENCHMARK(bm_use_device_range)->Unit(benchmark::kMillisecond);
