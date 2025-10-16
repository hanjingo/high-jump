#include <benchmark/benchmark.h>
#include <hj/util/init.hpp>
#include <vector>
#include <memory>
#include <string>
#include <atomic>
#include <random>
#include <functional>
#include <chrono>
#include <stdexcept>
#include <map>
#include <cstdlib>
#include <cstdint>

// Test basic init object creation and destruction
static void bm_init_empty(benchmark::State &state)
{
    for(auto _ : state)
    {
        // Test creating an empty initializer
        hj::init init_obj([]() {});
        benchmark::DoNotOptimize(init_obj);
    }
}
BENCHMARK(bm_init_empty);

// Test init with simple operation
static void bm_init_simple_operation(benchmark::State &state)
{
    static std::atomic<int> counter{0};

    for(auto _ : state)
    {
        auto    *counter_ptr = &counter;
        hj::init init_obj([counter_ptr]() {
            counter_ptr->fetch_add(1, std::memory_order_relaxed);
        });
        benchmark::DoNotOptimize(counter);
        benchmark::DoNotOptimize(init_obj);
    }
}
BENCHMARK(bm_init_simple_operation);

// Test init with complex computation
static void bm_init_complex_computation(benchmark::State &state)
{
    static std::vector<int> results;
    results.reserve(1000);

    for(auto _ : state)
    {
        std::vector<int> *results_ptr = &results;
        hj::init          init_obj([results_ptr]() {
            // Simulate some computation
            int sum = 0;
            for(int i = 0; i < 100; ++i)
            {
                sum += i * i;
            }
            results_ptr->push_back(sum);

            // Keep vector size manageable
            if(results_ptr->size() > 500)
            {
                results_ptr->erase(results_ptr->begin(),
                                   results_ptr->begin() + 100);
            }
        });
        benchmark::DoNotOptimize(results);
        benchmark::DoNotOptimize(init_obj);
    }
}
BENCHMARK(bm_init_complex_computation);

// Test init with exception handling
static void bm_init_exception_handling(benchmark::State &state)
{
    static std::atomic<int> exception_count{0};

    for(auto _ : state)
    {
        std::atomic<int> *exception_count_ptr = &exception_count;
        hj::init          init_obj([exception_count_ptr]() {
            try
            {
                // Simulate potential exception
                if(exception_count_ptr->load() % 1000 == 0)
                {
                    throw std::runtime_error("Test exception");
                }
                exception_count_ptr->fetch_add(1, std::memory_order_relaxed);
            }
            catch(...)
            {
                // Exception caught and handled
            }
        });
        benchmark::DoNotOptimize(exception_count);
        benchmark::DoNotOptimize(init_obj);
    }
}
BENCHMARK(bm_init_exception_handling);

// Test batch creation of init objects
static void bm_init_batch_creation(benchmark::State &state)
{
    const int               batch_size = static_cast<int>(state.range(0));
    static std::atomic<int> counter{0};

    for(auto _ : state)
    {
        std::vector<std::unique_ptr<hj::init>> init_objects;
        init_objects.reserve(batch_size);

        std::atomic<int> *counter_ptr = &counter;
        for(int i = 0; i < batch_size; ++i)
        {
            init_objects.push_back(
                std::make_unique<hj::init>([counter_ptr, i]() {
                    counter_ptr->fetch_add(i, std::memory_order_relaxed);
                }));
        }

        benchmark::DoNotOptimize(counter);
        benchmark::DoNotOptimize(init_objects);
    }
}
BENCHMARK(bm_init_batch_creation)->Arg(10)->Arg(50)->Arg(100)->Arg(500);

// Test init with memory allocation
static void bm_init_memory_allocation(benchmark::State &state)
{
    static std::vector<std::unique_ptr<int>> allocated_memory;

    for(auto _ : state)
    {
        std::vector<std::unique_ptr<int>> *allocated_memory_ptr =
            &allocated_memory;
        hj::init init_obj([allocated_memory_ptr]() {
            allocated_memory_ptr->push_back(std::make_unique<int>(42));

            // Keep memory usage reasonable
            if(allocated_memory_ptr->size() > 200)
            {
                allocated_memory_ptr->erase(allocated_memory_ptr->begin(),
                                            allocated_memory_ptr->begin() + 50);
            }
        });

        benchmark::DoNotOptimize(allocated_memory);
        benchmark::DoNotOptimize(init_obj);
    }
}
BENCHMARK(bm_init_memory_allocation);

// Test init with string operations
static void bm_init_string_operations(benchmark::State &state)
{
    static std::vector<std::string> string_results;
    string_results.reserve(500);

    for(auto _ : state)
    {
        std::vector<std::string> *string_results_ptr = &string_results;
        int64_t                   iteration_count    = state.iterations();
        hj::init init_obj([string_results_ptr, iteration_count]() {
            std::string result =
                "benchmark_" + std::to_string(iteration_count) + "_result";
            string_results_ptr->push_back(std::move(result));

            // Keep vector size manageable
            if(string_results_ptr->size() > 300)
            {
                string_results_ptr->erase(string_results_ptr->begin(),
                                          string_results_ptr->begin() + 100);
            }
        });

        benchmark::DoNotOptimize(string_results);
        benchmark::DoNotOptimize(init_obj);
    }
}
BENCHMARK(bm_init_string_operations);

// Test init with timing measurements
static void bm_init_timing_overhead(benchmark::State &state)
{
    static std::vector<std::chrono::nanoseconds> timing_results;

    for(auto _ : state)
    {
        std::chrono::high_resolution_clock::time_point start =
            std::chrono::high_resolution_clock::now();
        std::vector<std::chrono::nanoseconds> *timing_results_ptr =
            &timing_results;

        hj::init init_obj([timing_results_ptr, start]() {
            std::chrono::high_resolution_clock::time_point end =
                std::chrono::high_resolution_clock::now();
            std::chrono::nanoseconds duration =
                std::chrono::duration_cast<std::chrono::nanoseconds>(end
                                                                     - start);
            timing_results_ptr->push_back(duration);

            // Keep vector size manageable
            if(timing_results_ptr->size() > 200)
            {
                timing_results_ptr->erase(timing_results_ptr->begin(),
                                          timing_results_ptr->begin() + 50);
            }
        });

        benchmark::DoNotOptimize(timing_results);
        benchmark::DoNotOptimize(init_obj);
    }
}
BENCHMARK(bm_init_timing_overhead);

// Test init with complex data structures
static void bm_init_complex_data_structures(benchmark::State &state)
{
    static std::vector<std::map<std::string, int>> complex_data;
    static std::atomic<int>                        complex_counter{0};

    for(auto _ : state)
    {
        std::vector<std::map<std::string, int>> *complex_data_ptr =
            &complex_data;
        std::atomic<int> *complex_counter_ptr = &complex_counter;
        hj::init          init_obj([complex_data_ptr, complex_counter_ptr]() {
            std::map<std::string, int> data_map;
            int                        current_count =
                complex_counter_ptr->fetch_add(1, std::memory_order_relaxed);

            data_map["iteration"] = current_count;
            data_map["timestamp"] = static_cast<int>(
                std::chrono::steady_clock::now().time_since_epoch().count());
            data_map["random"] = std::rand() % 1000;

            complex_data_ptr->push_back(std::move(data_map));

            // Keep memory usage reasonable
            if(complex_data_ptr->size() > 100)
            {
                complex_data_ptr->erase(complex_data_ptr->begin(),
                                        complex_data_ptr->begin() + 25);
            }
        });

        benchmark::DoNotOptimize(complex_data);
        benchmark::DoNotOptimize(complex_counter);
        benchmark::DoNotOptimize(init_obj);
    }
}
BENCHMARK(bm_init_complex_data_structures);

// Static variable for macro test (must be at file scope)
static std::atomic<int> global_macro_counter{0};

// Test INIT macro performance
static void bm_init_macro_usage(benchmark::State &state)
{
    for(auto _ : state)
    {
        // Since INIT macro creates a namespace, we need to use it differently
        // We'll test the overhead by creating init objects manually instead
        hj::init init_obj([]() {
            global_macro_counter.fetch_add(1, std::memory_order_relaxed);
        });

        benchmark::DoNotOptimize(global_macro_counter);
        benchmark::DoNotOptimize(init_obj);
    }
}
BENCHMARK(bm_init_macro_usage);

// Test comparison between manual initialization and init class
static void bm_init_vs_manual(benchmark::State &state)
{
    static std::atomic<int> counter{0};

    for(auto _ : state)
    {
        if(state.range(0) == 0)
        {
            // Manual initialization
            static bool initialized = false;
            if(!initialized)
            {
                counter.fetch_add(1, std::memory_order_relaxed);
                initialized = true;
            }
        } else
        {
            // Using hj::init
            std::atomic<int> *counter_ptr = &counter;
            hj::init          init_obj([counter_ptr]() {
                counter_ptr->fetch_add(1, std::memory_order_relaxed);
            });
            benchmark::DoNotOptimize(init_obj);
        }

        benchmark::DoNotOptimize(counter);
    }
}
BENCHMARK(bm_init_vs_manual)->Arg(0)->Arg(1);

// Test random number generation in init
static void bm_init_random_operations(benchmark::State &state)
{
    const int               num_operations = static_cast<int>(state.range(0));
    static std::vector<int> random_results;

    for(auto _ : state)
    {
        std::random_device              rd;
        std::mt19937                    gen(rd());
        std::uniform_int_distribution<> dis(1, 1000);

        std::vector<int> *random_results_ptr = &random_results;
        for(int i = 0; i < num_operations; ++i)
        {
            int      random_val = dis(gen);
            hj::init init_obj([random_results_ptr, random_val]() {
                random_results_ptr->push_back(random_val * 2);

                // Keep vector size manageable
                if(random_results_ptr->size() > 500)
                {
                    random_results_ptr->erase(random_results_ptr->begin(),
                                              random_results_ptr->begin()
                                                  + 100);
                }
            });
            benchmark::DoNotOptimize(init_obj);
        }

        benchmark::DoNotOptimize(random_results);
    }
}
BENCHMARK(bm_init_random_operations)->Arg(5)->Arg(20)->Arg(50)->Arg(100);

// Test basic overhead of init objects
static void bm_init_overhead_comparison(benchmark::State &state)
{
    static std::atomic<int> counter{0};

    for(auto _ : state)
    {
        // Direct function call
        auto direct_start = std::chrono::high_resolution_clock::now();
        counter.fetch_add(1, std::memory_order_relaxed);
        auto direct_end = std::chrono::high_resolution_clock::now();

        // Using init object
        std::chrono::high_resolution_clock::time_point init_start =
            std::chrono::high_resolution_clock::now();
        std::atomic<int> *counter_ptr = &counter;
        hj::init          init_obj([counter_ptr]() {
            counter_ptr->fetch_add(1, std::memory_order_relaxed);
        });
        std::chrono::high_resolution_clock::time_point init_end =
            std::chrono::high_resolution_clock::now();

        benchmark::DoNotOptimize(counter);
        benchmark::DoNotOptimize(init_obj);
        benchmark::DoNotOptimize(direct_start);
        benchmark::DoNotOptimize(direct_end);
        benchmark::DoNotOptimize(init_start);
        benchmark::DoNotOptimize(init_end);
    }
}
BENCHMARK(bm_init_overhead_comparison);

// Test multiple init objects in sequence
static void bm_init_multiple_sequence(benchmark::State &state)
{
    static std::vector<int> sequence_results;

    for(auto _ : state)
    {
        std::vector<std::unique_ptr<hj::init>> init_sequence;

        std::vector<int> *sequence_results_ptr = &sequence_results;
        for(int i = 0; i < 10; ++i)
        {
            init_sequence.push_back(
                std::make_unique<hj::init>([sequence_results_ptr, i]() {
                    sequence_results_ptr->push_back(i * i);
                }));
        }

        // Keep results vector size manageable
        if(sequence_results.size() > 1000)
        {
            sequence_results.erase(sequence_results.begin(),
                                   sequence_results.begin() + 500);
        }

        benchmark::DoNotOptimize(sequence_results);
        benchmark::DoNotOptimize(init_sequence);
    }
}
BENCHMARK(bm_init_multiple_sequence);