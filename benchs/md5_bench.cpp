#include <benchmark/benchmark.h>
#include <hj/crypto/md5.hpp>
#include <string>
#include <vector>
#include <random>
#include <sstream>
#include <array>

template <typename RNG>
std::string random_string(size_t len, RNG &rng)
{
    static const char charset[] =
        "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string result;
    result.resize(len);
    std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);
    for(size_t i = 0; i < len; ++i)
    {
        result[i] = charset[dist(rng)];
    }
    return result;
}

static void bm_md5_string(benchmark::State &state, size_t len)
{
    std::mt19937 rng(123);
    std::string  input = random_string(len, rng);
    std::string  md5;
    for(auto _ : state)
    {
        md5.clear();
        auto ec = hj::md5::encode(md5, input);
        benchmark::DoNotOptimize(md5);
        if(ec != hj::md5::error_code::ok)
            state.SkipWithError("md5 encode failed");
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(len));
}

static void bm_md5_array(benchmark::State &state, size_t len)
{
    std::mt19937                           rng(456);
    std::string                            input = random_string(len, rng);
    std::array<uint8_t, MD5_DIGEST_LENGTH> md5{};
    for(auto _ : state)
    {
        md5.fill(0);
        auto ec = hj::md5::encode(md5, input);
        benchmark::DoNotOptimize(md5);
        if(ec != hj::md5::error_code::ok)
            state.SkipWithError("md5 encode failed");
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(len));
}

static void bm_md5_stream(benchmark::State &state, size_t len)
{
    std::mt19937       rng(789);
    std::string        input = random_string(len, rng);
    std::istringstream in(input);
    std::string        md5;
    for(auto _ : state)
    {
        in.clear();
        in.seekg(0);
        md5.clear();
        auto ec = hj::md5::encode(md5, in);
        benchmark::DoNotOptimize(md5);
        if(ec != hj::md5::error_code::ok)
            state.SkipWithError("md5 encode failed");
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(len));
}

#define REGISTER_MD5_BENCHMARKS(LEN)                                           \
    benchmark::RegisterBenchmark(("md5_string/" #LEN), &bm_md5_string, LEN);   \
    benchmark::RegisterBenchmark(("md5_array/" #LEN), &bm_md5_array, LEN);     \
    benchmark::RegisterBenchmark(("md5_stream/" #LEN), &bm_md5_stream, LEN);

void RegisterAllMD5Benchmarks()
{
    REGISTER_MD5_BENCHMARKS(8);
    REGISTER_MD5_BENCHMARKS(64);
    REGISTER_MD5_BENCHMARKS(1024);
    REGISTER_MD5_BENCHMARKS(4096);
    REGISTER_MD5_BENCHMARKS(1024 * 1024);
}

auto md5_bench_init = (RegisterAllMD5Benchmarks(), 0);
