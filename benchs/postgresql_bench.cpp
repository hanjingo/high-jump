#include <benchmark/benchmark.h>
#include <hj/db/postgresql.hpp>
#include <string>
#include <cstdlib>
#include <vector>

// Benchmarks for hj::pg_connection
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

// Fast, CI-safe: construct with an invalid conninfo and check is_open()
static void bm_pg_connect_fail(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        const std::string bad = "invalid=1";
        state.ResumeTiming();

        hj::pg_connection conn(bad);
        bool              open = conn.is_open();
        benchmark::DoNotOptimize(open);
    }
}

// If allowed, run real connect/disconnect benchmark using HJ_PG_CONNINFO
static void bm_pg_connect_disconnect_real(benchmark::State &state)
{
    if(!env_bool("HJ_BENCH_ALLOW_PG"))
    {
        state.SkipWithError("HJ_BENCH_ALLOW_PG not set");
        return;
    }

    const std::string conninfo = env_str("HJ_PG_CONNINFO");
    if(conninfo.empty())
    {
        state.SkipWithError("HJ_PG_CONNINFO not provided");
        return;
    }

    for(auto _ : state)
    {
        state.PauseTiming();
        state.ResumeTiming();

        hj::pg_connection conn(conninfo);
        bool              open = conn.is_open();
        benchmark::DoNotOptimize(open);
        conn.disconnect();
    }
}

// If allowed, exec a trivial statement
static void bm_pg_exec_simple_real(benchmark::State &state)
{
    if(!env_bool("HJ_BENCH_ALLOW_PG"))
    {
        state.SkipWithError("HJ_BENCH_ALLOW_PG not set");
        return;
    }
    const std::string conninfo = env_str("HJ_PG_CONNINFO");
    if(conninfo.empty())
    {
        state.SkipWithError("HJ_PG_CONNINFO not provided");
        return;
    }

    hj::pg_connection db(conninfo);
    if(!db.is_open())
    {
        state.SkipWithError("Cannot open pg connection");
        return;
    }

    for(auto _ : state)
    {
        bool ok = db.exec("SELECT 1");
        benchmark::DoNotOptimize(ok);
    }
}

// If allowed, run a small query returning a few rows
static void bm_pg_query_small_real(benchmark::State &state)
{
    if(!env_bool("HJ_BENCH_ALLOW_PG"))
    {
        state.SkipWithError("HJ_BENCH_ALLOW_PG not set");
        return;
    }
    const std::string conninfo = env_str("HJ_PG_CONNINFO");
    if(conninfo.empty())
    {
        state.SkipWithError("HJ_PG_CONNINFO not provided");
        return;
    }

    hj::pg_connection db(conninfo);
    if(!db.is_open())
    {
        state.SkipWithError("Cannot open pg connection");
        return;
    }

    for(auto _ : state)
    {
        auto rows = db.query("SELECT generate_series(1,10)");
        benchmark::DoNotOptimize(rows.size());
    }
}

} // namespace

BENCHMARK(bm_pg_connect_fail)->Iterations(2000);
BENCHMARK(bm_pg_connect_disconnect_real)->Iterations(500);
BENCHMARK(bm_pg_exec_simple_real)->Iterations(1000);
BENCHMARK(bm_pg_query_small_real)->Iterations(500);
