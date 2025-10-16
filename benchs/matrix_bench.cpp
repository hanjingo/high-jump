#include <benchmark/benchmark.h>
#include <hj/math/matrix.hpp>
#include <vector>
#include <chrono>

using namespace hj;

static void bm_matrix_default_construct(benchmark::State &state)
{
    for(auto _ : state)
    {
        matrix<int> m;
        benchmark::DoNotOptimize(m);
    }
}

static void bm_matrix_size_construct(benchmark::State &state)
{
    for(auto _ : state)
    {
        matrix<int> m(100, 100);
        benchmark::DoNotOptimize(m);
    }
}

static void bm_matrix_value_construct(benchmark::State &state)
{
    for(auto _ : state)
    {
        matrix<int> m(200, 200, 7);
        benchmark::DoNotOptimize(m);
    }
}

static void bm_matrix_copy_assign(benchmark::State &state)
{
    matrix<int> src(100, 100, 5);
    for(auto _ : state)
    {
        matrix<int> a(src);
        matrix<int> b;
        b = src;
        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
    }
}

static void bm_matrix_element_access(benchmark::State &state)
{
    matrix<int> m(200, 200, 0);
    for(auto _ : state)
    {
        for(int r = 0; r < m.row_n(); ++r)
            for(int c = 0; c < m.col_n(); ++c)
                m.at(r, c) = r + c;

        int s = 0;
        for(int r = 0; r < m.row_n(); ++r)
            for(int c = 0; c < m.col_n(); ++c)
                s += m[r][c];
        benchmark::DoNotOptimize(s);
    }
}

static void bm_matrix_resize(benchmark::State &state)
{
    matrix<int> m(50, 50, 1);
    for(auto _ : state)
    {
        state.PauseTiming();
        // prepare new size values (not measured)
        int nr = 100;
        int nc = 120;
        state.ResumeTiming();

        m.resize(nr, nc);
        benchmark::DoNotOptimize(m);
    }
}

static void bm_matrix_row_iterate(benchmark::State &state)
{
    matrix<int> m(300, 300, 1);
    for(auto _ : state)
    {
        long long sum = 0;
        for(auto it = m.begin(); it != m.end(); ++it)
            sum += *it;
        benchmark::DoNotOptimize(sum);
    }
}

static void bm_matrix_column_iterate(benchmark::State &state)
{
    matrix<int> m(300, 300, 1);
    for(auto _ : state)
    {
        long long sum = 0;
        for(auto it = m.vbegin(); it != m.vend(); ++it)
            sum += *it;
        benchmark::DoNotOptimize(sum);
    }
}

// register benchmarks with conservative iterations for CI
BENCHMARK(bm_matrix_default_construct)->Iterations(50000);
BENCHMARK(bm_matrix_size_construct)->Iterations(5000);
BENCHMARK(bm_matrix_value_construct)->Iterations(2000);
BENCHMARK(bm_matrix_copy_assign)->Iterations(2000);
BENCHMARK(bm_matrix_element_access)->Iterations(500);
BENCHMARK(bm_matrix_resize)->Iterations(500);
BENCHMARK(bm_matrix_row_iterate)->Iterations(200);
BENCHMARK(bm_matrix_column_iterate)->Iterations(200);