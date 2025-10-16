#include <benchmark/benchmark.h>

#if defined(__has_include)
#if __has_include(<hj/sync/coroutine.hpp>)
#define HJ_HAS_COROUTINE 1
#include <hj/sync/coroutine.hpp>
#include <string>
#include <memory>
#endif
#endif

#ifndef HJ_HAS_COROUTINE
// Fallback noop benchmark when Boost.Coroutine2 or project header isn't available.
static void bm_coroutine_not_available(benchmark::State &st)
{
    for(auto _ : st)
        benchmark::DoNotOptimize(0);
}
BENCHMARK(bm_coroutine_not_available)->Arg(1);
#else

using hj::coroutine;

// Benchmark: cost to construct/destroy a pull_type coroutine with an empty body
static void bm_construct_pull_empty(benchmark::State &st)
{
    for(auto _ : st)
    {
        coroutine<void>::pull_type p([](coroutine<void>::push_type &) {});
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_construct_pull_empty)->Iterations(1000);

// Benchmark: perform N yield/resume round-trips between push and pull
static void bm_roundtrip_switches(benchmark::State &st)
{
    const int rounds = static_cast<int>(st.range(0));
    for(auto _ : st)
    {
        int                       counter = 0;
        coroutine<int>::pull_type p([&](coroutine<int>::push_type &out) {
            // producer: push numbers, consumer will pull
            for(int i = 0; i < rounds; ++i)
            {
                out(i);
            }
            // final empty push to signal
            out(0);
        });

        // consumer: pull all values
        while(p.get() != 0)
        {
            counter += p.get();
            p();
        }
        benchmark::DoNotOptimize(counter);
    }
}
BENCHMARK(bm_roundtrip_switches)->Arg(10)->Arg(100)->Arg(1000);

// Benchmark: transfer strings between push/pull to measure copy/move overhead
static void bm_string_transfer(benchmark::State &st)
{
    const int rounds = static_cast<int>(st.range(0));
    for(auto _ : st)
    {
        coroutine<std::string>::pull_type p(
            [&](coroutine<std::string>::push_type &out) {
                for(int i = 0; i < rounds; ++i)
                {
                    out(std::string(128, 'x'));
                }
                out(std::string());
            });

        size_t total = 0;
        while(p.get().size() != 0)
        {
            total += p.get().size();
            p();
        }
        benchmark::DoNotOptimize(total);
    }
}
BENCHMARK(bm_string_transfer)->Arg(10)->Arg(100)->Arg(500);

#endif // HJ_HAS_COROUTINE
