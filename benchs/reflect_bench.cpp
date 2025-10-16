#include <benchmark/benchmark.h>
#include <hj/types/reflect.hpp>
#include <string>
#include <vector>
#include <cstring>

// Benchmarks for hj::reflect
namespace
{

struct TestStruct
{
    int    a;
    double b;
    char   c;
};

static void bm_reflect_type_name(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        int        i = 0;
        TestStruct t{};
        state.ResumeTiming();

        auto n1 = hj::reflect::type_name(i);
        auto n2 = hj::reflect::type_name(t);
        benchmark::DoNotOptimize(n1);
        benchmark::DoNotOptimize(n2);
    }
}

static void bm_reflect_is_pod(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        TestStruct t{};
        state.ResumeTiming();

        bool b1 = hj::reflect::is_pod(t);
        bool b2 = hj::reflect::is_pod(int(1));
        benchmark::DoNotOptimize(b1);
        benchmark::DoNotOptimize(b2);
    }
}

static void bm_reflect_copy_clone(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        TestStruct t{1, 2.5, 'x'};
        state.ResumeTiming();

        auto t2 = hj::reflect::copy(t);
        auto t3 = hj::reflect::clone(t);
        benchmark::DoNotOptimize(t2.a);
        benchmark::DoNotOptimize(t3.b);
    }
}

static void bm_reflect_serialize_unserialize(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        TestStruct t{42, 3.14, 'z'};
        state.ResumeTiming();

        auto buf = hj::reflect::serialize(t);
        auto t2  = hj::reflect::unserialize<TestStruct>(buf.data());
        benchmark::DoNotOptimize(t2.a);
        benchmark::DoNotOptimize(buf.size());
    }
}

static void bm_reflect_offset_size_align_of(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        state.ResumeTiming();

        auto off = hj::reflect::offset_of(&TestStruct::a);
        auto sz  = hj::reflect::size_of(&TestStruct::b);
        auto al  = hj::reflect::align_of(&TestStruct::c);
        benchmark::DoNotOptimize(off);
        benchmark::DoNotOptimize(sz);
        benchmark::DoNotOptimize(al);
    }
}

static void bm_reflect_is_same_type(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        int    a = 1;
        double b = 2.0;
        char   c = 'x';
        state.ResumeTiming();

        bool r1 = hj::reflect::is_same_type(a, int(2));
        bool r2 = hj::reflect::is_same_type(a, b);
        bool r3 = hj::reflect::is_same_type(a, int(2), int(3), int(4));
        benchmark::DoNotOptimize(r1);
        benchmark::DoNotOptimize(r2);
        benchmark::DoNotOptimize(r3);
    }
}

} // namespace

BENCHMARK(bm_reflect_type_name)->Iterations(2000);
BENCHMARK(bm_reflect_is_pod)->Iterations(2000);
BENCHMARK(bm_reflect_copy_clone)->Iterations(2000);
BENCHMARK(bm_reflect_serialize_unserialize)->Iterations(2000);
BENCHMARK(bm_reflect_offset_size_align_of)->Iterations(5000);
BENCHMARK(bm_reflect_is_same_type)->Iterations(2000);
