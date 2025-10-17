#include <benchmark/benchmark.h>

#include <hj/encoding/xml.hpp>
#include <string>
#include <fstream>
#include <filesystem>
#include <chrono>

using namespace hj;

// Load from string (in-memory parse)
static void bm_xml_load_from_string(benchmark::State &state)
{
    const char *text = "<root><item>42</item></root>";

    for(auto _ : state)
    {
        xml x;
        x.load(text);
        auto item = x.child("item");
        benchmark::DoNotOptimize(item.value());
        benchmark::ClobberMemory();
    }
}

// Load from file: create the file outside timing, measure load_file
static void bm_xml_load_file(benchmark::State &state)
{
    // prepare a temp file path per run
    for(auto _ : state)
    {
        state.PauseTiming();
        auto tmpdir = std::filesystem::temp_directory_path();
        auto fn =
            tmpdir
            / ("hj_xml_bench_"
               + std::to_string(
                   std::chrono::steady_clock::now().time_since_epoch().count())
               + ".xml");
        {
            std::ofstream out(fn.string());
            out << "<root><foo>bar</foo></root>";
        }
        state.ResumeTiming();

        xml  x;
        bool ok = x.load_file(fn.string().c_str());
        benchmark::DoNotOptimize(ok);
        benchmark::DoNotOptimize(x.child("foo").value());

        state.PauseTiming();
        std::filesystem::remove(fn);
        state.ResumeTiming();

        benchmark::ClobberMemory();
    }
}

// Append child / set attr / set value micro-benchmark
static void bm_xml_append_child_attr(benchmark::State &state)
{
    for(auto _ : state)
    {
        xml x;
        x.load("<root></root>");
        auto child = x.append_child("item");
        child.set_value("abc");
        child.set_attr("id", "123");
        benchmark::DoNotOptimize(child.value());
        benchmark::DoNotOptimize(child.attr("id"));
        benchmark::ClobberMemory();
    }
}

// Remove child micro-benchmark
static void bm_xml_remove_child(benchmark::State &state)
{
    for(auto _ : state)
    {
        xml x;
        x.load("<root><a/><b/><c/></root>");
        bool removed = x.remove_child("b");
        benchmark::DoNotOptimize(removed);
        benchmark::ClobberMemory();
    }
}

// String serialization benchmark
static void bm_xml_str_serialize(benchmark::State &state)
{
    for(auto _ : state)
    {
        xml x;
        x.load("<root><foo>bar</foo></root>");
        auto s = x.str();
        benchmark::DoNotOptimize(s);
        benchmark::ClobberMemory();
    }
}

BENCHMARK(bm_xml_load_from_string)->Unit(benchmark::kMicrosecond);
BENCHMARK(bm_xml_load_file)->Unit(benchmark::kMillisecond);
BENCHMARK(bm_xml_append_child_attr)->Unit(benchmark::kMicrosecond);
BENCHMARK(bm_xml_remove_child)->Unit(benchmark::kMicrosecond);
BENCHMARK(bm_xml_str_serialize)->Unit(benchmark::kMicrosecond);
