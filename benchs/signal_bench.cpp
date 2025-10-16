#include <benchmark/benchmark.h>
#include <hj/os/signal.hpp>
#include <cstdlib>
#include <string>
#include <atomic>

#if defined(_WIN32) || defined(_WIN64)
#define TEST_SIG1 SIGINT
#define TEST_SIG2 SIGFPE
#else
#define TEST_SIG1 SIGUSR1
#define TEST_SIG2 SIGUSR2
#endif

static bool env_allows_signal()
{
    const char *v = std::getenv("HJ_BENCH_ALLOW_SIGNAL");
    return v && std::string(v) == "1";
}

// Measure registering a handler and calling poll when no signal has been raised
static void bm_signal_register_and_poll_no_raise(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        using hj::sighandler;
        // register a handler but do not actually raise; poll should be a fast no-op
        sighandler::instance().sigcatch(TEST_SIG1, [](int) {}, false);
        state.ResumeTiming();

        sighandler::instance().poll();

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_signal_register_and_poll_no_raise)->Iterations(1000);

// Measure is_registered / is_one_shoot checks
static void bm_signal_status_checks(benchmark::State &state)
{
    using hj::sighandler;
    state.PauseTiming();
    sighandler::instance().sigcatch(TEST_SIG1, [](int) {}, false);
    sighandler::instance().sigcatch(TEST_SIG2, [](int) {}, true);
    state.ResumeTiming();

    for(auto _ : state)
    {
        bool a = sighandler::instance().is_registered(TEST_SIG1);
        bool b = sighandler::instance().is_one_shoot(TEST_SIG2);
        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
    }
}
BENCHMARK(bm_signal_status_checks)->Iterations(10000);

// Real raise + poll: this performs OS signal raises and is guarded by HJ_BENCH_ALLOW_SIGNAL=1
static void bm_signal_raise_and_poll(benchmark::State &state)
{
    if(!env_allows_signal())
    {
        state.SkipWithError("HJ_BENCH_ALLOW_SIGNAL not set");
        return;
    }

    using hj::sighandler;
    std::atomic<int> counter{0};

    // one_shoot=false so handler remains
    sighandler::instance().sigcatch(
        TEST_SIG1,
        [&counter](int) { counter.fetch_add(1); },
        false);

    for(auto _ : state)
    {
        // raise and then poll to invoke the callback in controlled context
        bool ok = sighandler::instance().sigraise(TEST_SIG1);
        benchmark::DoNotOptimize(ok);
        sighandler::instance().poll();
        benchmark::DoNotOptimize(counter.load());
    }
}
BENCHMARK(bm_signal_raise_and_poll)->Iterations(1000);
