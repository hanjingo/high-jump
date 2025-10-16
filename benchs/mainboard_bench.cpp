#include <benchmark/benchmark.h>
#include <hj/hardware/mainboard.h>
#include <string>
#include <cstring>

static void bm_mainboard_model(benchmark::State &state)
{
    for(auto _ : state)
    {
        char buf[256] = {0};
        int  ret      = mainboard_model(buf, sizeof(buf));
        benchmark::DoNotOptimize(ret);
        benchmark::DoNotOptimize(buf[0]);
    }
}

static void bm_mainboard_vendor(benchmark::State &state)
{
    for(auto _ : state)
    {
        char buf[256] = {0};
        int  ret      = mainboard_vendor(buf, sizeof(buf));
        benchmark::DoNotOptimize(ret);
        benchmark::DoNotOptimize(buf[0]);
    }
}

static void bm_mainboard_serial(benchmark::State &state)
{
    for(auto _ : state)
    {
        char buf[256] = {0};
        int  ret      = mainboard_serial_num(buf, sizeof(buf));
        benchmark::DoNotOptimize(ret);
        benchmark::DoNotOptimize(buf[0]);
    }
}

static void bm_mainboard_bios_version(benchmark::State &state)
{
    for(auto _ : state)
    {
        char buf[256] = {0};
        int  ret      = mainboard_bios_version(buf, sizeof(buf));
        benchmark::DoNotOptimize(ret);
        benchmark::DoNotOptimize(buf[0]);
    }
}

static void bm_mainboard_chipset(benchmark::State &state)
{
    for(auto _ : state)
    {
        char buf[256] = {0};
        int  ret      = mainboard_chipset(buf, sizeof(buf));
        benchmark::DoNotOptimize(ret);
        benchmark::DoNotOptimize(buf[0]);
    }
}

static void bm_mainboard_memory_slots(benchmark::State &state)
{
    for(auto _ : state)
    {
        int ret = mainboard_memory_slots();
        benchmark::DoNotOptimize(ret);
    }
}

static void bm_mainboard_expansion_slots(benchmark::State &state)
{
    for(auto _ : state)
    {
        int ret = mainboard_expansion_slots();
        benchmark::DoNotOptimize(ret);
    }
}

static void bm_mainboard_manufacturer_product(benchmark::State &state)
{
    for(auto _ : state)
    {
        char manuf[256] = {0};
        char prod[256]  = {0};
        int  ret1       = mainboard_manufacturer_name(manuf, sizeof(manuf));
        int  ret2       = mainboard_product_name(prod, sizeof(prod));
        benchmark::DoNotOptimize(ret1);
        benchmark::DoNotOptimize(ret2);
        benchmark::DoNotOptimize(manuf[0]);
        benchmark::DoNotOptimize(prod[0]);
    }
}

static void bm_mainboard_version(benchmark::State &state)
{
    for(auto _ : state)
    {
        uint8_t major = 0, minor = 0, patch = 0;
        int     ret = mainboard_version(&major, &minor, &patch);
        benchmark::DoNotOptimize(ret);
        benchmark::DoNotOptimize(major);
        benchmark::DoNotOptimize(minor);
        benchmark::DoNotOptimize(patch);
    }
}

// register benchmarks with conservative iteration counts for CI
BENCHMARK(bm_mainboard_model)->Iterations(2000);
BENCHMARK(bm_mainboard_vendor)->Iterations(2000);
BENCHMARK(bm_mainboard_serial)->Iterations(2000);
BENCHMARK(bm_mainboard_bios_version)->Iterations(2000);
BENCHMARK(bm_mainboard_chipset)->Iterations(2000);
BENCHMARK(bm_mainboard_memory_slots)->Iterations(5000);
BENCHMARK(bm_mainboard_expansion_slots)->Iterations(5000);
BENCHMARK(bm_mainboard_manufacturer_product)->Iterations(2000);
BENCHMARK(bm_mainboard_version)->Iterations(2000);