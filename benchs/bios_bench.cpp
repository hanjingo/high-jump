#include <benchmark/benchmark.h>
#include <hj/hardware/bios.h>
#include <cstring>

static void bm_bios_vendor(benchmark::State &state)
{
    char buf[128] = {0};
    for(auto _ : state)
    {
        size_t       len = sizeof(buf);
        bios_error_t ret = bios_vendor(buf, &len);
        if(ret == BIOS_ERROR_NOT_SUPPORTED || ret == BIOS_ERROR_NOT_FOUND)
        {
            state.SkipWithError("bios_vendor not supported on this platform");
            return;
        }
        benchmark::DoNotOptimize(len);
    }
}
BENCHMARK(bm_bios_vendor);

static void bm_bios_version(benchmark::State &state)
{
    char buf[64] = {0};
    for(auto _ : state)
    {
        size_t       len = sizeof(buf);
        bios_error_t ret = bios_version(buf, &len);
        if(ret == BIOS_ERROR_NOT_SUPPORTED || ret == BIOS_ERROR_NOT_FOUND)
        {
            state.SkipWithError("bios_version not supported on this platform");
            return;
        }
        benchmark::DoNotOptimize(len);
    }
}
BENCHMARK(bm_bios_version);

static void bm_bios_release_date(benchmark::State &state)
{
    char buf[32] = {0};
    for(auto _ : state)
    {
        size_t       len = sizeof(buf);
        bios_error_t ret = bios_release_date(buf, &len);
        if(ret == BIOS_ERROR_NOT_SUPPORTED || ret == BIOS_ERROR_NOT_FOUND)
        {
            state.SkipWithError(
                "bios_release_date not supported on this platform");
            return;
        }
        benchmark::DoNotOptimize(len);
    }
}
BENCHMARK(bm_bios_release_date);

static void bm_bios_serial_num(benchmark::State &state)
{
    char buf[128] = {0};
    for(auto _ : state)
    {
        size_t       len = sizeof(buf);
        bios_error_t ret = bios_serial_num(buf, &len);
        if(ret == BIOS_ERROR_NOT_SUPPORTED || ret == BIOS_ERROR_NOT_FOUND)
        {
            state.SkipWithError(
                "bios_serial_num not supported on this platform");
            return;
        }
        benchmark::DoNotOptimize(len);
    }
}
BENCHMARK(bm_bios_serial_num);

static void bm_bios_starting_segment(benchmark::State &state)
{
    uint16_t seg = 0;
    for(auto _ : state)
    {
        bios_error_t ret = bios_starting_segment(&seg);
        if(ret == BIOS_ERROR_NOT_SUPPORTED || ret == BIOS_ERROR_NOT_FOUND)
        {
            state.SkipWithError(
                "bios_starting_segment not supported on this platform");
            return;
        }
        benchmark::DoNotOptimize(seg);
    }
}
BENCHMARK(bm_bios_starting_segment);

static void bm_bios_rom_size(benchmark::State &state)
{
    size_t sz = 0;
    for(auto _ : state)
    {
        bios_error_t ret = bios_rom_size(&sz);
        if(ret == BIOS_ERROR_NOT_SUPPORTED || ret == BIOS_ERROR_NOT_FOUND)
        {
            state.SkipWithError("bios_rom_size not supported on this platform");
            return;
        }
        benchmark::DoNotOptimize(sz);
    }
}
BENCHMARK(bm_bios_rom_size);

static void bm_bios_info(benchmark::State &state)
{
    bios_info_t info;
    for(auto _ : state)
    {
        bios_error_t ret = bios_info(&info);
        if(ret == BIOS_ERROR_NOT_SUPPORTED || ret == BIOS_ERROR_NOT_FOUND)
        {
            state.SkipWithError("bios_info not supported on this platform");
            return;
        }
        benchmark::DoNotOptimize(info.vendor);
    }
}
BENCHMARK(bm_bios_info);

static void bm_bios_safe_string_copy(benchmark::State &state)
{
    const char *src = "This is a test string for bios_safe_string_copy";
    char        dst[128];
    for(auto _ : state)
    {
        bios_error_t ret = bios_safe_string_copy(dst, sizeof(dst), src);
        benchmark::DoNotOptimize(ret);
    }
}
BENCHMARK(bm_bios_safe_string_copy);
