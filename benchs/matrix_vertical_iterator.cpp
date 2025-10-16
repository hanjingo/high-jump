#include <benchmark/benchmark.h>
#include <hj/math/matrix_vertical_iterator.hpp>
#include <vector>
#include <cstddef>

using hj::matrix_vertical_iterator;

// Lightweight wrapper over std::vector<vector<int>> that provides
// operator()(row,col) used by matrix_vertical_iterator in tests.
struct Vec2DWrapperV
{
    std::vector<std::vector<int>> &data;
    Vec2DWrapperV(std::vector<std::vector<int>> &d)
        : data(d)
    {
    }
    int &operator()(int row, int col) const { return data[row][col]; }
};

static void bm_matrixverticaliterator_deref_inc(benchmark::State &state)
{
    const int                                    R = 200, C = 200;
    std::vector<std::vector<int>>                d(R, std::vector<int>(C, 1));
    Vec2DWrapperV                                w(d);
    matrix_vertical_iterator<Vec2DWrapperV, int> it(&w, R, C, 0);
    matrix_vertical_iterator<Vec2DWrapperV, int> end(&w, R, C, R * C);

    for(auto _ : state)
    {
        auto      cur = it;
        long long sum = 0;
        for(; cur != end; ++cur)
            sum += *cur;
        benchmark::DoNotOptimize(sum);
    }
}

static void bm_matrixverticaliterator_randomaccess(benchmark::State &state)
{
    const int                     R = 200, C = 200;
    std::vector<std::vector<int>> d(R, std::vector<int>(C, 2));
    Vec2DWrapperV                 w(d);
    auto base = matrix_vertical_iterator<Vec2DWrapperV, int>(&w, R, C, 0);

    for(auto _ : state)
    {
        auto a    = base + 100;
        auto b    = a + 500;
        auto diff = b - a;
        benchmark::DoNotOptimize(diff);
        benchmark::DoNotOptimize((*a));
    }
}

static void
bm_matrixverticaliterator_compare_copyassign(benchmark::State &state)
{
    const int                     R = 100, C = 100;
    std::vector<std::vector<int>> d(R, std::vector<int>(C, 3));
    Vec2DWrapperV                 w(d);
    auto it = matrix_vertical_iterator<Vec2DWrapperV, int>(&w, R, C, 0);
    auto mid =
        matrix_vertical_iterator<Vec2DWrapperV, int>(&w, R, C, (R * C) / 2);

    for(auto _ : state)
    {
        auto it2 = it;
        bool eq  = (it2 == it);
        bool lt  = (it2 < mid);
        benchmark::DoNotOptimize(eq);
        benchmark::DoNotOptimize(lt);
        it2 += 10;
        benchmark::DoNotOptimize(it2.offset());
    }
}

static void bm_matrixverticaliterator_iteration_sum(benchmark::State &state)
{
    const int                     R = 300, C = 300;
    std::vector<std::vector<int>> d(R, std::vector<int>(C, 1));
    Vec2DWrapperV                 w(d);
    auto begin = matrix_vertical_iterator<Vec2DWrapperV, int>(&w, R, C, 0);
    auto end   = matrix_vertical_iterator<Vec2DWrapperV, int>(&w, R, C, R * C);

    for(auto _ : state)
    {
        long long sum = 0;
        for(auto it = begin; it != end; ++it)
            sum += *it;
        benchmark::DoNotOptimize(sum);
    }
}

BENCHMARK(bm_matrixverticaliterator_deref_inc)->Iterations(2000);
BENCHMARK(bm_matrixverticaliterator_randomaccess)->Iterations(5000);
BENCHMARK(bm_matrixverticaliterator_compare_copyassign)->Iterations(5000);
BENCHMARK(bm_matrixverticaliterator_iteration_sum)->Iterations(200);