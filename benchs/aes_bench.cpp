#include <benchmark/benchmark.h>
#include <hj/crypto/aes.hpp>
#include <vector>
#include <random>
#include <string>

static std::vector<unsigned char> make_random_vec(size_t len)
{
    std::vector<unsigned char>                  v(len);
    std::mt19937                                rng(12345);
    std::uniform_int_distribution<unsigned int> dist(0, 255);
    for(size_t i = 0; i < len; ++i)
        v[i] = dist(rng);
    return v;
}

static std::string                key128     = std::string(16, 'K');
static std::string                key256     = std::string(32, 'Z');
static std::string                iv         = std::string(16, 'I');
static std::vector<unsigned char> small_data = make_random_vec(128);
static std::vector<unsigned char> large_data = make_random_vec(1024 * 1024);

static void bm_aes_encrypt_small_ecb_pkcs7(benchmark::State &state)
{
    std::vector<unsigned char> out(256);
    std::size_t                out_len = out.size();
    for(auto _ : state)
    {
        auto ec = hj::aes::encrypt(
            out.data(),
            out_len,
            small_data.data(),
            small_data.size(),
            reinterpret_cast<const unsigned char *>(key128.c_str()),
            key128.size(),
            hj::aes::mode::ecb,
            hj::aes::padding::pkcs7,
            nullptr,
            0);
        benchmark::DoNotOptimize(ec);
    }
}
BENCHMARK(bm_aes_encrypt_small_ecb_pkcs7);

static void bm_aes_encrypt_large_cbc_pkcs7(benchmark::State &state)
{
    std::vector<unsigned char> out(large_data.size() + 32);
    std::size_t                out_len = out.size();
    for(auto _ : state)
    {
        auto ec = hj::aes::encrypt(
            out.data(),
            out_len,
            large_data.data(),
            large_data.size(),
            reinterpret_cast<const unsigned char *>(key256.c_str()),
            key256.size(),
            hj::aes::mode::cbc,
            hj::aes::padding::pkcs7,
            reinterpret_cast<const unsigned char *>(iv.c_str()),
            iv.size());
        benchmark::DoNotOptimize(ec);
    }
}
BENCHMARK(bm_aes_encrypt_large_cbc_pkcs7);

static void bm_aes_decrypt_small_ecb_pkcs7(benchmark::State &state)
{
    std::vector<unsigned char> enc(256);
    std::size_t                enc_len = enc.size();
    hj::aes::encrypt(enc.data(),
                     enc_len,
                     small_data.data(),
                     small_data.size(),
                     reinterpret_cast<const unsigned char *>(key128.c_str()),
                     key128.size(),
                     hj::aes::mode::ecb,
                     hj::aes::padding::pkcs7,
                     nullptr,
                     0);
    std::vector<unsigned char> out(128);
    std::size_t                out_len = out.size();
    for(auto _ : state)
    {
        auto ec = hj::aes::decrypt(
            out.data(),
            out_len,
            enc.data(),
            enc_len,
            reinterpret_cast<const unsigned char *>(key128.c_str()),
            key128.size(),
            hj::aes::mode::ecb,
            hj::aes::padding::pkcs7,
            nullptr,
            0);
        benchmark::DoNotOptimize(ec);
    }
}
BENCHMARK(bm_aes_decrypt_small_ecb_pkcs7);

static void bm_aes_decrypt_large_cbc_pkcs7(benchmark::State &state)
{
    std::vector<unsigned char> enc(large_data.size() + 32);
    std::size_t                enc_len = enc.size();
    hj::aes::encrypt(enc.data(),
                     enc_len,
                     large_data.data(),
                     large_data.size(),
                     reinterpret_cast<const unsigned char *>(key256.c_str()),
                     key256.size(),
                     hj::aes::mode::cbc,
                     hj::aes::padding::pkcs7,
                     reinterpret_cast<const unsigned char *>(iv.c_str()),
                     iv.size());
    std::vector<unsigned char> out(large_data.size());
    std::size_t                out_len = out.size();
    for(auto _ : state)
    {
        auto ec = hj::aes::decrypt(
            out.data(),
            out_len,
            enc.data(),
            enc_len,
            reinterpret_cast<const unsigned char *>(key256.c_str()),
            key256.size(),
            hj::aes::mode::cbc,
            hj::aes::padding::pkcs7,
            reinterpret_cast<const unsigned char *>(iv.c_str()),
            iv.size());
        benchmark::DoNotOptimize(ec);
    }
}
BENCHMARK(bm_aes_decrypt_large_cbc_pkcs7);
