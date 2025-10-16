#include <benchmark/benchmark.h>
#include <hj/os/compat.hpp>
#include <functional>

// Small function call overhead: FORCE_INLINE vs NO_INLINE
FORCE_INLINE int bench_add_inline(int a, int b)
{
    return a + b;
}

NO_INLINE int bench_add_noinline(int a, int b)
{
    return a + b;
}

static void bm_add_inline(benchmark::State &st)
{
    int a = 1, b = 2;
    int acc = 0;
    for(auto _ : st)
    {
        for(int i = 0; i < 1000; ++i)
            acc += bench_add_inline(a, b);
    }
    benchmark::DoNotOptimize(acc);
}
BENCHMARK(bm_add_inline)->Iterations(100);

static void bm_add_noinline(benchmark::State &st)
{
    int a = 1, b = 2;
    int acc = 0;
    for(auto _ : st)
    {
        for(int i = 0; i < 1000; ++i)
            acc += bench_add_noinline(a, b);
    }
    benchmark::DoNotOptimize(acc);
}
BENCHMARK(bm_add_noinline)->Iterations(100);

// Branch prediction microbench: LIKELY vs UNLIKELY
static void bm_branch_likely(benchmark::State &st)
{
    volatile int sink = 0;
    for(auto _ : st)
    {
        int n = 0;
        for(int i = 0; i < 100000; ++i)
        {
            if(LIKELY(i % 100 != 0))
                n += 1;
            else
                n += 2;
        }
        sink = n;
    }
    benchmark::DoNotOptimize(sink);
}
BENCHMARK(bm_branch_likely)->Iterations(10);

static void bm_branch_unlikely(benchmark::State &st)
{
    volatile int sink = 0;
    for(auto _ : st)
    {
        int n = 0;
        for(int i = 0; i < 100000; ++i)
        {
            if(UNLIKELY(i % 100 == 0))
                n += 2;
            else
                n += 1;
        }
        sink = n;
    }
    benchmark::DoNotOptimize(sink);
}
BENCHMARK(bm_branch_unlikely)->Iterations(10);

// Functor call vs std::function wrapper
struct MyFunc : std::unary_function<int, int>
{
    int operator()(int x) const { return x + 1; }
};

static void bm_functor_call(benchmark::State &st)
{
    MyFunc f;
    int    acc = 0;
    for(auto _ : st)
    {
        for(int i = 0; i < 10000; ++i)
            acc += f(i);
    }
    benchmark::DoNotOptimize(acc);
}
BENCHMARK(bm_functor_call)->Iterations(50);

static void bm_std_function_call(benchmark::State &st)
{
    std::function<int(int)> fn  = MyFunc();
    int                     acc = 0;
    for(auto _ : st)
    {
        for(int i = 0; i < 10000; ++i)
            acc += fn(i);
    }
    benchmark::DoNotOptimize(acc);
}
BENCHMARK(bm_std_function_call)->Iterations(50);
