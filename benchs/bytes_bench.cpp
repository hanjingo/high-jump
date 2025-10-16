#include <benchmark/benchmark.h>
#include <hj/encoding/bytes.hpp>

#include <array>
#include <string>
#include <vector>

// Benchmarks for hj::bytes conversion utilities.

static void bm_bool_to_from(benchmark::State &st)
{
    std::array<unsigned char, 1> buf = {0};
    for(auto _ : st)
    {
        hj::bool_to_bytes(true, buf);
        bool v = hj::bytes_to_bool(buf);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(bm_bool_to_from);

static void bm_int32_to_from_big(benchmark::State &st)
{
    std::array<unsigned char, 4> buf = {0};
    int32_t                      n   = 0x12345678;
    for(auto _ : st)
    {
        hj::int32_to_bytes(n, buf, true);
        int32_t m = hj::bytes_to_int32(buf, true);
        benchmark::DoNotOptimize(m);
    }
}
BENCHMARK(bm_int32_to_from_big)->Range(1, 1 << 10);

static void bm_int32_to_from_little(benchmark::State &st)
{
    std::array<unsigned char, 4> buf = {0};
    int32_t                      n   = 0x12345678;
    for(auto _ : st)
    {
        hj::int32_to_bytes(n, buf, false);
        int32_t m = hj::bytes_to_int32(buf, false);
        benchmark::DoNotOptimize(m);
    }
}
BENCHMARK(bm_int32_to_from_little)->Range(1, 1 << 10);

static void bm_int64_to_from(benchmark::State &st)
{
    std::array<unsigned char, 8> buf = {0};
    int64_t                      n   = 0x123456789ABCDEF0LL;
    for(auto _ : st)
    {
        hj::int64_to_bytes(n, buf, true);
        int64_t m = hj::bytes_to_int64(buf, true);
        benchmark::DoNotOptimize(m);
    }
}
BENCHMARK(bm_int64_to_from)->Range(1, 1 << 8);

static void bm_float_to_from(benchmark::State &st)
{
    std::array<unsigned char, 4> buf = {0};
    float                        f   = 3.1415926f;
    for(auto _ : st)
    {
        hj::float_to_bytes(f, buf);
        float r = hj::bytes_to_float(buf);
        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(bm_float_to_from);

static void bm_double_to_from(benchmark::State &st)
{
    std::array<unsigned char, 8> buf = {0};
    double                       d   = 3.141592653589793;
    for(auto _ : st)
    {
        hj::double_to_bytes(d, buf);
        double r = hj::bytes_to_double(buf);
        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(bm_double_to_from);

static void bm_string_to_bytes_small(benchmark::State &st)
{
    std::array<unsigned char, 32> buf = {0};
    std::string                   s   = "hello world";
    for(auto _ : st)
    {
        hj::string_to_bytes(s, buf);
        std::string r = hj::bytes_to_string(buf, s.size());
        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(bm_string_to_bytes_small)->Iterations(1000);

static void bm_string_to_bytes_large(benchmark::State &st)
{
    const size_t               SZ = static_cast<size_t>(st.range(0));
    std::vector<unsigned char> buf(SZ);
    std::string                s(SZ, 'A');
    for(auto _ : st)
    {
        // reuse vector as trivially copyable container via data pointer
        // build a temporary array-like wrapper using std::span not used here
        // so we copy manually into vector memory
        std::memcpy(buf.data(), s.data(), SZ);
        std::string r(reinterpret_cast<const char *>(buf.data()), SZ);
        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(bm_string_to_bytes_large)->Arg(256)->Arg(1024)->Arg(8192);

static void bm_bytes_to_string_varlen(benchmark::State &st)
{
    const int                  N = static_cast<int>(st.range(0));
    std::vector<unsigned char> buf(N, 'x');
    for(auto _ : st)
    {
        std::string s = hj::bytes_to_string(buf, buf.size());
        benchmark::DoNotOptimize(s);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_bytes_to_string_varlen)->Arg(64)->Arg(512)->Arg(4096);
