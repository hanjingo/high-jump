#include <benchmark/benchmark.h>
#include <hj/testing/crash.hpp>
#include <string>
#include <memory>

// Benchmarks for hj::testing::crash_handler

static void bm_crash_create_destroy(benchmark::State &st)
{
    for(auto _ : st)
    {
        // create a temporary handler and allow it to be destroyed
        auto h = std::make_unique<hj::crash_handler>("./bench_temp");
        benchmark::DoNotOptimize(h);
    }
}
BENCHMARK(bm_crash_create_destroy)->Iterations(1000);

static void bm_crash_set_local_path(benchmark::State &st)
{
    hj::crash_handler h;
    for(auto _ : st)
    {
        // set various paths repeatedly to exercise string handling & handler re-init
        for(int i = 0; i < 10; ++i)
        {
            std::string p = std::string("./bench_path_") + std::to_string(i);
            h.set_local_path(p);
            benchmark::ClobberMemory();
        }
    }
}
BENCHMARK(bm_crash_set_local_path)->Iterations(200);

// Platform-specific safe dump callbacks matching hj::crash_handler expectations
#if defined(_WIN32)
static bool win_cb(const wchar_t      *dump_dir,
                   const wchar_t      *minidump_id,
                   void               *context,
                   EXCEPTION_POINTERS *exinfo,
                   MDRawAssertionInfo *assertion,
                   bool                succeeded)
{
    (void) dump_dir;
    (void) minidump_id;
    (void) context;
    (void) exinfo;
    (void) assertion;
    (void) succeeded;
    return true;
}
#elif defined(__APPLE__)
static bool apple_cb(const char *dump_dir,
                     const char *minidump_id,
                     void       *context,
                     bool        succeeded)
{
    (void) dump_dir;
    (void) minidump_id;
    (void) context;
    (void) succeeded;
    return true;
}
#else
static bool linux_cb(const google_breakpad::MinidumpDescriptor &descriptor,
                     void                                      *context,
                     bool                                       succeeded)
{
    (void) descriptor;
    (void) context;
    (void) succeeded;
    return true;
}
#endif

static void bm_crash_set_dump_callback(benchmark::State &st)
{
    hj::crash_handler h;
    for(auto _ : st)
    {
#if defined(_WIN32)
        h.set_dump_callback(win_cb);
#elif defined(__APPLE__)
        h.set_dump_callback(apple_cb);
#else
        h.set_dump_callback(linux_cb);
#endif
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_crash_set_dump_callback)->Iterations(500);

static void bm_crash_singleton_access(benchmark::State &st)
{
    for(auto _ : st)
    {
        auto p = hj::crash_handler::instance();
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(bm_crash_singleton_access)->Iterations(10000);
