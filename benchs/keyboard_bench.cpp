#include <benchmark/benchmark.h>
#include <hj/hardware/keyboard.h>
#include <string>
#include <cstring>
#include <cstdlib>
#include <filesystem>

static void bm_keyboard_enumerate(benchmark::State &state)
{
    keyboard_info_t infos[16];
    for(auto _ : state)
    {
        int n = keyboard_enumerate(infos, 16);
        benchmark::DoNotOptimize(n);
    }
}

static void bm_keyboard_openclose(benchmark::State &state)
{
    // get a device path once (not measured)
    keyboard_info_t infos[8] = {0};
    int             n        = keyboard_enumerate(infos, 8);
    const char     *path     = nullptr;
    if(n > 0 && std::strlen(infos[0].device_path) > 0)
        path = infos[0].device_path;

    for(auto _ : state)
    {
        int handle = keyboard_open(path);
        benchmark::DoNotOptimize(handle);
        keyboard_close(handle);
    }
}

static void bm_keyboard_read_event(benchmark::State &state)
{
    // open a handle (not part of per-iteration timing)
    keyboard_info_t infos[4] = {0};
    int             n        = keyboard_enumerate(infos, 4);
    const char     *path     = nullptr;
    if(n > 0 && std::strlen(infos[0].device_path) > 0)
        path = infos[0].device_path;

    int handle = keyboard_open(path);

    for(auto _ : state)
    {
        key_event_t ev  = {0};
        int         ret = keyboard_read_event(handle, &ev);
        benchmark::DoNotOptimize(ret);
        benchmark::ClobberMemory();
    }

    keyboard_close(handle);
}

static void bm_keyboard_set_repeat(benchmark::State &state)
{
    // This modifies system keyboard settings on some platforms; skip unless explicitly allowed
    const char *allow = std::getenv("HJ_BENCH_ALLOW_HARDWARE");
    if(!allow)
    {
        state.counters["hardware_skipped"] = 1;
        return;
    }

    int handle = keyboard_open(NULL);
    for(auto _ : state)
    {
        int ret = keyboard_set_repeat(handle, 500, 30);
        benchmark::DoNotOptimize(ret);
    }
    keyboard_close(handle);
}

BENCHMARK(bm_keyboard_enumerate)->Iterations(5000);
BENCHMARK(bm_keyboard_openclose)->Iterations(2000);
BENCHMARK(bm_keyboard_read_event)->Iterations(10000);
BENCHMARK(bm_keyboard_set_repeat)->Iterations(100);