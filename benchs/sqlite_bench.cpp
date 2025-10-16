#include <benchmark/benchmark.h>
#include <hj/db/sqlite.hpp>
#include <cstdio>
#include <string>
#include <vector>

using namespace hj;

static bool is_sqlite_valid()
{
    try
    {
        sqlite db;
        if(!db.open("bench_check.db"))
            return false;
        db.close();
        std::remove("bench_check.db");
        return true;
    }
    catch(...)
    {
        return false;
    }
}

// Open and close database repeatedly
static void bm_sqlite_open_close(benchmark::State &state)
{
    if(!is_sqlite_valid())
    {
        state.SkipWithError("sqlite not available");
        return;
    }

    const char *fname = "bench_openclose.db";
    for(auto _ : state)
    {
        state.PauseTiming();
        std::remove(fname);
        state.ResumeTiming();

        sqlite db;
        bool   ok = db.open(fname);
        benchmark::DoNotOptimize(ok);
        db.close();

        state.PauseTiming();
        std::remove(fname);
        state.ResumeTiming();
    }
}
BENCHMARK(bm_sqlite_open_close)->Iterations(1000);

// Exec many INSERT statements (measures exec overhead)
static void bm_sqlite_exec_inserts(benchmark::State &state)
{
    if(!is_sqlite_valid())
    {
        state.SkipWithError("sqlite not available");
        return;
    }

    const size_t inserts = static_cast<size_t>(state.range(0));
    const char  *fname   = "bench_exec.db";

    for(auto _ : state)
    {
        state.PauseTiming();
        std::remove(fname);
        sqlite db;
        db.open(fname);
        db.exec("DROP TABLE IF EXISTS t;");
        db.exec("CREATE TABLE t (id INTEGER PRIMARY KEY, v TEXT);");
        state.ResumeTiming();

        for(size_t i = 0; i < inserts; ++i)
        {
            db.exec("INSERT INTO t (v) VALUES ('x');");
        }

        benchmark::DoNotOptimize(db.is_open());

        state.PauseTiming();
        db.close();
        std::remove(fname);
        state.ResumeTiming();
    }
}
BENCHMARK(bm_sqlite_exec_inserts)->Arg(100)->Arg(1000);

// Query performance: prepare DB with N rows then measure SELECT performance
static void bm_sqlite_query(benchmark::State &state)
{
    if(!is_sqlite_valid())
    {
        state.SkipWithError("sqlite not available");
        return;
    }

    const size_t rows  = static_cast<size_t>(state.range(0));
    const char  *fname = "bench_query.db";

    for(auto _ : state)
    {
        state.PauseTiming();
        std::remove(fname);
        sqlite db;
        db.open(fname);
        db.exec("DROP TABLE IF EXISTS t;");
        db.exec("CREATE TABLE t (id INTEGER PRIMARY KEY, v INTEGER);");
        for(size_t i = 0; i < rows; ++i)
            db.exec("INSERT INTO t (v) VALUES (0);");
        state.ResumeTiming();

        auto result = db.query("SELECT id, v FROM t WHERE v=0;", nullptr);
        benchmark::DoNotOptimize(result.size());

        state.PauseTiming();
        db.close();
        std::remove(fname);
        state.ResumeTiming();
    }
}
BENCHMARK(bm_sqlite_query)->Arg(100)->Arg(1000)->Arg(10000);

// Transaction overhead: begin + insert + rollback/commit
static void bm_sqlite_transaction(benchmark::State &state)
{
    if(!is_sqlite_valid())
    {
        state.SkipWithError("sqlite not available");
        return;
    }

    const char *fname = "bench_trans.db";
    for(auto _ : state)
    {
        state.PauseTiming();
        std::remove(fname);
        sqlite db;
        db.open(fname);
        db.exec("DROP TABLE IF EXISTS t;");
        db.exec("CREATE TABLE t (id INTEGER PRIMARY KEY, v INTEGER);");
        state.ResumeTiming();

        db.begin();
        db.exec("INSERT INTO t (v) VALUES (1);");
        db.commit();

        benchmark::DoNotOptimize(db.is_open());

        state.PauseTiming();
        db.close();
        std::remove(fname);
        state.ResumeTiming();
    }
}
BENCHMARK(bm_sqlite_transaction)->Iterations(1000);
