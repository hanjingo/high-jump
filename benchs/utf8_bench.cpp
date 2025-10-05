#include <benchmark/benchmark.h>
#include <hj/encoding/utf8.hpp>
#include <string>
#include <vector>

static std::string  utf8_str = u8"你好，世界！Hello, world! 🌍";
static std::wstring wide_str = hj::utf8::encode(utf8_str);

static void bm_utf8_is_valid(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(hj::utf8::is_valid(utf8_str));
    }
}
BENCHMARK(bm_utf8_is_valid);

// std::string -> std::wstring
static void bm_utf8_encode(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(hj::utf8::encode(utf8_str));
    }
}
BENCHMARK(bm_utf8_encode);

// std::wstring -> std::string
static void bm_utf8_decode(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(hj::utf8::decode(wide_str));
    }
}
BENCHMARK(bm_utf8_decode);

// std::wstring -> std::string
static void bm_utf8_decode_large(benchmark::State &state)
{
    std::wstring large_wstr;
    for(int i = 0; i < 10000; ++i)
        large_wstr += wide_str;
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(hj::utf8::decode(large_wstr));
    }
}
BENCHMARK(bm_utf8_decode_large);

// std::string -> std::wstring
static void bm_utf8_encode_large(benchmark::State &state)
{
    std::string large_str;
    for(int i = 0; i < 10000; ++i)
        large_str += utf8_str;
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(hj::utf8::encode(large_str));
    }
}
BENCHMARK(bm_utf8_encode_large);