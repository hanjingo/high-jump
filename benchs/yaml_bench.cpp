#include <benchmark/benchmark.h>

#include <hj/encoding/yaml.hpp>
#include <string>
#include <fstream>
#include <filesystem>
#include <vector>

using namespace hj;

// Parse from string
static void bm_yaml_parse_from_string(benchmark::State &state)
{
    const char *text = "name: test\nvalue: 42\n";
    for(auto _ : state)
    {
        yaml y = yaml::load(text);
        benchmark::DoNotOptimize(y);
        benchmark::ClobberMemory();
    }
}

// Read/Write file
static void bm_yaml_read_write_file(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        auto tmp =
            std::filesystem::temp_directory_path()
            / ("hj_yaml_bench_" + std::to_string(state.iterations()) + ".yaml");
        std::ofstream fout(tmp.string(), std::ios::binary);
        yaml          y;
        y["foo"] = "bar";
        y["num"] = 123;
        state.ResumeTiming();

        bool wrote = y.dump(fout);
        benchmark::DoNotOptimize(wrote);
        fout.close();

        state.PauseTiming();
        std::ifstream fin(tmp.string(), std::ios::binary);
        yaml          y2;
        state.ResumeTiming();

        y2.load(fin);
        benchmark::DoNotOptimize(y2);

        state.PauseTiming();
        fin.close();
        std::filesystem::remove(tmp);
        state.ResumeTiming();

        benchmark::ClobberMemory();
    }
}

// Serialize to string
static void bm_yaml_serialize_to_string(benchmark::State &state)
{
    for(auto _ : state)
    {
        yaml y;
        y["a"]        = 1;
        y["b"]        = 2;
        std::string s = y.str();
        benchmark::DoNotOptimize(s);
        benchmark::ClobberMemory();
    }
}

// Dump to buffer
static void bm_yaml_dump_to_buffer(benchmark::State &state)
{
    for(auto _ : state)
    {
        yaml y;
        y["x"]          = 100;
        char   buf[256] = {};
        size_t sz       = sizeof(buf);
        bool   ok       = y.dump(buf, sz);
        benchmark::DoNotOptimize(ok);
        benchmark::DoNotOptimize(sz);
        benchmark::ClobberMemory();
    }
}

// Iterator and type checks
static void bm_yaml_iterator_and_type(benchmark::State &state)
{
    const char *text = "arr:\n  - 1\n  - 2\n  - 3\nmap:\n  k1: v1\n  k2: v2\n";
    for(auto _ : state)
    {
        yaml y   = yaml::load(text);
        auto arr = y["arr"];
        int  sum = 0;
        for(auto it = arr.begin(); it != arr.end(); ++it)
            sum += it->as<int>();
        benchmark::DoNotOptimize(sum);
        int  count = 0;
        auto map   = y["map"];
        for(auto it = map.begin(); it != map.end(); ++it)
            ++count;
        benchmark::DoNotOptimize(count);
        benchmark::ClobberMemory();
    }
}

// Assignment and copy
static void bm_yaml_assignment_copy(benchmark::State &state)
{
    for(auto _ : state)
    {
        yaml y1;
        y1["a"] = 10;
        yaml y2 = y1;
        benchmark::DoNotOptimize(y2["a"].as<int>());
        y2["a"] = 20;
        benchmark::DoNotOptimize(y2["a"].as<int>());
        y1 = y2;
        benchmark::DoNotOptimize(y1["a"].as<int>());
        benchmark::ClobberMemory();
    }
}

// Force insert and push_back
static void bm_yaml_force_insert_push_back(benchmark::State &state)
{
    for(auto _ : state)
    {
        yaml y;
        y.force_insert("k", "v");
        yaml arr;
        arr.push_back(1);
        arr.push_back(2);
        arr.push_back(yaml::load("x: 5"));
        benchmark::DoNotOptimize(arr[2]["x"].as<int>());
        benchmark::ClobberMemory();
    }
}

BENCHMARK(bm_yaml_parse_from_string)->Unit(benchmark::kMicrosecond);
BENCHMARK(bm_yaml_read_write_file)->Unit(benchmark::kMillisecond);
BENCHMARK(bm_yaml_serialize_to_string)->Unit(benchmark::kMicrosecond);
BENCHMARK(bm_yaml_dump_to_buffer)->Unit(benchmark::kMicrosecond);
BENCHMARK(bm_yaml_iterator_and_type)->Unit(benchmark::kMicrosecond);
BENCHMARK(bm_yaml_assignment_copy)->Unit(benchmark::kMicrosecond);
BENCHMARK(bm_yaml_force_insert_push_back)->Unit(benchmark::kMicrosecond);
