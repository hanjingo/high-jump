#include <benchmark/benchmark.h>

#include <hj/compress/gzip.hpp>
#include <vector>
#include <string>
#include <random>
#include <sstream>

using namespace hj;

// Prepare some data once for multiple benches
static std::vector<unsigned char> gzip_small_data;
static std::vector<unsigned char> gzip_random_data;
static std::vector<unsigned char> gzip_large_data;

static void prepare_gzip_data()
{
    if(!gzip_small_data.empty())
        return;
    std::string s = "Hello, World! This is a test string for hj::gzip.";
    gzip_small_data.assign(s.begin(), s.end());

    // random data ~50KB
    gzip_random_data.resize(50 * 1024);
    {
        std::mt19937                            gen(12345);
        std::uniform_int_distribution<unsigned> dis(0, 255);
        for(auto &b : gzip_random_data)
            b = static_cast<unsigned char>(dis(gen));
    }

    // large data ~200KB of zeros (compressible)
    gzip_large_data.assign(200 * 1024, 0);
}

static void bm_gzip_compress_small(benchmark::State &state)
{
    prepare_gzip_data();
    for(auto _ : state)
    {
        std::vector<unsigned char> out;
        auto                       res =
            gzip::compress(out, gzip_small_data.data(), gzip_small_data.size());
        if(res != gzip::err::ok)
            state.SkipWithError("compress failed");
        benchmark::ClobberMemory();
    }
}

static void bm_gzip_decompress_small(benchmark::State &state)
{
    prepare_gzip_data();
    // compress once
    std::vector<unsigned char> compressed;
    if(gzip::compress(compressed,
                      gzip_small_data.data(),
                      gzip_small_data.size())
       != gzip::err::ok)
    {
        state.SkipWithError("prepare compress failed");
        return;
    }

    for(auto _ : state)
    {
        std::vector<unsigned char> out;
        auto res = gzip::decompress(out, compressed.data(), compressed.size());
        if(res != gzip::err::ok)
            state.SkipWithError("decompress failed");
        benchmark::ClobberMemory();
    }
}

static void bm_gzip_compress_random(benchmark::State &state)
{
    prepare_gzip_data();
    for(auto _ : state)
    {
        std::vector<unsigned char> out;
        auto                       res = gzip::compress(out,
                                  gzip_random_data.data(),
                                  gzip_random_data.size());
        if(res != gzip::err::ok)
            state.SkipWithError("compress failed");
        benchmark::ClobberMemory();
    }
}

static void bm_gzip_compress_large(benchmark::State &state)
{
    prepare_gzip_data();
    for(auto _ : state)
    {
        std::vector<unsigned char> out;
        auto                       res =
            gzip::compress(out, gzip_large_data.data(), gzip_large_data.size());
        if(res != gzip::err::ok)
            state.SkipWithError("compress failed");
        benchmark::ClobberMemory();
    }
}

static void bm_gzip_stream_compress(benchmark::State &state)
{
    prepare_gzip_data();
    for(auto _ : state)
    {
        state.PauseTiming();
        std::istringstream in(
            std::string(gzip_small_data.begin(), gzip_small_data.end()));
        std::ostringstream out;
        state.ResumeTiming();

        auto res = gzip::compress(out, in);
        if(res != gzip::err::ok)
            state.SkipWithError("stream compress failed");

        benchmark::ClobberMemory();
    }
}

static void bm_gzip_crc32(benchmark::State &state)
{
    prepare_gzip_data();
    for(auto _ : state)
    {
        volatile auto c = gzip::crc32_checksum(gzip_large_data.data(),
                                               gzip_large_data.size());
        (void) c;
        benchmark::ClobberMemory();
    }
}

static void bm_gzip_compression_ratio(benchmark::State &state)
{
    prepare_gzip_data();
    std::vector<unsigned char> compressed;
    if(gzip::compress(compressed,
                      gzip_large_data.data(),
                      gzip_large_data.size())
       != gzip::err::ok)
    {
        state.SkipWithError("prepare compress failed");
        return;
    }

    for(auto _ : state)
    {
        volatile double r =
            gzip::compression_ratio(gzip_large_data.size(), compressed.size());
        (void) r;
        benchmark::ClobberMemory();
    }
}

BENCHMARK(bm_gzip_compress_small);
BENCHMARK(bm_gzip_decompress_small);
BENCHMARK(bm_gzip_compress_random);
BENCHMARK(bm_gzip_compress_large);
BENCHMARK(bm_gzip_stream_compress);
BENCHMARK(bm_gzip_crc32);
BENCHMARK(bm_gzip_compression_ratio);
