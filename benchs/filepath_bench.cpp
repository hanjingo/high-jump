#include <benchmark/benchmark.h>

#include <hj/io/filepath.hpp>
#include <string>
#include <vector>
#include <chrono>
#include <atomic>

using namespace hj;

static void bm_pwd(benchmark::State &state)
{
    for(auto _ : state)
    {
        auto p = filepath::pwd();
        benchmark::ClobberMemory();
    }
}

static void bm_join(benchmark::State &state)
{
    for(auto _ : state)
    {
        auto s = filepath::join(filepath::pwd(), "subdir", "file.txt");
        benchmark::ClobberMemory();
    }
}

static void bm_parent_absolute_relative(benchmark::State &state)
{
    std::string sample = "./some/long/path/file.txt";
    for(auto _ : state)
    {
        auto p = filepath::parent(sample);
        auto a = filepath::absolute(sample);
        auto r = filepath::relative(a, filepath::pwd());
        benchmark::ClobberMemory();
    }
}

static void bm_filename_operations(benchmark::State &state)
{
    std::string sample = filepath::join(filepath::pwd(), "dir", "007.txt");
    for(auto _ : state)
    {
        auto fn  = filepath::file_name(sample);
        auto dn  = filepath::dir_name(sample);
        auto pn  = filepath::path_name(sample);
        auto ext = filepath::extension(sample);
        auto rep = filepath::replace_extension(sample, ".log");
        benchmark::ClobberMemory();
    }
}

static void bm_is_exist_is_dir_size_list_find(benchmark::State &state)
{
    auto        pwd      = filepath::pwd();
    std::string nonexist = filepath::join(pwd, "__not_exist_hopefully__");
    for(auto _ : state)
    {
        volatile bool      e  = filepath::is_exist(pwd);
        volatile bool      d  = filepath::is_dir(pwd);
        volatile long long sz = filepath::size(nonexist);
        auto               li = filepath::list(pwd);
        auto found = filepath::find(pwd, "does_not_mostly_exist_12345");
        (void) e;
        (void) d;
        (void) sz;
        (void) li;
        (void) found;
        benchmark::ClobberMemory();
    }
}

static std::atomic_uint_fast64_t bench_file_counter{0};

static void bm_make_and_remove_file(benchmark::State &state)
{
    for(auto _ : state)
    {
        // Prepare a unique temp file path outside measured region
        state.PauseTiming();
        auto        id = ++bench_file_counter;
        std::string fname =
            filepath::join(filepath::pwd(),
                           "bench_tmp_" + std::to_string(id) + ".tmp");
        if(filepath::is_exist(fname))
            filepath::remove(fname, false);
        state.ResumeTiming();

        // Measure file creation + size query
        bool created = filepath::make_file(fname);
        (void) created;
        volatile long long sz = filepath::size(fname);
        (void) sz;

        // Stop timing to remove file to keep environment clean
        state.PauseTiming();
        if(filepath::is_exist(fname))
            filepath::remove(fname, false);
        state.ResumeTiming();
    }
}

// Register benches
BENCHMARK(bm_pwd);
BENCHMARK(bm_join);
BENCHMARK(bm_parent_absolute_relative);
BENCHMARK(bm_filename_operations);
BENCHMARK(bm_is_exist_is_dir_size_list_find);
BENCHMARK(bm_make_and_remove_file);
