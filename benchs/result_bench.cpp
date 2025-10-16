#include <benchmark/benchmark.h>
#include <hj/types/result.hpp>
#include <boost/leaf.hpp>
#include <string>

namespace
{

// Benchmark: construct a value result<T>
static void bm_result_value(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        state.ResumeTiming();

        hj::result<int> r = 42;
        benchmark::DoNotOptimize(r.value());
    }
}

// Benchmark: construct an error result<T>
static void bm_result_error(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        state.ResumeTiming();

        hj::result<int> r  = boost::leaf::new_error(std::string("err"));
        bool            ok = static_cast<bool>(r);
        benchmark::DoNotOptimize(ok);
    }
}

// Benchmark: move a result<std::string>
static void bm_result_move_string(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::result<std::string> r1 = std::string("abc");
        state.ResumeTiming();

        hj::result<std::string> r2 = std::move(r1);
        benchmark::DoNotOptimize(r2.value());
    }
}

// Benchmark: void result success
static void bm_result_void_success(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        state.ResumeTiming();

        hj::result<void> r  = {};
        bool             ok = static_cast<bool>(r);
        benchmark::DoNotOptimize(ok);
    }
}

// Benchmark: void result error
static void bm_result_void_error(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        state.ResumeTiming();

        hj::result<void> r  = boost::leaf::new_error(std::string("err"));
        bool             ok = static_cast<bool>(r);
        benchmark::DoNotOptimize(ok);
    }
}

// Benchmark: check boolean conversion
static void bm_result_check_bool(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::result<int> good = 1;
        hj::result<int> bad  = boost::leaf::new_error(std::string("err"));
        state.ResumeTiming();

        bool a = static_cast<bool>(good);
        bool b = static_cast<bool>(bad);
        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
    }
}

} // namespace

BENCHMARK(bm_result_value)->Iterations(2000);
BENCHMARK(bm_result_error)->Iterations(2000);
BENCHMARK(bm_result_move_string)->Iterations(2000);
BENCHMARK(bm_result_void_success)->Iterations(2000);
BENCHMARK(bm_result_void_error)->Iterations(2000);
BENCHMARK(bm_result_check_bool)->Iterations(2000);
