#include <benchmark/benchmark.h>
#include <hj/encoding/i18n.hpp>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <chrono>

using namespace hj;

static void bm_translator_construct(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(hj::i18n::translator("en_US"));
    }
}

static void bm_translator_add_translate(benchmark::State &state)
{
    // prepare a translator with many entries (setup not measured)
    const int            entries = 10000;
    hj::i18n::translator trans("test_locale");
    for(int i = 0; i < entries; ++i)
        trans.add(std::string("key.") + std::to_string(i),
                  std::string("value.") + std::to_string(i));

    size_t idx = 0;
    for(auto _ : state)
    {
        // cycle through keys
        std::string key = std::string("key.") + std::to_string(idx % entries);
        auto        v   = trans.translate(key);
        benchmark::DoNotOptimize(v);
        ++idx;
    }
}

static void bm_load_from_map(benchmark::State &state)
{
    const int                                    entries = 5000;
    std::unordered_map<std::string, std::string> m;
    m.reserve(entries);
    for(int i = 0; i < entries; ++i)
        m.emplace(std::string("k") + std::to_string(i),
                  std::string("v") + std::to_string(i));

    for(auto _ : state)
    {
        hj::i18n::translator t;
        bool                 ok = t.load_from_map(m);
        benchmark::DoNotOptimize(ok);
    }
}

static void bm_save_load_properties(benchmark::State &state)
{
    const int            entries = 2000;
    hj::i18n::translator t("save_locale");
    for(int i = 0; i < entries; ++i)
        t.add(std::string("s.") + std::to_string(i),
              std::string("val.") + std::to_string(i));

    for(auto _ : state)
    {
        // create a unique temp file path for this iteration (do not include setup in timing)
        state.PauseTiming();
        auto tmp = std::filesystem::temp_directory_path();
        auto filename =
            tmp
            / (std::string("hj_i18n_bench_")
               + std::to_string(std::chrono::high_resolution_clock::now()
                                    .time_since_epoch()
                                    .count())
               + std::string(".properties"));
        state.ResumeTiming();

        bool saved = t.save_to_properties(filename.string());
        benchmark::DoNotOptimize(saved);

        hj::i18n::translator loaded("save_locale");
        bool loaded_ok = loaded.load_from_properties(filename.string());
        benchmark::DoNotOptimize(loaded_ok);

        // best-effort cleanup (not measured)
        state.PauseTiming();
        std::error_code ec;
        std::filesystem::remove(filename, ec);
        state.ResumeTiming();
    }
}

static void bm_instance_translate(benchmark::State &state)
{
    auto &inst = hj::i18n::instance();
    inst.set_locale("bench_locale");
    auto trans = std::make_unique<hj::i18n::translator>("bench_locale");
    trans->add("hello", "Hello World");
    inst.install("main", std::move(trans));

    for(auto _ : state)
    {
        auto r = inst.translate("hello");
        benchmark::DoNotOptimize(r);
    }
}

// register benchmarks
BENCHMARK(bm_translator_construct);
BENCHMARK(bm_translator_add_translate)->Iterations(10000);
BENCHMARK(bm_load_from_map)->Iterations(2000);
BENCHMARK(bm_save_load_properties)->Iterations(200);
BENCHMARK(bm_instance_translate)->Iterations(20000);