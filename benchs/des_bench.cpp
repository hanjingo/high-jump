#include <benchmark/benchmark.h>
#include <hj/crypto/des.hpp>
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

static const std::vector<hj::des::mode> all_modes = {hj::des::mode::ecb,
                                                     hj::des::mode::cbc,
                                                     hj::des::mode::cfb,
                                                     hj::des::mode::ofb,
                                                     hj::des::mode::ctr};

static const std::vector<hj::des::padding> all_paddings = {
    hj::des::padding::pkcs5,
    hj::des::padding::pkcs7,
    hj::des::padding::zero,
    hj::des::padding::iso10126,
    hj::des::padding::ansix923,
    hj::des::padding::iso_iec_7816_4,
    hj::des::padding::no_padding};

static void bm_des_encrypt(benchmark::State &state,
                           hj::des::mode     mode,
                           hj::des::padding  padding,
                           size_t            key_len,
                           size_t            plain_len)
{
    std::mt19937 rng(42);
    std::string  key   = random_string(key_len, rng);
    std::string  iv    = random_string(8, rng);
    std::string  plain = random_string(plain_len, rng);
    std::string  cipher;
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(plain);
        cipher.clear();
        auto ec =
            hj::des::encrypt(cipher,
                             plain,
                             key,
                             mode,
                             padding,
                             (mode == hj::des::mode::ecb ? std::string() : iv));
        benchmark::DoNotOptimize(cipher);
        benchmark::ClobberMemory();
        if(ec != hj::des::error_code::ok)
            state.SkipWithError("encrypt failed");
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(plain_len));
}

static void bm_des_decrypt(benchmark::State &state,
                           hj::des::mode     mode,
                           hj::des::padding  padding,
                           size_t            key_len,
                           size_t            plain_len)
{
    std::mt19937 rng(43);
    std::string  key   = random_string(key_len, rng);
    std::string  iv    = random_string(8, rng);
    std::string  plain = random_string(plain_len, rng);
    std::string  cipher;
    hj::des::encrypt(cipher,
                     plain,
                     key,
                     mode,
                     padding,
                     (mode == hj::des::mode::ecb ? std::string() : iv));
    std::string decrypted;
    for(auto _ : state)
    {
        decrypted.clear();
        auto ec =
            hj::des::decrypt(decrypted,
                             cipher,
                             key,
                             mode,
                             padding,
                             (mode == hj::des::mode::ecb ? std::string() : iv));
        benchmark::DoNotOptimize(decrypted);
        benchmark::ClobberMemory();
        if(ec != hj::des::error_code::ok)
            state.SkipWithError("decrypt failed");
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(plain_len));
}

#define REGISTER_DES_BENCHMARKS(KEYLEN, PLAINTEXTLEN)                          \
    for(auto mode : all_modes)                                                 \
    {                                                                          \
        for(auto padding : all_paddings)                                       \
        {                                                                      \
            std::string name =                                                 \
                "des_encrypt/" + std::to_string(KEYLEN) + "_"                  \
                + std::to_string(PLAINTEXTLEN)                                 \
                + "/mode=" + std::to_string(static_cast<int>(mode))            \
                + "/pad=" + std::to_string(static_cast<int>(padding));         \
            benchmark::RegisterBenchmark(name.c_str(),                         \
                                         &bm_des_encrypt,                      \
                                         mode,                                 \
                                         padding,                              \
                                         KEYLEN,                               \
                                         PLAINTEXTLEN);                        \
            name = "des_decrypt/" + std::to_string(KEYLEN) + "_"               \
                   + std::to_string(PLAINTEXTLEN)                              \
                   + "/mode=" + std::to_string(static_cast<int>(mode))         \
                   + "/pad=" + std::to_string(static_cast<int>(padding));      \
            benchmark::RegisterBenchmark(name.c_str(),                         \
                                         &bm_des_decrypt,                      \
                                         mode,                                 \
                                         padding,                              \
                                         KEYLEN,                               \
                                         PLAINTEXTLEN);                        \
        }                                                                      \
    }

void RegisterAllDESBenchmarks()
{
    REGISTER_DES_BENCHMARKS(8, 8);
    REGISTER_DES_BENCHMARKS(8, 64);
    REGISTER_DES_BENCHMARKS(8, 1024);
    REGISTER_DES_BENCHMARKS(8, 4096);
    REGISTER_DES_BENCHMARKS(16, 64);
    REGISTER_DES_BENCHMARKS(24, 1024);
}

auto des_bench_init = (RegisterAllDESBenchmarks(), 0);
