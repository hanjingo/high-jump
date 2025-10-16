#include <benchmark/benchmark.h>

#include <hj/hardware/gpu.h>
#include <vector>

static void bm_gpu_count(benchmark::State &state)
{
    for(auto _ : state)
    {
        int c = gpu_count();
        (void) c;
        benchmark::ClobberMemory();
    }
}

static void bm_gpu_device_get_free(benchmark::State &state)
{
    for(auto _ : state)
    {
        gpu_device_info_t *infos = NULL;
        int                count = 0;
        gpu_device_get(&infos, &count);
        if(infos)
            gpu_device_free(infos, count);
        benchmark::ClobberMemory();
    }
}

static void bm_gpu_context_create_free(benchmark::State &state)
{
    for(auto _ : state)
    {
        gpu_context_t ctx;
        bool          created = gpu_context_create(&ctx, nullptr);
        if(created)
            gpu_context_free(&ctx);
        benchmark::ClobberMemory();
    }
}

static void bm_gpu_program_create_invalid(benchmark::State &state)
{
    const char *invalid_source = "this is not valid GPU code";
    for(auto _ : state)
    {
        gpu_program_t program;
        char          log[128];
        unsigned long log_sz = sizeof(log);
        bool          ok     = gpu_program_create_from_source(&program,
                                                 nullptr,
                                                 log,
                                                 &log_sz,
                                                 invalid_source);
        (void) ok;
        benchmark::ClobberMemory();
    }
}

BENCHMARK(bm_gpu_count);
BENCHMARK(bm_gpu_device_get_free);
BENCHMARK(bm_gpu_context_create_free);
BENCHMARK(bm_gpu_program_create_invalid);
