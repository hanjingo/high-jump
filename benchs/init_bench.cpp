#include <benchmark/benchmark.h>
#include <hj/util/init.hpp>
#include <vector>
#include <memory>
#include <string>
#include <atomic>
#include <random>

static void bm_init_empty(benchmark::State &state)
{
    for(auto _ : state)
    {
        // Test creating an empty initializer
        auto init_obj = hj::init::make_initializer([]() {}, 0, "empty_test");
        benchmark::DoNotOptimize(init_obj);
    }
}
BENCHMARK(bm_init_empty);

static void bm_init_simple_operation(benchmark::State &state)
{
    static std::atomic<int> counter{0};

    for(auto _ : state)
    {
        auto init_obj = hj::init::make_initializer(
            [counter_ptr = &counter]() { counter_ptr->fetch_add(1); },
            0,
            "simple_op");
        benchmark::DoNotOptimize(counter);
        benchmark::DoNotOptimize(init_obj);
    }
}
BENCHMARK(bm_init_simple_operation);

static void bm_init_with_priority(benchmark::State &state)
{
    static std::atomic<int> counter{0};

    for(auto _ : state)
    {
        auto init_obj = hj::init::make_initializer(
            [counter_ptr = &counter]() { counter_ptr->fetch_add(1); },
            100,
            "priority_test");
        benchmark::DoNotOptimize(counter);
        benchmark::DoNotOptimize(init_obj);
    }
}
BENCHMARK(bm_init_with_priority);

static void bm_init_named(benchmark::State &state)
{
    static std::atomic<int> counter{0};

    for(auto _ : state)
    {
        std::string name     = "bench_init_" + std::to_string(counter.load());
        auto        init_obj = hj::init::make_initializer(
            [counter_ptr = &counter]() { counter_ptr->fetch_add(1); },
            0,
            name.c_str());
        benchmark::DoNotOptimize(counter);
        benchmark::DoNotOptimize(name);
        benchmark::DoNotOptimize(init_obj);
    }
}
BENCHMARK(bm_init_named);

static void bm_init_full(benchmark::State &state)
{
    static std::atomic<int> counter{0};

    for(auto _ : state)
    {
        std::string name     = "full_init_" + std::to_string(counter.load());
        auto        init_obj = hj::init::make_initializer(
            [counter_ptr = &counter]() { counter_ptr->fetch_add(1); },
            50,
            name.c_str());
        benchmark::DoNotOptimize(counter);
        benchmark::DoNotOptimize(name);
        benchmark::DoNotOptimize(init_obj);
    }
}
BENCHMARK(bm_init_full);

static void bm_make_initializer_factory(benchmark::State &state)
{
    static std::atomic<int> counter{0};

    for(auto _ : state)
    {
        auto init_obj = hj::init::make_initializer(
            [counter_ptr = &counter]() { counter_ptr->fetch_add(1); },
            0,
            "factory_test");

        benchmark::DoNotOptimize(counter);
        benchmark::DoNotOptimize(init_obj);
    }
}
BENCHMARK(bm_make_initializer_factory);

static void bm_batch_initializer_registration(benchmark::State &state)
{
    const int               batch_size = static_cast<int>(state.range(0));
    static std::atomic<int> counter{0};

    for(auto _ : state)
    {
        std::vector<std::atomic<int> *> counters;
        counters.reserve(batch_size);

        for(int i = 0; i < batch_size; ++i)
        {
            counters.push_back(&counter);
            auto init_obj = hj::init::make_initializer(
                [counter_ptr = &counter]() { counter_ptr->fetch_add(1); },
                i,
                "batch_" + std::to_string(i));

            benchmark::DoNotOptimize(init_obj);
        }

        benchmark::DoNotOptimize(counters);
    }
}
BENCHMARK(bm_batch_initializer_registration)
    ->Arg(10)
    ->Arg(50)
    ->Arg(100)
    ->Arg(500);

static void bm_execute_initializers(benchmark::State &state)
{
    static bool initialized = false;
    if(!initialized)
    {
        for(int i = 0; i < 100; ++i)
        {
            auto init_obj = hj::init::make_initializer(
                [i]() {
                    volatile int dummy = i * 2;
                    (void) dummy;
                },
                i,
                "exec_test_" + std::to_string(i));
        }
        initialized = true;
    }

    for(auto _ : state)
    {
        hj::init::execute_initializers();
    }
}
BENCHMARK(bm_execute_initializers);

static void bm_get_initialization_results(benchmark::State &state)
{
    static bool setup_done = false;
    if(!setup_done)
    {
        for(int i = 0; i < 50; ++i)
        {
            auto init_obj = hj::init::make_initializer(
                [i]() {
                    volatile int dummy = i;
                    (void) dummy;
                },
                0,
                "result_test_" + std::to_string(i));
        }
        hj::init::execute_initializers();
        setup_done = true;
    }

    for(auto _ : state)
    {
        auto results = hj::init::get_initialization_results();
        benchmark::DoNotOptimize(results);
    }
}
BENCHMARK(bm_get_initialization_results);

static void bm_has_initialization_errors(benchmark::State &state)
{
    static bool setup_done = false;
    if(!setup_done)
    {
        for(int i = 0; i < 20; ++i)
        {
            auto init_obj = hj::init::make_initializer(
                [i]() {
                    volatile int dummy = i;
                    (void) dummy;
                },
                0,
                "success_" + std::to_string(i));
        }

        auto failing_init = hj::init::make_initializer(
            []() { throw std::runtime_error("Benchmark test exception"); },
            0,
            "failing_init");

        hj::init::execute_initializers();
        setup_done = true;
    }

    for(auto _ : state)
    {
        bool has_errors = hj::init::has_initialization_errors();
        benchmark::DoNotOptimize(has_errors);
    }
}
BENCHMARK(bm_has_initialization_errors);

static void bm_complex_initialization_scenario(benchmark::State &state)
{
    static std::vector<std::string> complex_data;
    static std::atomic<int>         complex_counter{0};

    for(auto _ : state)
    {
        auto init1 = hj::init::make_initializer(
            [data_ptr = &complex_data]() {
                data_ptr->push_back("high_priority");
            },
            100,
            "high_priority");

        auto init2 = hj::init::make_initializer(
            [counter_ptr = &complex_counter, data_ptr = &complex_data]() {
                counter_ptr->fetch_add(1);
                data_ptr->push_back("named_init");
            },
            0,
            "complex_named");

        auto init3 = hj::init::make_initializer(
            [counter_ptr = &complex_counter, data_ptr = &complex_data]() {
                auto value = counter_ptr->load();
                data_ptr->push_back("full_" + std::to_string(value));
            },
            50,
            "complex_full");

        benchmark::DoNotOptimize(complex_data);
        benchmark::DoNotOptimize(complex_counter);
        benchmark::DoNotOptimize(init1);
        benchmark::DoNotOptimize(init2);
        benchmark::DoNotOptimize(init3);

        if(complex_data.size() > 1000)
        {
            complex_data.clear();
        }
    }
}
BENCHMARK(bm_complex_initialization_scenario);

static void bm_exception_handling_overhead(benchmark::State &state)
{
    for(auto _ : state)
    {
        try
        {
            auto failing_init = hj::init::make_initializer(
                []() { throw std::runtime_error("Benchmark exception"); },
                0,
                "exception_test");

            hj::init::execute_initializers();

            benchmark::DoNotOptimize(failing_init);
        }
        catch(...)
        {
        }
    }
}
BENCHMARK(bm_exception_handling_overhead);

static void bm_string_operations_init(benchmark::State &state)
{
    static std::vector<std::string> string_results;
    string_results.reserve(1000);

    for(auto _ : state)
    {
        std::string base   = "benchmark_string_";
        std::string suffix = std::to_string(state.iterations());

        auto init_obj = hj::init::make_initializer(
            [results_ptr = &string_results, base, suffix]() {
                results_ptr->push_back(base + suffix);
                if(results_ptr->size() > 100)
                {
                    results_ptr->erase(results_ptr->begin(),
                                       results_ptr->begin() + 50);
                }
            },
            0,
            "string_ops");

        benchmark::DoNotOptimize(base);
        benchmark::DoNotOptimize(suffix);
        benchmark::DoNotOptimize(string_results);
        benchmark::DoNotOptimize(init_obj);
    }
}
BENCHMARK(bm_string_operations_init);

static void bm_memory_allocation_init(benchmark::State &state)
{
    static std::vector<std::unique_ptr<int>> managed_ptrs;

    for(auto _ : state)
    {
        auto init_obj = hj::init::make_initializer(
            [ptrs = &managed_ptrs]() {
                ptrs->push_back(std::make_unique<int>(42));
                if(ptrs->size() > 100)
                {
                    ptrs->erase(ptrs->begin(), ptrs->begin() + 50);
                }
            },
            0,
            "memory_alloc");

        benchmark::DoNotOptimize(managed_ptrs);
        benchmark::DoNotOptimize(init_obj);
    }
}
BENCHMARK(bm_memory_allocation_init);

static void bm_priority_sorting_overhead(benchmark::State &state)
{
    const int               num_priorities = static_cast<int>(state.range(0));
    static std::atomic<int> counter{0};

    for(auto _ : state)
    {
        std::random_device              rd;
        std::mt19937                    gen(rd());
        std::uniform_int_distribution<> dis(1, 1000);

        for(int i = 0; i < num_priorities; ++i)
        {
            int  priority = dis(gen);
            auto init_obj = hj::init::make_initializer(
                [counter_ptr = &counter]() { counter_ptr->fetch_add(1); },
                priority,
                "priority_test_" + std::to_string(i));

            benchmark::DoNotOptimize(init_obj);
        }

        benchmark::DoNotOptimize(counter);
    }
}
BENCHMARK(bm_priority_sorting_overhead)->Arg(5)->Arg(20)->Arg(50)->Arg(100);

static void bm_manual_initialization(benchmark::State &state)
{
    static std::atomic<int> counter{0};

    for(auto _ : state)
    {
        static bool initialized = false;
        if(!initialized)
        {
            counter.fetch_add(1);
            initialized = true;
        }

        benchmark::DoNotOptimize(counter);
    }
}
BENCHMARK(bm_manual_initialization);

static void bm_init_macro_vs_manual(benchmark::State &state)
{
    static std::atomic<int> counter{0};

    for(auto _ : state)
    {
        auto init_obj = hj::init::make_initializer(
            [counter_ptr = &counter]() { counter_ptr->fetch_add(1); },
            0,
            "macro_vs_manual");
        benchmark::DoNotOptimize(counter);
        benchmark::DoNotOptimize(init_obj);
    }
}
BENCHMARK(bm_init_macro_vs_manual);

static void bm_thread_safety_overhead(benchmark::State &state)
{
    static std::vector<std::atomic<int>> thread_counters(10);

    for(auto _ : state)
    {
        for(int i = 0; i < 10; ++i)
        {
            auto init_obj = hj::init::make_initializer(
                [i, counters_ptr = &thread_counters]() {
                    (*counters_ptr)[i].fetch_add(1);
                },
                0,
                "thread_test_" + std::to_string(i));

            benchmark::DoNotOptimize(init_obj);
        }

        benchmark::DoNotOptimize(thread_counters);
    }
}
BENCHMARK(bm_thread_safety_overhead);