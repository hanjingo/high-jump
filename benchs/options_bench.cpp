#include <benchmark/benchmark.h>
#include <hj/os/options.hpp>
#include <vector>
#include <string>
#include <cstring>
#include <memory>

namespace
{

char **create_argv(const std::vector<std::string> &args,
                   std::vector<char *>            &allocated_args)
{
    char **argv = new char *[args.size()];
    for(size_t i = 0; i < args.size(); ++i)
    {
        argv[i] = new char[args[i].length() + 1];
        std::strcpy(argv[i], args[i].c_str());
        allocated_args.push_back(argv[i]);
    }
    return argv;
}

void free_argv(char **argv, const std::vector<char *> &allocated_args)
{
    for(auto p : allocated_args)
        delete[] p;
    delete[] argv;
}

} // namespace

// Benchmark: parse an integer option
static void bm_parse_integer_option(benchmark::State &state)
{
    for(auto _ : state)
    {
        // setup outside timing
        state.PauseTiming();
        hj::options opts;
        opts.add<int>("port", 8080, "Server port number");
        std::vector<std::string> args = {"program", "--port", "9000"};
        std::vector<char *>      allocated;
        char                   **argv = create_argv(args, allocated);
        state.ResumeTiming();

        int result =
            opts.parse<int>(static_cast<int>(args.size()), argv, "port");
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();

        // cleanup outside timing
        state.PauseTiming();
        free_argv(argv, allocated);
        state.ResumeTiming();
    }
}

// Benchmark: parse integer option when not provided (uses default)
static void bm_parse_integer_option_with_default(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::options opts;
        opts.add<int>("port", 8080, "Server port number");
        std::vector<std::string> args = {"program"};
        std::vector<char *>      allocated;
        char                   **argv = create_argv(args, allocated);
        state.ResumeTiming();

        int result =
            opts.parse<int>(static_cast<int>(args.size()), argv, "port");
        benchmark::DoNotOptimize(result);

        state.PauseTiming();
        free_argv(argv, allocated);
        state.ResumeTiming();
    }
}

// Benchmark: parse a string option
static void bm_parse_string_option(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::options opts;
        opts.add<std::string>("host", "localhost", "Server hostname");
        std::vector<std::string> args = {"program", "--host", "example.com"};
        std::vector<char *>      allocated;
        char                   **argv = create_argv(args, allocated);
        state.ResumeTiming();

        std::string result =
            opts.parse<std::string>(static_cast<int>(args.size()),
                                    argv,
                                    "host");
        benchmark::DoNotOptimize(result);

        state.PauseTiming();
        free_argv(argv, allocated);
        state.ResumeTiming();
    }
}

// Benchmark: parse multiple options at once
static void bm_parse_multiple_options(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::options opts;
        opts.add<std::string>("host", "localhost", "Server hostname");
        opts.add<int>("port", 8080, "Server port number");
        opts.add<bool>("verbose", false, "Enable verbose output");
        std::vector<std::string> args = {"program",
                                         "--host",
                                         "example.com",
                                         "--port",
                                         "9000",
                                         "--verbose",
                                         "true"};
        std::vector<char *>      allocated;
        char                   **argv = create_argv(args, allocated);
        state.ResumeTiming();

        std::string host =
            opts.parse<std::string>(static_cast<int>(args.size()),
                                    argv,
                                    "host");
        int port = opts.parse<int>(static_cast<int>(args.size()), argv, "port");
        bool verbose =
            opts.parse<bool>(static_cast<int>(args.size()), argv, "verbose");
        benchmark::DoNotOptimize(host);
        benchmark::DoNotOptimize(port);
        benchmark::DoNotOptimize(verbose);

        state.PauseTiming();
        free_argv(argv, allocated);
        state.ResumeTiming();
    }
}

// Benchmark: parse positional vector of files
static void bm_parse_position_vector(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::options opts;
        opts.add<std::vector<std::string>>("files", {}, "input files");
        opts.add_positional("files", 3);
        std::vector<std::string> args = {"program", "a.txt", "b.txt", "c.txt"};
        std::vector<char *>      allocated;
        char                   **argv = create_argv(args, allocated);
        state.ResumeTiming();

        auto files = opts.parse_positional<std::vector<std::string>>(
            static_cast<int>(args.size()),
            argv,
            "files");
        benchmark::DoNotOptimize(files.size());

        state.PauseTiming();
        free_argv(argv, allocated);
        state.ResumeTiming();
    }
}

BENCHMARK(bm_parse_integer_option)->Iterations(2000);
BENCHMARK(bm_parse_integer_option_with_default)->Iterations(2000);
BENCHMARK(bm_parse_string_option)->Iterations(2000);
BENCHMARK(bm_parse_multiple_options)->Iterations(1000);
BENCHMARK(bm_parse_position_vector)->Iterations(1000);