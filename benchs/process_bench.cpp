#include <benchmark/benchmark.h>
#include <hj/os/process.hpp>
#include <cstdlib>
#include <string>
#include <vector>

namespace
{

static bool env_bool(const char *name)
{
    const char *v = std::getenv(name);
    return v && (v[0] == '1' || v[0] == 'y' || v[0] == 'Y');
}

static std::string env_str(const char *name)
{
    const char *v = std::getenv(name);
    return v ? std::string(v) : std::string();
}

// Cheap: measure getpid() overhead
static void bm_process_getpid(benchmark::State &state)
{
    for(auto _ : state)
    {
        hj::process::pid_t p = hj::process::getpid();
        benchmark::DoNotOptimize(p);
    }
}

// Cheap: measure getppid() overhead
static void bm_process_getppid(benchmark::State &state)
{
    for(auto _ : state)
    {
        hj::process::pid_t p = hj::process::getppid();
        benchmark::DoNotOptimize(p);
    }
}

// Measure fast process listing with a simple matcher (no heavy work)
static void bm_process_list_quick(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        std::vector<hj::process::pid_t> result;
        state.ResumeTiming();

        hj::process::list(result,
                          [](const std::vector<std::string> &a) -> bool {
                              // quick rejection: require at least 2 fields
                              return a.size() >= 2;
                          });
        benchmark::DoNotOptimize(result.size());
    }
}

// Optional: spawn a child process and kill it. Runs only if HJ_BENCH_ALLOW_PROCESS=1
// and HJ_BENCH_CHILD_PATH is provided. This avoids running processes in CI by default.
static void bm_process_spawn_and_kill(benchmark::State &state)
{
    if(!env_bool("HJ_BENCH_ALLOW_PROCESS"))
    {
        state.SkipWithError("HJ_BENCH_ALLOW_PROCESS not set");
        return;
    }
    const std::string child_path = env_str("HJ_BENCH_CHILD_PATH");
    if(child_path.empty())
    {
        state.SkipWithError("HJ_BENCH_CHILD_PATH not provided");
        return;
    }

    for(auto _ : state)
    {
        state.PauseTiming();
        state.ResumeTiming();

        // measure spawn
        hj::process::pid_t pid = hj::process::child(child_path);
        benchmark::DoNotOptimize(pid);

        // cleanup outside timing
        state.PauseTiming();
        if(pid)
            hj::process::kill(pid);
        state.ResumeTiming();
    }
}

} // namespace

BENCHMARK(bm_process_getpid)->Iterations(200000);
BENCHMARK(bm_process_getppid)->Iterations(200000);
BENCHMARK(bm_process_list_quick)->Iterations(2000);
BENCHMARK(bm_process_spawn_and_kill)->Iterations(200);
