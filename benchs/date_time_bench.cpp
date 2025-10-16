#include <benchmark/benchmark.h>
#include <hj/time/date_time.hpp>
#include <string>

using hj::date_time;

static void bm_default_ctor(benchmark::State &st)
{
    for(auto _ : st)
    {
        date_time dt;
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_default_ctor)->Iterations(10000);

static void bm_components_ctor(benchmark::State &st)
{
    for(auto _ : st)
    {
        date_time dt(2023, 1, 1, 12, 34, 56);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_components_ctor)->Iterations(10000);

static void bm_from_time_t(benchmark::State &st)
{
    std::time_t t = std::time(nullptr);
    for(auto _ : st)
    {
        date_time dt(t);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_from_time_t)->Iterations(10000);

static void bm_now_utc(benchmark::State &st)
{
    for(auto _ : st)
    {
        auto dt = date_time::now(hj::timezone::UTC);
        benchmark::DoNotOptimize(dt);
    }
}
BENCHMARK(bm_now_utc)->Iterations(2000);

static void bm_now_local(benchmark::State &st)
{
    for(auto _ : st)
    {
        auto dt = date_time::now(hj::timezone::LOCAL);
        benchmark::DoNotOptimize(dt);
    }
}
BENCHMARK(bm_now_local)->Iterations(2000);

static void bm_today(benchmark::State &st)
{
    for(auto _ : st)
    {
        auto d = date_time::today();
        benchmark::DoNotOptimize(d);
    }
}
BENCHMARK(bm_today)->Iterations(2000);

static void bm_format(benchmark::State &st)
{
    date_time dt(2023, 1, 1, 0, 0, 0);
    for(auto _ : st)
    {
        auto s = date_time::format(dt);
        benchmark::DoNotOptimize(s);
    }
}
BENCHMARK(bm_format)->Iterations(5000);

static void bm_parse_return(benchmark::State &st)
{
    const std::string s = "2023-01-01 12:34:56";
    for(auto _ : st)
    {
        auto dt = date_time::parse(s, TIME_FMT);
        benchmark::DoNotOptimize(dt);
    }
}
BENCHMARK(bm_parse_return)->Iterations(5000);

static void bm_parse_inout(benchmark::State &st)
{
    const std::string s = "2023-01-01 12:34:56";
    date_time         dt;
    for(auto _ : st)
    {
        date_time::parse(dt, s, TIME_FMT);
        benchmark::DoNotOptimize(dt);
    }
}
BENCHMARK(bm_parse_inout)->Iterations(5000);

static void bm_next_prev_ops(benchmark::State &st)
{
    date_time d(2023, 2, 15);
    for(auto _ : st)
    {
        auto a = d.next_day();
        auto b = d.pre_day();
        auto c = d.next_month();
        auto e = d.pre_month();
        auto f = d.next_year();
        auto g = d.pre_year();
        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
        benchmark::DoNotOptimize(c);
        benchmark::DoNotOptimize(e);
        benchmark::DoNotOptimize(f);
        benchmark::DoNotOptimize(g);
    }
}
BENCHMARK(bm_next_prev_ops)->Iterations(2000);

static void bm_is_working_and_weekday(benchmark::State &st)
{
    date_time d(2025, 7, 11);
    for(auto _ : st)
    {
        bool w  = d.is_working_day();
        auto wd = d.day_of_week();
        auto ws = d.day_of_week_str();
        benchmark::DoNotOptimize(w);
        benchmark::DoNotOptimize(wd);
        benchmark::DoNotOptimize(ws);
    }
}
BENCHMARK(bm_is_working_and_weekday)->Iterations(5000);