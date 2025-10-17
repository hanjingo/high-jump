#include <benchmark/benchmark.h>

#include <hj/encoding/unicode.hpp>
#include <string>
#include <locale.h>

// Ensure a UTF-8 C locale for the conversions (match tests)
struct UnicodeLocaleInit
{
    UnicodeLocaleInit() { setlocale(LC_ALL, "en_US.UTF-8"); }
};
static UnicodeLocaleInit _unicode_locale_init;

static void bm_unicode_from_utf8_to_utf8_english(benchmark::State &state)
{
    std::string utf8_en = "hello";

    for(auto _ : state)
    {
        state.PauseTiming();
        // warm-up / ensure inputs are ready
        const std::string in = utf8_en;
        state.ResumeTiming();

        auto w   = hj::unicode::from_utf8(in);
        auto out = hj::unicode::to_utf8(w);
        benchmark::DoNotOptimize(out);
        benchmark::ClobberMemory();
    }
}

static void bm_unicode_from_utf8_to_utf8_chinese(benchmark::State &state)
{
    std::string utf8_cn = u8"你好，世界";

    for(auto _ : state)
    {
        state.PauseTiming();
        const std::string in = utf8_cn;
        state.ResumeTiming();

        auto w   = hj::unicode::from_utf8(in);
        auto out = hj::unicode::to_utf8(w);
        benchmark::DoNotOptimize(out);
        benchmark::ClobberMemory();
    }
}

static void bm_unicode_empty(benchmark::State &state)
{
    std::string empty = "";

    for(auto _ : state)
    {
        state.PauseTiming();
        const std::string in = empty;
        state.ResumeTiming();

        auto w   = hj::unicode::from_utf8(in);
        auto out = hj::unicode::to_utf8(w);
        benchmark::DoNotOptimize(w);
        benchmark::DoNotOptimize(out);
        benchmark::ClobberMemory();
    }
}

// Measure invalid utf8 handling (exception path). We catch the exception so the
// benchmark can continue; the throw cost is included in timing.
static void bm_unicode_invalid_utf8_throws(benchmark::State &state)
{
    std::string bad = "\xFF\xFF";

    for(auto _ : state)
    {
        state.PauseTiming();
        const std::string in = bad;
        state.ResumeTiming();

        try
        {
            auto w = hj::unicode::from_utf8(in);
            // If no exception, still touch result
            benchmark::DoNotOptimize(w);
        }
        catch(const std::exception &)
        {
            // expected for invalid utf8
            benchmark::ClobberMemory();
        }
    }
}

BENCHMARK(bm_unicode_from_utf8_to_utf8_english)->Unit(benchmark::kMicrosecond);
BENCHMARK(bm_unicode_from_utf8_to_utf8_chinese)->Unit(benchmark::kMicrosecond);
BENCHMARK(bm_unicode_empty)->Unit(benchmark::kMicrosecond);
BENCHMARK(bm_unicode_invalid_utf8_throws)->Unit(benchmark::kMicrosecond);
