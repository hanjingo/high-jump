#include <benchmark/benchmark.h>
#include <hj/testing/stacktrace.hpp>
#include <sstream>
#include <cstdlib>
#include <iostream>

static bool env_allow_throw()
{
    const char *v = std::getenv("HJ_BENCH_ALLOW_THROW");
    return v && std::string(v) == "1";
}

// Measure cost of constructing a stacktrace object
static void bm_stacktrace_create(benchmark::State &state)
{
    for(auto _ : state)
    {
        auto st = hj::stacktrace();
        benchmark::DoNotOptimize(st);
    }
}
BENCHMARK(bm_stacktrace_create)->Iterations(1000);

// Measure cost of streaming a stacktrace into a stringstream
static void bm_stacktrace_stream(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        auto st = hj::stacktrace();
        state.ResumeTiming();

        std::ostringstream oss;
        oss << st;
        benchmark::DoNotOptimize(oss.str().size());
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_stacktrace_stream)->Iterations(500);

// Optional: measure throw/catch + stacktrace printing path. Guarded to avoid
// disturbing CI unless explicitly enabled via HJ_BENCH_ALLOW_THROW=1.
static void bm_stacktrace_on_exception(benchmark::State &state)
{
    if(!env_allow_throw())
    {
        state.SkipWithError("HJ_BENCH_ALLOW_THROW not set");
        return;
    }

    for(auto _ : state)
    {
        try
        {
            throw std::runtime_error("bench exception");
        }
        catch(...)
        {
            std::ostringstream oss;
            oss << hj::stacktrace();
            benchmark::DoNotOptimize(oss.str().size());
        }
    }
}
BENCHMARK(bm_stacktrace_on_exception)->Iterations(200);
