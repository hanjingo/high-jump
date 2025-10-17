#include <benchmark/benchmark.h>
#include <hj/ai/qrcode.hpp>
#include <string>
#include <vector>
#include <filesystem>

using namespace hj;
namespace fs = std::filesystem;

static std::string make_payload(size_t n)
{
    std::string s;
    s.reserve(n);
    for(size_t i = 0; i < n; ++i)
        s.push_back(static_cast<char>('a' + (i % 26)));
    return s;
}

static void bm_qrcode_encode_small(benchmark::State &state)
{
    const std::string payload = make_payload(32);
    for(auto _ : state)
    {
        qrcode::bitmap bm;
        auto           ec = qrcode::builder::encode(bm, payload);
        benchmark::DoNotOptimize(ec);
        benchmark::DoNotOptimize(bm.data.data());
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_qrcode_encode_small);

static void bm_qrcode_encode_large(benchmark::State &state)
{
    const std::string payload = make_payload(1024);
    for(auto _ : state)
    {
        qrcode::bitmap bm;
        auto           ec = qrcode::builder::encode(bm, payload);
        benchmark::DoNotOptimize(ec);
        benchmark::DoNotOptimize(bm.data.data());
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_qrcode_encode_large);

static void bm_qrcode_decode_from_bitmap(benchmark::State &state)
{
    // prepare a bitmap with a medium payload
    const std::string payload = make_payload(128);
    qrcode::bitmap    bm;
    auto              ec = qrcode::builder::encode(bm, payload);
    if(ec.value() != 0)
    {
        state.SkipWithError("failed to prepare bitmap for decode benchmark");
        return;
    }

    for(auto _ : state)
    {
        std::vector<std::string> results;
        auto                     ec2 = qrcode::parser::decode(results, bm);
        benchmark::DoNotOptimize(ec2);
        benchmark::DoNotOptimize(results.size());
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_qrcode_decode_from_bitmap);
