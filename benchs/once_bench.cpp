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

// ONCE with return value simulation
static void bm_once_with_result_simple(benchmark::State &state)
{
    static int result = 0;
    static bool initialized = false;

    for(auto _ : state)
    {
        ONCE(result = 42; initialized = true;);
        if(initialized)
        {
            benchmark::DoNotOptimize(result);
        }
    }
}
BENCHMARK(bm_once_with_result_simple);

static int complex_calculation()
{
    int sum = 0;
    for(int i = 0; i < 100; ++i)
    {
        sum += i * i;
    }
    return sum;
}

static void bm_once_complex_calculation(benchmark::State &state)
{
    static int result = 0;

    for(auto _ : state)
    {
        ONCE(result = complex_calculation(););
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(bm_once_complex_calculation);

static void bm_once_string_operation(benchmark::State &state)
{
    static std::string result;

    for(auto _ : state)
    {
        ONCE(result = std::string("Hello, World! ") + std::to_string(42););
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(bm_once_string_operation);

// Multiple ONCE operations
static void bm_once_multiple_operations(benchmark::State &state)
{
    static int counter1 = 0, counter2 = 0, counter3 = 0;

    for(auto _ : state)
    {
        ONCE(counter1++;);
        ONCE(counter2 += 2;);
        ONCE(counter3 += 3;);
        benchmark::DoNotOptimize(counter1);
        benchmark::DoNotOptimize(counter2);
        benchmark::DoNotOptimize(counter3);
    }
}
BENCHMARK(bm_once_multiple_operations);

// ONCE with exception handling (built into the macro)
static void bm_once_with_exception(benchmark::State &state)
{
    static int counter = 0;
    static bool exception_thrown = false;

    for(auto _ : state)
    {
        ONCE(
            counter++;
            if(!exception_thrown) {
                exception_thrown = true;
                throw std::runtime_error("test error"); // This will be caught by ONCE macro
            }
        );
        benchmark::DoNotOptimize(counter);
    }
}
BENCHMARK(bm_once_with_exception);

// Simulated executor pattern with ONCE
static void bm_once_function_execution(benchmark::State &state)
{
    static int result = 0;

    for(auto _ : state)
    {
        ONCE(result = 42;);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(bm_once_function_execution);

static void bm_once_void_execution(benchmark::State &state)
{
    static int counter = 0;

    for(auto _ : state)
    {
        ONCE(counter++;);
        benchmark::DoNotOptimize(counter);
    }
}
BENCHMARK(bm_once_void_execution);

// Multiple ONCE operations with different static variables
static void bm_once_multiple_functions(benchmark::State &state)
{
    const int count = static_cast<int>(state.range(0));

    for(auto _ : state)
    {
        for(int i = 0; i < count; ++i)
        {
            if(i == 0) {
                static int result0 = 0;
                ONCE(result0 = i * i;);
                benchmark::DoNotOptimize(result0);
            } else if(i == 1) {
                static int result1 = 0;
                ONCE(result1 = i * i;);
                benchmark::DoNotOptimize(result1);
            } else if(i == 2) {
                static int result2 = 0;
                ONCE(result2 = i * i;);
                benchmark::DoNotOptimize(result2);
            }
            // Note: For larger counts, this becomes impractical but demonstrates concept
        }
    }
}
BENCHMARK(bm_once_multiple_functions)->Arg(1)->Arg(2)->Arg(3);

static void bm_once_complex_lambda(benchmark::State &state)
{
    static int result = 0;

    for(auto _ : state)
    {
        ONCE(
            std::vector<int> vec(100);
            std::iota(vec.begin(), vec.end(), 0);
            result = std::accumulate(vec.begin(), vec.end(), 0);
        );
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(bm_once_complex_lambda);

// Memory allocation benchmarks
static void bm_once_memory_allocation(benchmark::State &state)
{
    static std::vector<std::unique_ptr<int>> storage;

    for(auto _ : state)
    {
        ONCE(
            storage.reserve(1000);
            for(int i = 0; i < 100; ++i) {
                storage.push_back(std::make_unique<int>(i));
            }
        );
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
        ONCE(
            global_string.reserve(10000);
            for(int i = 0; i < 100; ++i) {
                global_string += "Hello World " + std::to_string(i) + " ";
            }
        );
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
static void bm_once_exception_handling(benchmark::State &state)
{
    static int result = 0;
    static bool exception_handled = false;

    for(auto _ : state)
    {
        ONCE(
            if(!exception_handled) {
                exception_handled = true;
                throw std::runtime_error("Test exception"); // Will be caught by ONCE
            }
            result = 42;
        );
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(bm_once_exception_handling);

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
        ONCE(
            counters.resize(batch_size);
            for(int i = 0; i < batch_size; ++i) { 
                counters[i] = i * 2; 
            }
        );
        benchmark::DoNotOptimize(counters);
    }
}
BENCHMARK(bm_once_batch_operations)->Arg(10)->Arg(50)->Arg(100)->Arg(500);

// Nested once operations (note: this won't work as expected since each ONCE has its own flag)
static void bm_once_nested_operations(benchmark::State &state)
{
    static int level1 = 0, level2 = 0, level3 = 0;

    for(auto _ : state)
    {
        ONCE(level1 = 1;);
        ONCE(level2 = 2;); 
        ONCE(level3 = 3;);
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
        static int result = 0;
        ONCE(result = class_counter * 2;);
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

// Large data structure initialization
static void bm_once_large_data_structure(benchmark::State &state)
{
    static std::unordered_map<int, std::string> large_map;

    for(auto _ : state)
    {
        ONCE(
            large_map.reserve(10000);
            for(int i = 0; i < 1000; ++i) {
                large_map[i] = "Value_" + std::to_string(i) + "_" + std::string(20, 'x');
            }
        );
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