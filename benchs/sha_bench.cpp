#include <benchmark/benchmark.h>
#include <hj/crypto/sha.hpp>
#include <string>
#include <vector>
#include <random>
#include <sstream>

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

static void bm_sha_encode_string(benchmark::State  &state,
                                 hj::sha::algorithm algo,
                                 size_t             input_len)
{
    std::mt19937 rng(42);
    std::string  input = random_string(input_len, rng);
    std::string  output;

    for(auto _ : state)
    {
        output.clear();
        auto ec = hj::sha::encode(output, input, algo);
        benchmark::DoNotOptimize(output);
        if(ec != hj::sha::error_code::ok)
        {
            state.SkipWithError("SHA encode failed");
        }
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(input_len));
}

static void bm_sha_encode_binary(benchmark::State  &state,
                                 hj::sha::algorithm algo,
                                 size_t             input_len)
{
    std::mt19937               rng(43);
    std::string                input = random_string(input_len, rng);
    std::vector<unsigned char> output(hj::sha::get_digest_length(algo));
    std::size_t                output_len = output.size();

    for(auto _ : state)
    {
        output_len = output.size();
        auto ec    = hj::sha::encode(
            output.data(),
            output_len,
            reinterpret_cast<const unsigned char *>(input.data()),
            input.size(),
            algo);
        benchmark::DoNotOptimize(output);
        if(ec != hj::sha::error_code::ok)
        {
            state.SkipWithError("SHA encode failed");
        }
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(input_len));
}

static void bm_sha_encode_stream(benchmark::State  &state,
                                 hj::sha::algorithm algo,
                                 size_t             input_len)
{
    std::mt19937 rng(44);
    std::string  input = random_string(input_len, rng);
    std::string  output;

    for(auto _ : state)
    {
        std::istringstream in(input);
        output.clear();
        auto ec = hj::sha::encode(output, in, algo);
        benchmark::DoNotOptimize(output);
        if(ec != hj::sha::error_code::ok)
        {
            state.SkipWithError("SHA stream encode failed");
        }
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(input_len));
}

template <size_t N>
static void bm_sha_encode_array(benchmark::State  &state,
                                hj::sha::algorithm algo,
                                size_t             input_len)
{
    std::mt19937      rng(45);
    std::string       input = random_string(input_len, rng);
    hj::hash_array<N> output;

    for(auto _ : state)
    {
        output.fill(0);
        auto ec = hj::sha::encode(output, input, algo);
        benchmark::DoNotOptimize(output);
        if(ec != hj::sha::error_code::ok)
        {
            state.SkipWithError("SHA array encode failed");
        }
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(input_len));
}

static void bm_sha1_convenience(benchmark::State &state, size_t input_len)
{
    std::mt19937 rng(46);
    std::string  input = random_string(input_len, rng);
    std::string  output;

    for(auto _ : state)
    {
        output.clear();
        auto ec = hj::sha::sha1(output, input);
        benchmark::DoNotOptimize(output);
        if(ec != hj::sha::error_code::ok)
        {
            state.SkipWithError("SHA1 convenience failed");
        }
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(input_len));
}

static void bm_sha256_convenience(benchmark::State &state, size_t input_len)
{
    std::mt19937 rng(47);
    std::string  input = random_string(input_len, rng);
    std::string  output;

    for(auto _ : state)
    {
        output.clear();
        auto ec = hj::sha::sha256(output, input);
        benchmark::DoNotOptimize(output);
        if(ec != hj::sha::error_code::ok)
        {
            state.SkipWithError("SHA256 convenience failed");
        }
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(input_len));
}

static void bm_sha512_convenience(benchmark::State &state, size_t input_len)
{
    std::mt19937 rng(48);
    std::string  input = random_string(input_len, rng);
    std::string  output;

    for(auto _ : state)
    {
        output.clear();
        auto ec = hj::sha::sha512(output, input);
        benchmark::DoNotOptimize(output);
        if(ec != hj::sha::error_code::ok)
        {
            state.SkipWithError("SHA512 convenience failed");
        }
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(input_len));
}

#define REGISTER_SHA_BENCHMARKS(ALGO, ALGO_NAME, INPUT_SIZE)                   \
    benchmark::RegisterBenchmark(("sha_" #ALGO_NAME "_string/" #INPUT_SIZE),   \
                                 &bm_sha_encode_string,                        \
                                 hj::sha::algorithm::ALGO,                     \
                                 INPUT_SIZE);                                  \
    benchmark::RegisterBenchmark(("sha_" #ALGO_NAME "_binary/" #INPUT_SIZE),   \
                                 &bm_sha_encode_binary,                        \
                                 hj::sha::algorithm::ALGO,                     \
                                 INPUT_SIZE);                                  \
    benchmark::RegisterBenchmark(("sha_" #ALGO_NAME "_stream/" #INPUT_SIZE),   \
                                 &bm_sha_encode_stream,                        \
                                 hj::sha::algorithm::ALGO,                     \
                                 INPUT_SIZE);

#define REGISTER_SHA_ARRAY_BENCHMARKS(ALGO, ALGO_NAME, DIGEST_LEN, INPUT_SIZE) \
    benchmark::RegisterBenchmark(("sha_" #ALGO_NAME "_array/" #INPUT_SIZE),    \
                                 &bm_sha_encode_array<DIGEST_LEN>,             \
                                 hj::sha::algorithm::ALGO,                     \
                                 INPUT_SIZE);

void RegisterAllSHABenchmarks()
{
    const std::vector<size_t> input_sizes =
        {16, 64, 256, 1024, 4096, 16384, 65536};

    for(auto size : input_sizes)
    {
        // SHA-1 (20 bytes digest)
        REGISTER_SHA_BENCHMARKS(sha1, SHA1, size);
        REGISTER_SHA_ARRAY_BENCHMARKS(sha1, SHA1, 20, size);

        // SHA-224 (28 bytes digest)
        REGISTER_SHA_BENCHMARKS(sha224, SHA224, size);
        REGISTER_SHA_ARRAY_BENCHMARKS(sha224, SHA224, 28, size);

        // SHA-256 (32 bytes digest)
        REGISTER_SHA_BENCHMARKS(sha256, SHA256, size);
        REGISTER_SHA_ARRAY_BENCHMARKS(sha256, SHA256, 32, size);

        // SHA-384 (48 bytes digest)
        REGISTER_SHA_BENCHMARKS(sha384, SHA384, size);
        REGISTER_SHA_ARRAY_BENCHMARKS(sha384, SHA384, 48, size);

        // SHA-512 (64 bytes digest)
        REGISTER_SHA_BENCHMARKS(sha512, SHA512, size);
        REGISTER_SHA_ARRAY_BENCHMARKS(sha512, SHA512, 64, size);
    }

    for(auto size : {64, 1024, 16384})
    {
        benchmark::RegisterBenchmark(
            ("SHA1_convenience/" + std::to_string(size)).c_str(),
            &bm_sha1_convenience,
            size);
        benchmark::RegisterBenchmark(
            ("SHA256_convenience/" + std::to_string(size)).c_str(),
            &bm_sha256_convenience,
            size);
        benchmark::RegisterBenchmark(
            ("SHA512_convenience/" + std::to_string(size)).c_str(),
            &bm_sha512_convenience,
            size);
    }
}

auto sha_bench_init = (RegisterAllSHABenchmarks(), 0);