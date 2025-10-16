#include <benchmark/benchmark.h>
#include <hj/encoding/json.hpp>
#include <string>
#include <fstream>
#include <filesystem>
#include <chrono>

using namespace hj;

static void bm_json_parse_string(benchmark::State &state)
{
    const char *s = R"({"pi":3.141,"happy":true,"name":"hj"})";
    for(auto _ : state)
    {
        auto js = hj::json::parse(s);
        benchmark::DoNotOptimize(js);
    }
}

static void bm_json_make_object(benchmark::State &state)
{
    for(auto _ : state)
    {
        hj::json js;
        js["id"]     = 1001;
        js["name"]   = "benchmark_user";
        js["active"] = true;
        js["score"]  = 99.5;
        benchmark::ClobberMemory();
        benchmark::DoNotOptimize(js);
    }
}

static void bm_json_dump_parse(benchmark::State &state)
{
    hj::json js;
    js["id"]   = 42;
    js["name"] = "roundtrip";
    js["vals"] = hj::json::array();
    for(int i = 0; i < 50; ++i)
        js["vals"].push_back(i);

    for(auto _ : state)
    {
        std::string dumped = js.dump();
        auto        parsed = hj::json::parse(dumped);
        benchmark::DoNotOptimize(parsed);
    }
}

static void bm_json_array_iterate(benchmark::State &state)
{
    hj::json arr = hj::json::array();
    for(int i = 0; i < 1000; ++i)
        arr.push_back(i);

    for(auto _ : state)
    {
        long long sum = 0;
        for(const auto &v : arr)
            sum += v.get<int>();
        benchmark::DoNotOptimize(sum);
    }
}

static void bm_json_file_parse(benchmark::State &state)
{
    // prepare a small JSON file once (not timed)
    auto tmp = std::filesystem::temp_directory_path();
    auto filename =
        tmp
        / (std::string("hj_json_bench_")
           + std::to_string(std::chrono::high_resolution_clock::now()
                                .time_since_epoch()
                                .count())
           + std::string(".json"));

    {
        std::ofstream ofs(filename);
        ofs << "{ \"pi\": 3.141, \"happy\": true, \"arr\": [1,2,3,4,5] }";
        ofs.close();
    }

    for(auto _ : state)
    {
        state.PauseTiming();
        std::ifstream ifs(filename);
        state.ResumeTiming();

        if(ifs)
        {
            auto js = hj::json::parse(ifs);
            benchmark::DoNotOptimize(js);
        }
        state.PauseTiming();
        ifs.close();
        state.ResumeTiming();
    }

    // cleanup
    std::error_code ec;
    std::filesystem::remove(filename, ec);
}

// register
BENCHMARK(bm_json_parse_string)->Iterations(5000);
BENCHMARK(bm_json_make_object)->Iterations(20000);
BENCHMARK(bm_json_dump_parse)->Iterations(2000);
BENCHMARK(bm_json_array_iterate)->Iterations(2000);
BENCHMARK(bm_json_file_parse)->Iterations(200);