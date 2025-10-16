#include <benchmark/benchmark.h>
#include <hj/encoding/endian.hpp>

static void bm_is_big_endian(benchmark::State &st)
{
    for(auto _ : st)
    {
        volatile bool be = hj::is_big_endian();
        benchmark::DoNotOptimize(be);
    }
}
BENCHMARK(bm_is_big_endian)->Iterations(100000);

static void bm_to_big_endian_16(benchmark::State &st)
{
    uint16_t v = 0x1234;
    for(auto _ : st)
    {
        auto r = hj::to_big_endian<uint16_t>(v);
        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(bm_to_big_endian_16)->Iterations(200000);

static void bm_to_big_endian_32(benchmark::State &st)
{
    uint32_t v = 0x01020304u;
    for(auto _ : st)
    {
        auto r = hj::to_big_endian<uint32_t>(v);
        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(bm_to_big_endian_32)->Iterations(200000);

static void bm_to_big_endian_64(benchmark::State &st)
{
    uint64_t v = 0x0102030405060708ULL;
    for(auto _ : st)
    {
        auto r = hj::to_big_endian<uint64_t>(v);
        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(bm_to_big_endian_64)->Iterations(100000);

static void bm_to_little_endian_16(benchmark::State &st)
{
    uint16_t v = 0x1234;
    for(auto _ : st)
    {
        auto r = hj::to_little_endian<uint16_t>(v);
        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(bm_to_little_endian_16)->Iterations(200000);

static void bm_to_little_endian_32(benchmark::State &st)
{
    uint32_t v = 0x01020304u;
    for(auto _ : st)
    {
        auto r = hj::to_little_endian<uint32_t>(v);
        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(bm_to_little_endian_32)->Iterations(200000);

static void bm_to_little_endian_64(benchmark::State &st)
{
    uint64_t v = 0x0102030405060708ULL;
    for(auto _ : st)
    {
        auto r = hj::to_little_endian<uint64_t>(v);
        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(bm_to_little_endian_64)->Iterations(100000);

// Round-trip: convert to big-endian then back to host (using to_little on big-endian
// platforms will swap back). This is a lightweight correctness/overhead check.
static void bm_roundtrip_32(benchmark::State &st)
{
    uint32_t v = 0x0A0B0C0Du;
    for(auto _ : st)
    {
        auto be   = hj::to_big_endian<uint32_t>(v);
        auto host = hj::to_little_endian<uint32_t>(be);
        benchmark::DoNotOptimize(host);
    }
}
BENCHMARK(bm_roundtrip_32)->Iterations(150000);
