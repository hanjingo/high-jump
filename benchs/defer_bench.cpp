#include <benchmark/benchmark.h>
#include <hj/util/defer.hpp>
#include <vector>
#include <memory>
#include <string>

static void bm_defer_empty(benchmark::State &state)
{
    for(auto _ : state)
    {
        DEFER(;);
    }
}
BENCHMARK(bm_defer_empty);

static void bm_defer_simple_increment(benchmark::State &state)
{
    int counter = 0;
    for(auto _ : state)
    {
        DEFER(counter++;);
        benchmark::DoNotOptimize(counter);
    }
}
BENCHMARK(bm_defer_simple_increment);

static void bm_defer_multiple_operations(benchmark::State &state)
{
    std::vector<int> vec;
    vec.reserve(100);

    for(auto _ : state)
    {
        DEFER(vec.push_back(1););
        DEFER(vec.push_back(2););
        DEFER(vec.push_back(3););
        DEFER(vec.push_back(4););
        DEFER(vec.push_back(5););

        benchmark::DoNotOptimize(vec);
        vec.clear();
    }
}
BENCHMARK(bm_defer_multiple_operations);

static void bm_defer_complex_capture(benchmark::State &state)
{
    std::vector<std::string> vec;
    std::string              str   = "test_string";
    int                      value = 42;

    for(auto _ : state)
    {
        DEFER(vec.push_back(str + std::to_string(value)); value++;);
        benchmark::DoNotOptimize(vec);
        benchmark::DoNotOptimize(str);
        benchmark::DoNotOptimize(value);
        vec.clear();
    }
}
BENCHMARK(bm_defer_complex_capture);

static void bm_defer_nested_scopes(benchmark::State &state)
{
    std::vector<int> order;
    order.reserve(10);

    for(auto _ : state)
    {
        {
            DEFER(order.push_back(1););
            {
                DEFER(order.push_back(2););
                {
                    DEFER(order.push_back(3););
                }
            }
        }
        benchmark::DoNotOptimize(order);
        order.clear();
    }
}
BENCHMARK(bm_defer_nested_scopes);

class BenchmarkClass
{
  public:
    std::vector<int> data;

    void test_method()
    {
        DEFER_CLASS(data.push_back(1););
        DEFER_CLASS(data.push_back(2););
    }
};

static void bm_defer_class_method(benchmark::State &state)
{
    BenchmarkClass obj;
    obj.data.reserve(100);

    for(auto _ : state)
    {
        obj.test_method();
        benchmark::DoNotOptimize(obj.data);
        obj.data.clear();
    }
}
BENCHMARK(bm_defer_class_method);

static void bm_manual_cleanup(benchmark::State &state)
{
    for(auto _ : state)
    {
        std::vector<int> vec;
        vec.push_back(1);
        vec.push_back(2);
        vec.push_back(3);

        vec.clear();
        benchmark::DoNotOptimize(vec);
    }
}
BENCHMARK(bm_manual_cleanup);

static void bm_defer_cleanup(benchmark::State &state)
{
    for(auto _ : state)
    {
        std::vector<int> vec;
        DEFER(vec.clear(););

        vec.push_back(1);
        vec.push_back(2);
        vec.push_back(3);

        benchmark::DoNotOptimize(vec);
    }
}
BENCHMARK(bm_defer_cleanup);

static void bm_defer_memory_management(benchmark::State &state)
{
    for(auto _ : state)
    {
        auto ptr = std::make_unique<std::vector<int>>(100);
        DEFER(ptr.reset(););

        ptr->push_back(42);
        benchmark::DoNotOptimize(ptr);
    }
}
BENCHMARK(bm_defer_memory_management);

static void bm_defer_exception_safety(benchmark::State &state)
{
    int cleanup_counter = 0;

    for(auto _ : state)
    {
        try
        {
            DEFER(cleanup_counter++;);

            std::vector<int> vec(10);
            vec.push_back(42);

            benchmark::DoNotOptimize(vec);
            benchmark::DoNotOptimize(cleanup_counter);
        }
        catch(...)
        {
        }
    }
}
BENCHMARK(bm_defer_exception_safety);

static void bm_defer_batch_operations(benchmark::State &state)
{
    const int        batch_size = static_cast<int>(state.range(0));
    std::vector<int> counters(batch_size, 0);

    for(auto _ : state)
    {
        for(int i = 0; i < batch_size; ++i)
        {
            DEFER(counters[i]++;);
        }
        benchmark::DoNotOptimize(counters);
    }
}
BENCHMARK(bm_defer_batch_operations)->Arg(10)->Arg(50)->Arg(100)->Arg(500);

static void bm_defer_string_operations(benchmark::State &state)
{
    std::string result;
    result.reserve(1000);

    for(auto _ : state)
    {
        std::string temp = "hello";
        DEFER(result += temp + "_world"; temp.clear(););

        temp += "_modified";
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(temp);
        result.clear();
    }
}
BENCHMARK(bm_defer_string_operations);
