#include <benchmark/benchmark.h>
#include <hj/hardware/mouse.h>
#include <cstring>
#include <cstdlib>
#include <filesystem>
#include <chrono>

static void bm_mouse_enumerate(benchmark::State &state)
{
    mouse_info_t infos[16];
    for(auto _ : state)
    {
        int n = mouse_enumerate(infos, 16);
        benchmark::DoNotOptimize(n);
    }
}

static void bm_mouse_openclose(benchmark::State &state)
{
    // pick a device path if available (not timed)
    mouse_info_t infos[8] = {0};
    int          n        = mouse_enumerate(infos, 8);
    const char  *path     = nullptr;
    if(n > 0 && std::strlen(infos[0].device_path) > 0)
        path = infos[0].device_path;

    for(auto _ : state)
    {
        int handle = mouse_open(path);
        benchmark::DoNotOptimize(handle);
        mouse_close(handle);
    }
}

static void bm_mouse_read_event(benchmark::State &state)
{
    mouse_info_t infos[4] = {0};
    int          n        = mouse_enumerate(infos, 4);
    const char  *path     = nullptr;
    if(n > 0 && std::strlen(infos[0].device_path) > 0)
        path = infos[0].device_path;

    int handle = mouse_open(path);

    for(auto _ : state)
    {
        mouse_event_t ev  = {0};
        int           ret = mouse_read_event(handle, &ev);
        benchmark::DoNotOptimize(ret);
        benchmark::ClobberMemory();
    }

    mouse_close(handle);
}

static void bm_mouse_set_param(benchmark::State &state)
{
    // modifying system mouse parameters can be intrusive; require env allow
    const char *allow = std::getenv("HJ_BENCH_ALLOW_HARDWARE");
    if(!allow)
    {
        state.counters["hardware_skipped"] = 1;
        return;
    }

    int handle = mouse_open(NULL);
    for(auto _ : state)
    {
        int ret = mouse_set_param(handle, 10);
        benchmark::DoNotOptimize(ret);
    }
    mouse_close(handle);
}

BENCHMARK(bm_mouse_enumerate)->Iterations(5000);
BENCHMARK(bm_mouse_openclose)->Iterations(2000);
BENCHMARK(bm_mouse_read_event)->Iterations(10000);
BENCHMARK(bm_mouse_set_param)->Iterations(100);