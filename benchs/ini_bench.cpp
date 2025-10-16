#include <benchmark/benchmark.h>
#include <hj/encoding/ini.hpp>
#include <string>
#include <filesystem>
#include <chrono>

using namespace hj;

static void bm_ini_parse(benchmark::State &state)
{
    const char text[] = "[section1]\na=123\nb=abc\nc=123.456";
    for(auto _ : state)
    {
        auto cfg = hj::ini::parse(text);
        benchmark::DoNotOptimize(cfg);
    }
}

static void bm_ini_get_set(benchmark::State &state)
{
    for(auto _ : state)
    {
        hj::ini cfg;
        cfg.put("person.name", "hanjingo");
        cfg.put("person.age", 30);
        cfg.put("person.income", 10000.123);

        auto a = cfg.get<int>("person.age");
        auto b = cfg.get<std::string>("person.name");
        auto c = cfg.get<double>("person.income");

        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
        benchmark::DoNotOptimize(c);
    }
}

static void bm_ini_str_roundtrip(benchmark::State &state)
{
    for(auto _ : state)
    {
        hj::ini cfg;
        cfg.put("person.name", "hjuser");
        cfg.put("person.val", 42);

        state.PauseTiming();
        std::string s = cfg.str();
        state.ResumeTiming();

        auto parsed = hj::ini::parse(s.c_str());
        benchmark::DoNotOptimize(parsed);
    }
}

static void bm_ini_file_read_write(benchmark::State &state)
{
    hj::ini cfg;
    cfg.put("x.y", "value");
    cfg.put("x.z", 12345);

    for(auto _ : state)
    {
        // prepare a temp file path (not timed)
        state.PauseTiming();
        auto tmp = std::filesystem::temp_directory_path();
        auto filename =
            tmp
            / (std::string("hj_ini_bench_")
               + std::to_string(std::chrono::high_resolution_clock::now()
                                    .time_since_epoch()
                                    .count())
               + std::string(".ini"));
        state.ResumeTiming();

        bool write_ok = cfg.write_file(filename.string().c_str());
        benchmark::DoNotOptimize(write_ok);

        hj::ini readcfg;
        bool    read_ok = readcfg.read_file(filename.string().c_str());
        benchmark::DoNotOptimize(read_ok);

        // cleanup (not timed)
        state.PauseTiming();
        std::error_code ec;
        std::filesystem::remove(filename, ec);
        state.ResumeTiming();
    }
}

BENCHMARK(bm_ini_parse)->Iterations(5000);
BENCHMARK(bm_ini_get_set)->Iterations(20000);
BENCHMARK(bm_ini_str_roundtrip)->Iterations(2000);
BENCHMARK(bm_ini_file_read_write)->Iterations(200);