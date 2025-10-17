#include <benchmark/benchmark.h>
#include <hj/types/string_view.hpp>
#include <string>

// Benchmarks for hj::string_view (thin wrapper over std::string_view or boost)

// Construct from C-string
static void bm_string_view_construct_cstr(benchmark::State &state)
{
    const char *cstr = "the quick brown fox jumps over the lazy dog";
    for(auto _ : state)
    {
        hj::string_view sv(cstr);
        benchmark::DoNotOptimize(sv);
    }
}
BENCHMARK(bm_string_view_construct_cstr)->Iterations(100000);

// Construct from std::string (string exists outside timing)
static void bm_string_view_from_std_string(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        std::string s = "abcdefghijklmnopqrstuvwxyz0123456789";
        state.ResumeTiming();

        hj::string_view sv(s);
        benchmark::DoNotOptimize(sv);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_string_view_from_std_string)->Iterations(100000);

// Compare (== and <) hot path
static void bm_string_view_compare(benchmark::State &state)
{
    const int checks = static_cast<int>(state.range(0));
    state.PauseTiming();
    hj::string_view a("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    hj::string_view b("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    hj::string_view c("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");
    state.ResumeTiming();

    for(auto _ : state)
    {
        for(int i = 0; i < checks; ++i)
        {
            bool eq = (a == b);
            bool lt = (a < c);
            benchmark::DoNotOptimize(eq);
            benchmark::DoNotOptimize(lt);
        }
    }
}
BENCHMARK(bm_string_view_compare)->Arg(100)->Arg(1000);

// substr and find behavior
static void bm_string_view_substr_find(benchmark::State &state)
{
    const int repeats = static_cast<int>(state.range(0));
    state.PauseTiming();
    hj::string_view sv("ab_cd_ef_gh_ij_kl_mn_op_qr_st_uv_wxyz");
    state.ResumeTiming();

    for(auto _ : state)
    {
        for(int i = 0; i < repeats; ++i)
        {
            auto pos = sv.find("_ef_");
            benchmark::DoNotOptimize(pos);
            if(pos != hj::string_view::npos)
            {
                auto sub = sv.substr(pos + 1, 2);
                benchmark::DoNotOptimize(sub);
            }
        }
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_string_view_substr_find)->Arg(100)->Arg(1000);

// index access
static void bm_string_view_index_access(benchmark::State &state)
{
    const int idxs = static_cast<int>(state.range(0));
    state.PauseTiming();
    hj::string_view sv("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    state.ResumeTiming();

    for(auto _ : state)
    {
        for(int i = 0; i < idxs; ++i)
        {
            char c = sv[i % sv.size()];
            benchmark::DoNotOptimize(c);
        }
    }
}
BENCHMARK(bm_string_view_index_access)->Arg(1000)->Arg(10000);
