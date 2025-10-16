#include <benchmark/benchmark.h>
#include <hj/types/noncopyable.hpp>
#include <memory>

// mirror test helper types
class MyNoncopyable : public hj::noncopyable
{
  public:
    MyNoncopyable() = default;
    int value       = 42;
};

class MyNoncopyableMove
{
  public:
    MyNoncopyableMove() = default;
    DISABLE_COPY_MOVE(MyNoncopyableMove)
    int value = 99;
};

static void bm_noncopyable_construct(benchmark::State &state)
{
    for(auto _ : state)
    {
        MyNoncopyable m;
        benchmark::DoNotOptimize(m.value);
    }
}

static void bm_noncopyable_heap_uniqueptr(benchmark::State &state)
{
    for(auto _ : state)
    {
        auto p = std::make_unique<MyNoncopyable>();
        benchmark::DoNotOptimize(p->value);
    }
}

static void bm_noncopyable_pass_by_ref(benchmark::State &state)
{
    MyNoncopyable m;
    for(auto _ : state)
    {
        // pass by ref should be cheap
        auto &r = m;
        benchmark::DoNotOptimize(r.value);
    }
}

static void bm_noncopyable_move_uniqueptr_move(benchmark::State &state)
{
    for(auto _ : state)
    {
        auto a = std::make_unique<MyNoncopyableMove>();
        auto b = std::move(a);
        benchmark::DoNotOptimize(b->value);
    }
}

BENCHMARK(bm_noncopyable_construct)->Iterations(50000);
BENCHMARK(bm_noncopyable_heap_uniqueptr)->Iterations(20000);
BENCHMARK(bm_noncopyable_pass_by_ref)->Iterations(50000);
BENCHMARK(bm_noncopyable_move_uniqueptr_move)->Iterations(20000);