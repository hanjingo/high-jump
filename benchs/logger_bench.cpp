#include <benchmark/benchmark.h>
#include <hj/log/logger.hpp>
#include <filesystem>
#include <memory>
#include <thread>
#include <vector>
#include <random>
#include <string>

using namespace hj::log;

class benchmark_data_generator
{
  public:
    static std::string random_string(size_t length = 50)
    {
        const char                         charset[] = "0123456789"
                                                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                                       "abcdefghijklmnopqrstuvwxyz";
        std::random_device                 rd;
        std::mt19937                       generator(rd());
        std::uniform_int_distribution<int> distribution(0, sizeof(charset) - 2);

        std::string result;
        result.reserve(length);
        for(size_t i = 0; i < length; ++i)
        {
            result += charset[distribution(generator)];
        }
        return result;
    }

    static int random_int()
    {
        std::random_device                 rd;
        std::mt19937                       generator(rd());
        std::uniform_int_distribution<int> distribution(1, 1000000);
        return distribution(generator);
    }

    static std::string large_string() { return random_string(10000); }

    static std::string medium_string() { return random_string(1000); }
};

class logger_benchmark : public benchmark::Fixture
{
  public:
    void SetUp(const ::benchmark::State &state) override
    {
        log_inst = logger::instance();
        log_inst->add_sink(logger::create_stdout_sink());
        log_inst->set_level(level::info);
    }

    void TearDown(const ::benchmark::State &state) override
    {
        if(state.thread_index() == 0)
        {
            log_inst->clear_sink();
        }
    }

    logger *log_inst;
};

BENCHMARK_F(logger_benchmark, info_logging)(benchmark::State &state)
{
    for(auto _ : state)
    {
        log_inst->info("This is an info message with number: {}",
                       benchmark_data_generator::random_int());
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_F(logger_benchmark, warn_logging)(benchmark::State &state)
{
    for(auto _ : state)
    {
        log_inst->warn("This is a warning with data: {}",
                       benchmark_data_generator::random_string());
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_F(logger_benchmark, error_logging)(benchmark::State &state)
{
    for(auto _ : state)
    {
        log_inst->error("This is an error with code: {}",
                        benchmark_data_generator::random_int());
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_F(logger_benchmark, formatted_logging)(benchmark::State &state)
{
    for(auto _ : state)
    {
        log_inst->info(
            "User: {}, ID: {}, Score: {:.2f}, Active: {}",
            benchmark_data_generator::random_string(20),
            benchmark_data_generator::random_int(),
            static_cast<double>(benchmark_data_generator::random_int()) / 100.0,
            benchmark_data_generator::random_int() % 2 == 0);
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_F(logger_benchmark, large_message_logging)(benchmark::State &state)
{
    for(auto _ : state)
    {
        log_inst->info("Large message: {}",
                       benchmark_data_generator::large_string());
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_F(logger_benchmark, conditional_logging)(benchmark::State &state)
{
    int counter = 0;
    for(auto _ : state)
    {
        if(counter % 10 == 0)
        {
            log_inst->info("Conditional log #{}: {}",
                           counter,
                           benchmark_data_generator::random_string());
        }
        ++counter;
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_F(logger_benchmark, different_levels)(benchmark::State &state)
{
    int counter = 0;
    for(auto _ : state)
    {
        switch(counter % 4)
        {
            case 0:
                log_inst->debug("Debug message: {}",
                                benchmark_data_generator::random_int());
                break;
            case 1:
                log_inst->info("Info message: {}",
                               benchmark_data_generator::random_int());
                break;
            case 2:
                log_inst->warn("Warning message: {}",
                               benchmark_data_generator::random_int());
                break;
            case 3:
                log_inst->error("Error message: {}",
                                benchmark_data_generator::random_int());
                break;
        }
        ++counter;
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_F(logger_benchmark, set_level_performance)(benchmark::State &state)
{
    const level levels[]    = {level::trace,
                               level::debug,
                               level::info,
                               level::warning,
                               level::error,
                               level::critical};
    size_t      level_index = 0;

    for(auto _ : state)
    {
        log_inst->set_level(levels[level_index % 6]);
        ++level_index;
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_F(logger_benchmark, set_pattern)(benchmark::State &state)
{
    const char *patterns[]    = {"[%Y-%m-%d %H:%M:%S.%e] [%l] %v",
                                 "%v",
                                 "[%T] [%^%l%$] %v",
                                 "%Y-%m-%d %H:%M:%S [%l] %v"};
    size_t      pattern_index = 0;

    for(auto _ : state)
    {
        log_inst->set_pattern(patterns[pattern_index % 4]);
        ++pattern_index;
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_F(logger_benchmark, flush_performance)(benchmark::State &state)
{
    for(int i = 0; i < 100; ++i)
    {
        log_inst->info("Message before flush: {}", i);
    }

    for(auto _ : state)
    {
        log_inst->flush();
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_F(logger_benchmark, multi_thread_logging)(benchmark::State &state)
{
    if(state.thread_index() == 0)
    {
        log_inst->set_level(level::info);
    }

    for(auto _ : state)
    {
        log_inst->info("Thread {} message: {}",
                       static_cast<int>(state.thread_index()),
                       benchmark_data_generator::random_int());
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_F(logger_benchmark, small_messages)(benchmark::State &state)
{
    log_inst->set_level(level::info);

    for(auto _ : state)
    {
        log_inst->info("Small: {}",
                       benchmark_data_generator::random_string(50));
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_F(logger_benchmark, medium_messages)(benchmark::State &state)
{
    log_inst->set_level(level::info);

    for(auto _ : state)
    {
        log_inst->info("Medium: {}", benchmark_data_generator::medium_string());
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_F(logger_benchmark, no_format_logging)(benchmark::State &state)
{
    for(auto _ : state)
    {
        log_inst->info("Static message without formatting");
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_F(logger_benchmark, disabled_level_logging)(benchmark::State &state)
{
    log_inst->set_level(level::warning);

    for(auto _ : state)
    {
        log_inst->debug("This debug message should be filtered");
        log_inst->info("This info message should be filtered");
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_REGISTER_F(logger_benchmark, multi_thread_logging)->Threads(4);