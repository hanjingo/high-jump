#include <benchmark/benchmark.h>
#include <hj/crypto/base64.hpp>
#include <string>
#include <vector>
#include <random>

static std::string make_random_data(size_t len)
{
    std::string                                 data(len, '\0');
    std::mt19937                                rng(12345);
    std::uniform_int_distribution<unsigned int> dist(0, 255);
    for(size_t i = 0; i < len; ++i)
        data[i] = static_cast<unsigned char>(dist(rng));
    return data;
}

static std::string small_data = make_random_data(128);
static std::string large_data = make_random_data(1024 * 1024);

static void bm_base64_encode_small(benchmark::State &state)
{
    std::string out;
    for(auto _ : state)
    {
        hj::base64::error_code ec = hj::base64::encode(out, small_data);
        benchmark::DoNotOptimize(ec);
    }
}
BENCHMARK(bm_base64_encode_small);

static void bm_base64_encode_large(benchmark::State &state)
{
    std::string out;
    for(auto _ : state)
    {
        hj::base64::error_code ec = hj::base64::encode(out, large_data);
        benchmark::DoNotOptimize(ec);
    }
}
BENCHMARK(bm_base64_encode_large);

static void bm_base64_decode_small(benchmark::State &state)
{
    std::string encoded;
    hj::base64::encode(encoded, small_data);
    std::string out;
    for(auto _ : state)
    {
        hj::base64::error_code ec = hj::base64::decode(out, encoded);
        benchmark::DoNotOptimize(ec);
    }
}
BENCHMARK(bm_base64_decode_small);

static void bm_base64_decode_large(benchmark::State &state)
{
    std::string encoded;
    hj::base64::encode(encoded, large_data);
    std::string out;
    for(auto _ : state)
    {
        hj::base64::error_code ec = hj::base64::decode(out, encoded);
        benchmark::DoNotOptimize(ec);
    }
}
BENCHMARK(bm_base64_decode_large);

static void bm_base64_is_valid_small(benchmark::State &state)
{
    std::string encoded;
    hj::base64::encode(encoded, small_data);
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(hj::base64::is_valid(encoded));
    }
}
BENCHMARK(bm_base64_is_valid_small);

static void bm_base64_is_valid_large(benchmark::State &state)
{
    std::string encoded;
    hj::base64::encode(encoded, large_data);
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(hj::base64::is_valid(encoded));
    }
}
BENCHMARK(bm_base64_is_valid_large);
