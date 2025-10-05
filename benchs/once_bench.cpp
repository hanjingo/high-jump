#include <benchmark/benchmark.h>
#include <hj/util/once.hpp>
#include <vector>
#include <memory>
#include <string>
#include <atomic>
#include <thread>
#include <chrono>
#include <numeric>
#include <unordered_map>
#include <functional>

// Basic ONCE macro benchmarks
static void bm_once_macro_empty(benchmark::State &state)
{
    for(auto _ : state)
    {
        ONCE(;);
    }
}
BENCHMARK(bm_once_macro_empty);

static void bm_once_macro_simple_increment(benchmark::State &state)
{
    static int counter = 0;

    for(auto _ : state)
    {
        ONCE(counter++;);
        benchmark::DoNotOptimize(counter);
    }
}
BENCHMARK(bm_once_macro_simple_increment);

static void bm_once_macro_complex_operation(benchmark::State &state)
{
    static std::vector<int> data;

    for(auto _ : state)
    {
        ONCE(data.reserve(1000); data.push_back(42); data.push_back(84););
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(bm_once_macro_complex_operation);

// ONCE_RETURN macro benchmarks
static void bm_once_return_simple(benchmark::State &state)
{
    int result;

    for(auto _ : state)
    {
        ONCE_RETURN(result, 42);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(bm_once_return_simple);

static int complex_calculation()
{
    int sum = 0;
    for(int i = 0; i < 100; ++i)
    {
        sum += i * i;
    }
    return sum;
}

static void bm_once_return_complex_calculation(benchmark::State &state)
{
    int result;

    for(auto _ : state)
    {
        ONCE_RETURN(result, complex_calculation());
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(bm_once_return_complex_calculation);

static void bm_once_return_string(benchmark::State &state)
{
    std::string result;

    for(auto _ : state)
    {
        ONCE_RETURN(result, std::string("Hello, World! ") + std::to_string(42));
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(bm_once_return_string);

// ONCE_NAMED macro benchmarks
static void bm_once_named_single(benchmark::State &state)
{
    static int counter = 0;

    for(auto _ : state)
    {
        ONCE_NAMED("benchmark_counter", counter++;);
        benchmark::DoNotOptimize(counter);
    }
}
BENCHMARK(bm_once_named_single);

static void bm_once_named_multiple(benchmark::State &state)
{
    static int counter1 = 0, counter2 = 0, counter3 = 0;

    for(auto _ : state)
    {
        ONCE_NAMED("counter1", counter1++;);
        ONCE_NAMED("counter2", counter2 += 2;);
        ONCE_NAMED("counter3", counter3 += 3;);
        benchmark::DoNotOptimize(counter1);
        benchmark::DoNotOptimize(counter2);
        benchmark::DoNotOptimize(counter3);
    }
}
BENCHMARK(bm_once_named_multiple);

// ONCE_SAFE macro benchmarks
static void bm_once_safe_no_exception(benchmark::State &state)
{
    static int counter = 0;

    for(auto _ : state)
    {
        ONCE_SAFE(counter++;, [](const std::string &) {});
        benchmark::DoNotOptimize(counter);
    }
}
BENCHMARK(bm_once_safe_no_exception);

static void bm_once_safe_with_exception(benchmark::State &state)
{
    static int         counter = 0;
    static std::string error_msg;

    for(auto _ : state)
    {
        ONCE_SAFE(counter++;
                  if(counter == 1) throw std::runtime_error("test error");
                  , [](const std::string &msg) {
                      static std::string *local_error_msg = &error_msg;
                      *local_error_msg                    = msg;
                  });
        benchmark::DoNotOptimize(counter);
        benchmark::DoNotOptimize(error_msg);
    }
}
BENCHMARK(bm_once_safe_with_exception);

// once_executor benchmarks
static void bm_once_executor_creation(benchmark::State &state)
{
    for(auto _ : state)
    {
        auto executor =
            hj::make_once_executor<std::function<int()>>("test_executor");
        benchmark::DoNotOptimize(executor);
    }
}
BENCHMARK(bm_once_executor_creation);

static void bm_once_executor_execution(benchmark::State &state)
{
    auto executor =
        hj::make_once_executor<std::function<int()>>("test_executor");

    for(auto _ : state)
    {
        auto result = executor.execute([]() { return 42; });
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(bm_once_executor_execution);

static void bm_once_executor_void_execution(benchmark::State &state)
{
    auto executor =
        hj::make_once_executor<std::function<void()>>("void_executor");
    static int counter = 0;

    for(auto _ : state)
    {
        auto result = executor.execute([&]() { counter++; });
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(counter);
    }
}
BENCHMARK(bm_once_executor_void_execution);

// execute_once_named benchmarks
static void bm_execute_once_named_simple(benchmark::State &state)
{
    for(auto _ : state)
    {
        auto result =
            hj::execute_once_named("simple_test", []() { return 42; });
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(bm_execute_once_named_simple);

static void bm_execute_once_named_multiple_names(benchmark::State &state)
{
    const int name_count = static_cast<int>(state.range(0));

    for(auto _ : state)
    {
        for(int i = 0; i < name_count; ++i)
        {
            std::string name = "test_" + std::to_string(i);
            auto result = hj::execute_once_named(name, [i]() { return i * i; });
            benchmark::DoNotOptimize(result);
        }
    }
}
BENCHMARK(bm_execute_once_named_multiple_names)
    ->Arg(1)
    ->Arg(10)
    ->Arg(50)
    ->Arg(100);

// execute_once benchmarks
static void bm_execute_once_function(benchmark::State &state)
{
    for(auto _ : state)
    {
        auto result = hj::execute_once([]() { return 42; });
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(bm_execute_once_function);

static void bm_execute_once_complex_lambda(benchmark::State &state)
{
    for(auto _ : state)
    {
        auto result = hj::execute_once([]() {
            std::vector<int> vec(100);
            std::iota(vec.begin(), vec.end(), 0);
            return std::accumulate(vec.begin(), vec.end(), 0);
        });
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(bm_execute_once_complex_lambda);

// Memory allocation benchmarks
static void bm_once_memory_allocation(benchmark::State &state)
{
    static std::vector<std::unique_ptr<int>> storage;

    for(auto _ : state)
    {
        ONCE(storage.reserve(1000); for(int i = 0; i < 100; ++i) {
            storage.push_back(std::make_unique<int>(i));
        });
        benchmark::DoNotOptimize(storage);
    }
}
BENCHMARK(bm_once_memory_allocation);

// String operations benchmarks
static void bm_once_string_operations(benchmark::State &state)
{
    static std::string global_string;

    for(auto _ : state)
    {
        ONCE(global_string.reserve(10000); for(int i = 0; i < 100; ++i) {
            global_string += "Hello World " + std::to_string(i) + " ";
        });
        benchmark::DoNotOptimize(global_string);
    }
}
BENCHMARK(bm_once_string_operations);

// Thread safety simulation (single thread benchmarks)
static void bm_once_atomic_operations(benchmark::State &state)
{
    static std::atomic<int> atomic_counter{0};

    for(auto _ : state)
    {
        ONCE(atomic_counter.store(42, std::memory_order_release););
        benchmark::DoNotOptimize(
            atomic_counter.load(std::memory_order_acquire));
    }
}
BENCHMARK(bm_once_atomic_operations);

// Exception handling benchmarks
static void bm_once_executor_exception_handling(benchmark::State &state)
{
    auto executor =
        hj::make_once_executor<std::function<int()>>("exception_executor");

    for(auto _ : state)
    {
        auto result = executor.execute([]() -> int {
            static bool first_call = true;
            if(first_call)
            {
                first_call = false;
                throw std::runtime_error("Test exception");
            }
            return 42;
        });
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(bm_once_executor_exception_handling);

// Comparison with std::call_once
static void bm_std_call_once_simple(benchmark::State &state)
{
    static std::once_flag flag;
    static int            counter = 0;

    for(auto _ : state)
    {
        std::call_once(flag, [&]() { counter++; });
        benchmark::DoNotOptimize(counter);
    }
}
BENCHMARK(bm_std_call_once_simple);

static void bm_std_call_once_complex(benchmark::State &state)
{
    static std::once_flag   flag;
    static std::vector<int> data;

    for(auto _ : state)
    {
        std::call_once(flag, []() {
            data.reserve(1000);
            for(int i = 0; i < 100; ++i)
            {
                data.push_back(i * i);
            }
        });
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(bm_std_call_once_complex);

// Batch operations benchmarks
static void bm_once_batch_operations(benchmark::State &state)
{
    const int               batch_size = static_cast<int>(state.range(0));
    static std::vector<int> counters;

    for(auto _ : state)
    {
        ONCE(counters.resize(batch_size);
             for(int i = 0; i < batch_size; ++i) { counters[i] = i * 2; });
        benchmark::DoNotOptimize(counters);
    }
}
BENCHMARK(bm_once_batch_operations)->Arg(10)->Arg(50)->Arg(100)->Arg(500);

// Nested once operations
static void bm_once_nested_operations(benchmark::State &state)
{
    static int level1 = 0, level2 = 0, level3 = 0;

    for(auto _ : state)
    {
        ONCE(level1 = 1; ONCE(level2 = 2; ONCE(level3 = 3;);););
        benchmark::DoNotOptimize(level1);
        benchmark::DoNotOptimize(level2);
        benchmark::DoNotOptimize(level3);
    }
}
BENCHMARK(bm_once_nested_operations);

// Performance comparison: overhead measurement
static void bm_no_once_simple_operation(benchmark::State &state)
{
    static bool executed = false;
    static int  counter  = 0;

    for(auto _ : state)
    {
        if(!executed)
        {
            counter++;
            executed = true;
        }
        benchmark::DoNotOptimize(counter);
    }
}
BENCHMARK(bm_no_once_simple_operation);

static void bm_once_macro_overhead(benchmark::State &state)
{
    static int counter = 0;

    for(auto _ : state)
    {
        ONCE(counter++;);
        benchmark::DoNotOptimize(counter);
    }
}
BENCHMARK(bm_once_macro_overhead);

// Class-based once operations
class OnceTestClass
{
  public:
    static int class_counter;

    void initialize_once() { ONCE(class_counter = 100;); }

    int get_once_value()
    {
        int result;
        ONCE_RETURN(result, class_counter * 2);
        return result;
    }
};

int OnceTestClass::class_counter = 0;

static void bm_once_class_method(benchmark::State &state)
{
    OnceTestClass obj;

    for(auto _ : state)
    {
        obj.initialize_once();
        int value = obj.get_once_value();
        benchmark::DoNotOptimize(value);
    }
}
BENCHMARK(bm_once_class_method);

// Manager operations
static void bm_once_manager_operations(benchmark::State &state)
{
    for(auto _ : state)
    {
        auto names = hj::get_once_names();
        benchmark::DoNotOptimize(names);
    }
}
BENCHMARK(bm_once_manager_operations);

// Large data structure initialization
static void bm_once_large_data_structure(benchmark::State &state)
{
    static std::unordered_map<int, std::string> large_map;

    for(auto _ : state)
    {
        ONCE(large_map.reserve(10000); for(int i = 0; i < 1000; ++i) {
            large_map[i] =
                "Value_" + std::to_string(i) + "_" + std::string(20, 'x');
        });
        benchmark::DoNotOptimize(large_map);
    }
}
BENCHMARK(bm_once_large_data_structure);

// Function pointer operations
static void once_function_pointer_test()
{
    static int counter = 0;
    ONCE(counter = 999;);
}

static void bm_once_function_pointer(benchmark::State &state)
{
    for(auto _ : state)
    {
        once_function_pointer_test();
        benchmark::DoNotOptimize(once_function_pointer_test);
    }
}
BENCHMARK(bm_once_function_pointer);

// Template function once operations
template <typename T>
void template_once_function(T value)
{
    static T stored_value;
    ONCE(stored_value = value;);
}

static void bm_once_template_function(benchmark::State &state)
{
    for(auto _ : state)
    {
        template_once_function<int>(42);
        template_once_function<double>(3.14);
        template_once_function<std::string>("hello");
        benchmark::DoNotOptimize(template_once_function<int>);
    }
}
BENCHMARK(bm_once_template_function);