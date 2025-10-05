#include <benchmark/benchmark.h>
#include <hj/crypto/rsa.hpp>
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

struct RsaKeyPair
{
    std::string pubkey;
    std::string prikey;
};

RsaKeyPair generate_rsa_keypair(size_t bits = 2048)
{
    RsaKeyPair kp;
    hj::rsa::keygen(kp.pubkey, kp.prikey, bits);
    return kp;
}

static void bm_rsa_encrypt(benchmark::State &state,
                           size_t            key_bits,
                           hj::rsa::padding  pad,
                           size_t            plain_len)
{
    std::mt19937 rng(42);
    auto         kp    = generate_rsa_keypair(key_bits);
    std::string  plain = random_string(plain_len, rng);
    std::string  cipher;
    for(auto _ : state)
    {
        cipher.clear();
        auto ec = hj::rsa::encrypt(cipher, plain, kp.pubkey, pad);
        benchmark::DoNotOptimize(cipher);
        if(ec != hj::rsa::error_code::ok)
            state.SkipWithError("encrypt failed");
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(plain_len));
}

static void bm_rsa_decrypt(benchmark::State &state,
                           size_t            key_bits,
                           hj::rsa::padding  pad,
                           size_t            plain_len)
{
    std::mt19937 rng(43);
    auto         kp    = generate_rsa_keypair(key_bits);
    std::string  plain = random_string(plain_len, rng);
    std::string  cipher;
    hj::rsa::encrypt(cipher, plain, kp.pubkey, pad);
    std::string decrypted;
    for(auto _ : state)
    {
        decrypted.clear();
        auto ec = hj::rsa::decrypt(decrypted, cipher, kp.prikey, pad);
        benchmark::DoNotOptimize(decrypted);
        if(ec != hj::rsa::error_code::ok)
            state.SkipWithError("decrypt failed");
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(plain_len));
}

static void
bm_rsa_signature(benchmark::State &state, size_t key_bits, size_t plain_len)
{
    std::mt19937               rng(44);
    auto                       kp    = generate_rsa_keypair(key_bits);
    std::string                plain = random_string(plain_len, rng);
    std::vector<unsigned char> sig(4096);
    std::size_t                sig_len = sig.size();
    for(auto _ : state)
    {
        sig_len = sig.size();
        auto ec = hj::rsa::signature(
            sig.data(),
            sig_len,
            reinterpret_cast<const unsigned char *>(plain.data()),
            plain.size(),
            reinterpret_cast<const unsigned char *>(kp.prikey.data()),
            kp.prikey.size());
        benchmark::DoNotOptimize(sig);
        if(ec != hj::rsa::error_code::ok)
            state.SkipWithError("signature failed");
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(plain_len));
}

static void
bm_rsa_verify(benchmark::State &state, size_t key_bits, size_t plain_len)
{
    std::mt19937               rng(45);
    auto                       kp    = generate_rsa_keypair(key_bits);
    std::string                plain = random_string(plain_len, rng);
    std::vector<unsigned char> sig(4096);
    std::size_t                sig_len = sig.size();
    hj::rsa::signature(
        sig.data(),
        sig_len,
        reinterpret_cast<const unsigned char *>(plain.data()),
        plain.size(),
        reinterpret_cast<const unsigned char *>(kp.prikey.data()),
        kp.prikey.size());
    std::vector<unsigned char> out(4096);
    std::size_t                out_len = out.size();
    for(auto _ : state)
    {
        out_len = out.size();
        auto ec = hj::rsa::verify_signature(
            out.data(),
            out_len,
            sig.data(),
            sig_len,
            reinterpret_cast<const unsigned char *>(kp.pubkey.data()),
            kp.pubkey.size());
        benchmark::DoNotOptimize(out);
        if(ec != hj::rsa::error_code::ok)
            state.SkipWithError("verify failed");
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(plain_len));
}

#define REGISTER_RSA_BENCHMARKS(KEYBITS, PAD, PLAINTEXTLEN)                    \
    benchmark::RegisterBenchmark(                                              \
        ("rsa_encrypt/" #KEYBITS "/" #PAD "/" #PLAINTEXTLEN),                  \
        &bm_rsa_encrypt,                                                       \
        KEYBITS,                                                               \
        hj::rsa::padding::PAD,                                                 \
        PLAINTEXTLEN);                                                         \
    benchmark::RegisterBenchmark(                                              \
        ("rsa_decrypt/" #KEYBITS "/" #PAD "/" #PLAINTEXTLEN),                  \
        &bm_rsa_decrypt,                                                       \
        KEYBITS,                                                               \
        hj::rsa::padding::PAD,                                                 \
        PLAINTEXTLEN);

void RegisterAllRSABenchmarks()
{
    REGISTER_RSA_BENCHMARKS(1024, pkcs1, 8);
    REGISTER_RSA_BENCHMARKS(1024, pkcs1, 32);
    REGISTER_RSA_BENCHMARKS(1024, pkcs1, 64);
    REGISTER_RSA_BENCHMARKS(1024, no_padding, 128);
    REGISTER_RSA_BENCHMARKS(2048, pkcs1, 8);
    REGISTER_RSA_BENCHMARKS(2048, pkcs1, 32);
    REGISTER_RSA_BENCHMARKS(2048, pkcs1, 128);
    REGISTER_RSA_BENCHMARKS(2048, no_padding, 256);
    REGISTER_RSA_BENCHMARKS(3072, pkcs1, 8);
    REGISTER_RSA_BENCHMARKS(3072, pkcs1, 32);
    REGISTER_RSA_BENCHMARKS(3072, pkcs1, 256);
    REGISTER_RSA_BENCHMARKS(3072, no_padding, 384);
    REGISTER_RSA_BENCHMARKS(4096, pkcs1, 8);
    REGISTER_RSA_BENCHMARKS(4096, pkcs1, 32);
    REGISTER_RSA_BENCHMARKS(4096, pkcs1, 256);
    REGISTER_RSA_BENCHMARKS(4096, no_padding, 512);

    benchmark::RegisterBenchmark("rsa_signature/2048/64",
                                 &bm_rsa_signature,
                                 2048,
                                 64);
    benchmark::RegisterBenchmark("rsa_verify/2048/64",
                                 &bm_rsa_verify,
                                 2048,
                                 64);
}

auto rsa_bench_init = (RegisterAllRSABenchmarks(), 0);
