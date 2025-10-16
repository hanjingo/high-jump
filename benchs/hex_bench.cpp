#include <benchmark/benchmark.h>

#include <hj/encoding/hex.hpp>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

using namespace hj;

static void bm_hex_decode_int(benchmark::State &state)
{
    for(auto _ : state)
    {
        volatile int v1 = hex::decode<int>("FF");
        volatile int v2 = hex::decode<int>("FFFF");
        (void) v1;
        (void) v2;
        benchmark::ClobberMemory();
    }
}

static void bm_hex_encode_int(benchmark::State &state)
{
    for(auto _ : state)
    {
        auto s1 = hex::encode<int>(255, true);
        auto s2 = hex::encode<int>(4095, false);
        (void) s1;
        (void) s2;
        benchmark::ClobberMemory();
    }
}

static void bm_hex_stream_encode(benchmark::State &state)
{
    std::string data(1024, '\xAA');
    for(auto _ : state)
    {
        std::istringstream in(data);
        std::ostringstream out;
        if(!hex::encode(out, in, true))
            state.SkipWithError("encode failed");
        benchmark::ClobberMemory();
    }
}

static void bm_hex_stream_decode(benchmark::State &state)
{
    std::string encoded =
        hex::encode(std::vector<unsigned char>(512, 0xAB), true);
    for(auto _ : state)
    {
        std::istringstream in(encoded);
        std::ostringstream out;
        if(!hex::decode(out, in))
            state.SkipWithError("decode failed");
        benchmark::ClobberMemory();
    }
}

static void bm_hex_is_valid_string(benchmark::State &state)
{
    std::string valid   = "AABBCCDDEEFF";
    std::string invalid = "AABBGG";
    for(auto _ : state)
    {
        volatile bool v1 = hex::is_valid(valid);
        volatile bool v2 = hex::is_valid(invalid);
        (void) v1;
        (void) v2;
        benchmark::ClobberMemory();
    }
}

static void bm_hex_is_valid_file(benchmark::State &state)
{
    const std::string fname = "bench_hex_tmp.txt";
    for(auto _ : state)
    {
        state.PauseTiming();
        // create a temp valid hex file
        std::ofstream ofs(fname, std::ios::binary);
        ofs << "AABBCCDDEEFF";
        ofs.close();
        state.ResumeTiming();

        bool ok = hex::is_valid_file(fname);
        if(!ok)
            state.SkipWithError("is_valid_file failed");

        state.PauseTiming();
        std::remove(fname.c_str());
        state.ResumeTiming();
    }
}

BENCHMARK(bm_hex_decode_int);
BENCHMARK(bm_hex_encode_int);
BENCHMARK(bm_hex_stream_encode);
BENCHMARK(bm_hex_stream_decode);
BENCHMARK(bm_hex_is_valid_string);
BENCHMARK(bm_hex_is_valid_file);
