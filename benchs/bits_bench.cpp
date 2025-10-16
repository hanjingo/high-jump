#include <benchmark/benchmark.h>
#include <hj/encoding/bits.hpp>
#include <cstdint>
#include <string>

using hj::bits;

static void bm_bits_get(benchmark::State &state)
{
    uint32_t v = 0xF0F0F0F0;
    for(auto _ : state)
    {
        bool b = bits::get(v, 5);
        benchmark::DoNotOptimize(b);
    }
}
BENCHMARK(bm_bits_get);

static void bm_bits_put_reset_flip(benchmark::State &state)
{
    for(auto _ : state)
    {
        uint32_t n = 0;
        bits::put(n, 1);
        bits::put(n, 32, true);
        bits::flip(n);
        bits::reset(n, false);
        benchmark::DoNotOptimize(n);
    }
}
BENCHMARK(bm_bits_put_reset_flip);

static void bm_bits_to_string_uint32(benchmark::State &state)
{
    uint32_t v = 0xDEADBEEF;
    for(auto _ : state)
    {
        std::string s = bits::to_string(v);
        benchmark::DoNotOptimize(s);
    }
}
BENCHMARK(bm_bits_to_string_uint32);

static void bm_bits_to_string_buffer(benchmark::State &state)
{
    uint16_t v = 0xFFFF;
    char     buf[sizeof(uint16_t) * 8 + 1];
    for(auto _ : state)
    {
        bits::to_string(v, buf);
        benchmark::DoNotOptimize(buf);
    }
}
BENCHMARK(bm_bits_to_string_buffer);

static void bm_bits_count_leading_zeros_32(benchmark::State &state)
{
    uint32_t v = 0x00F00000;
    for(auto _ : state)
    {
        int c = bits::count_leading_zeros<uint32_t>(v);
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(bm_bits_count_leading_zeros_32);

static void bm_bits_count_leading_zeros_64(benchmark::State &state)
{
    uint64_t v = 0x00F0000000000000ULL;
    for(auto _ : state)
    {
        int c = bits::count_leading_zeros<uint64_t>(v);
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(bm_bits_count_leading_zeros_64);
