#include <benchmark/benchmark.h>
#include <hj/os/dll.h>
#include <string>

static void bm_build_lib_filename(benchmark::State &st)
{
    for(auto _ : st)
    {
#if defined(_WIN32)
        std::string s = std::string("./dll_example") + DLL_EXT;
#else
        std::string s = std::string("./libdll_example") + DLL_EXT;
#endif
        benchmark::DoNotOptimize(s);
    }
}
BENCHMARK(bm_build_lib_filename)->Iterations(10000);

static void bm_dll_ext_length(benchmark::State &st)
{
    for(auto _ : st)
    {
        volatile size_t len = strlen(DLL_EXT);
        benchmark::DoNotOptimize(len);
    }
}
BENCHMARK(bm_dll_ext_length)->Iterations(100000);

static void bm_dll_open_nonexistent(benchmark::State &st)
{
#if defined(_WIN32)
    const char *name = "./this_library_should_not_exist.dll";
#else
    const char *name = "./this_library_should_not_exist.so";
#endif
    for(auto _ : st)
    {
        void *h = dll_open(name, DLL_RTLD_LAZY);
        if(h)
            dll_close(h);
        benchmark::DoNotOptimize(h);
    }
}
BENCHMARK(bm_dll_open_nonexistent)->Iterations(500);
