#include <benchmark/benchmark.h>
#include <hj/hardware/ram.h>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>

static bool env_bool(const char *name)
{
    const char *v = std::getenv(name);
    return v && (v[0] == '1' || v[0] == 'y' || v[0] == 'Y');
}

// Benchmark: get system memory info (cheap)
static void bm_ram_get_system_info(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        ram_system_info_t info;
        state.ResumeTiming();

        ram_err_t err = ram_get_system_info(&info);
        benchmark::DoNotOptimize(err);
        if(err == RAM_SUCCESS)
        {
            benchmark::DoNotOptimize(info.total_physical);
            benchmark::DoNotOptimize(info.available_physical);
            benchmark::DoNotOptimize(info.memory_load);
        }
    }
}

// Benchmark: allocate and free aligned memory
static void bm_ram_allocate_free_aligned(benchmark::State &state)
{
    const size_t size      = 4096;
    const size_t alignment = 64;
    for(auto _ : state)
    {
        state.PauseTiming();
        void *ptr = nullptr;
        state.ResumeTiming();

        ram_err_t err = ram_allocate_aligned(size, alignment, &ptr);
        benchmark::DoNotOptimize(err);
        if(err == RAM_SUCCESS)
        {
            // touch memory to ensure allocation
            std::memset(ptr, 0xAA, size);
            benchmark::DoNotOptimize(ptr);
            ram_free_aligned(ptr);
        }
    }
}

// Benchmark: format various sizes to string buffer
static void bm_ram_format_size(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        char buf[64];
        state.ResumeTiming();

        ram_err_t err = ram_format_size(1024 * 1024 * 2ULL, buf, sizeof(buf));
        benchmark::DoNotOptimize(err);
        benchmark::DoNotOptimize(buf[0]);
    }
}

// Benchmark: prefetch memory (guarded — may be no-op on some platforms)
static void bm_ram_prefetch_memory(benchmark::State &state)
{
    const size_t size = 64 * 1024; // 64 KB
    for(auto _ : state)
    {
        state.PauseTiming();
        void     *ptr       = nullptr;
        ram_err_t alloc_err = ram_allocate_aligned(size, 4096, &ptr);
        state.ResumeTiming();

        if(alloc_err == RAM_SUCCESS)
        {
            ram_err_t err = ram_prefetch_memory(ptr, size);
            benchmark::DoNotOptimize(err);
        }

        state.PauseTiming();
        if(alloc_err == RAM_SUCCESS)
            ram_free_aligned(ptr);
        state.ResumeTiming();
    }
}

// Benchmark: allocate/free large pages — guarded by env var to avoid CI surprises
static void bm_ram_large_page_alloc(benchmark::State &state)
{
    if(!env_bool("HJ_BENCH_ALLOW_RAM_HEAVY"))
    {
        state.SkipWithError("HJ_BENCH_ALLOW_RAM_HEAVY not set");
        return;
    }

    const size_t large_size = 2 * 1024 * 1024; // 2MB
    for(auto _ : state)
    {
        state.PauseTiming();
        void *ptr = nullptr;
        state.ResumeTiming();

        ram_err_t err = ram_allocate_large_pages(large_size, &ptr);
        benchmark::DoNotOptimize(err);
        if(err == RAM_SUCCESS)
        {
            // touch some memory
            volatile uint8_t *b = (volatile uint8_t *) ptr;
            b[0]                = 0x55;
            state.PauseTiming();
            ram_free_large_pages(ptr, large_size);
            state.ResumeTiming();
        }
    }
}

BENCHMARK(bm_ram_get_system_info)->Iterations(2000);
BENCHMARK(bm_ram_allocate_free_aligned)->Iterations(2000);
BENCHMARK(bm_ram_format_size)->Iterations(5000);
BENCHMARK(bm_ram_prefetch_memory)->Iterations(1000);
BENCHMARK(bm_ram_large_page_alloc)->Iterations(200);
