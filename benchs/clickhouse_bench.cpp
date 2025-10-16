#include <benchmark/benchmark.h>

#if CLICKHOUSE_ENABLE
#include <hj/db/clickhouse.hpp>
#include <memory>
#include <string>
#include <vector>

// Build a clickhouse::Block, create several columns and append N rows.
static void bm_build_block_append_columns(benchmark::State &st)
{
    const int rows = static_cast<int>(st.range(0));
    for(auto _ : st)
    {
        hj::ck::block b;
        auto          id    = std::make_shared<hj::ck::column_uint64>();
        auto          name  = std::make_shared<hj::ck::column_string>();
        auto          score = std::make_shared<hj::ck::column_float64>();

        for(int i = 0; i < rows; ++i)
        {
            id->Append(static_cast<uint64_t>(i));
            name->Append(std::string("user-") + std::to_string(i));
            score->Append(50.0 + (i % 50));
        }

        b.AppendColumn("id", id);
        b.AppendColumn("name", name);
        b.AppendColumn("score", score);

        benchmark::DoNotOptimize(b.GetRowCount());
    }
}
BENCHMARK(bm_build_block_append_columns)->Arg(1)->Arg(10)->Arg(100)->Arg(1000);

// Measure the overhead of calling the utility append_column wrapper repeatedly.
static void bm_append_column_helper(benchmark::State &st)
{
    hj::ck::block b;
    auto          col = std::make_shared<hj::ck::column_uint64>();
    for(int i = 0; i < 100; ++i)
        col->Append(static_cast<uint64_t>(i));

    for(auto _ : st)
    {
        hj::ck::append_column(b, "col", col);
        benchmark::DoNotOptimize(b.GetRowCount());
    }
}
BENCHMARK(bm_append_column_helper)->Iterations(1000);

#else

// ClickHouse not enabled: provide a noop benchmark so the bench target still builds.
static void bm_clickhouse_not_enabled(benchmark::State &st)
{
    for(auto _ : st)
        benchmark::DoNotOptimize(0);
}
BENCHMARK(bm_clickhouse_not_enabled)->Arg(1);

#endif
