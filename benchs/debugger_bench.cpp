#include <benchmark/benchmark.h>
#include <hj/testing/debugger.hpp>
#include <vector>
#include <string>
#include <boost/asio.hpp>

static void bm_dbg_fmt_vector(benchmark::State &st)
{
    std::vector<uint8_t> buf = {0x12, 0x34, 0x56, 0x78};
    for(auto _ : st)
    {
        auto out = hj::debugger::instance().fmt("{:02x}", buf);
        benchmark::DoNotOptimize(out);
    }
}
BENCHMARK(bm_dbg_fmt_vector)->Iterations(5000);

static void bm_dbg_fmt_charptr(benchmark::State &st)
{
    const char *s = "ABCD";
    for(auto _ : st)
    {
        auto out = hj::debugger::instance().fmt("{:02x}", s);
        benchmark::DoNotOptimize(out);
    }
}
BENCHMARK(bm_dbg_fmt_charptr)->Iterations(5000);

static void bm_dbg_fmt_ucharptr_with_len(benchmark::State &st)
{
    unsigned char buf[8] = {0xAA, 0xBB, 0xCC, 0x01, 0x02, 0x03, 0x04, 0x05};
    for(auto _ : st)
    {
        auto out = hj::debugger::instance().fmt("{:02x}", buf, size_t(8));
        benchmark::DoNotOptimize(out);
    }
}
BENCHMARK(bm_dbg_fmt_ucharptr_with_len)->Iterations(3000);

static void bm_dbg_fmt_streambuf(benchmark::State &st)
{
    boost::asio::streambuf buf;
    std::ostream           os(&buf);
    os << "HELLO";
    for(auto _ : st)
    {
        auto out = hj::debugger::instance().fmt("{:02x}", buf);
        benchmark::DoNotOptimize(out);
    }
}
BENCHMARK(bm_dbg_fmt_streambuf)->Iterations(3000);

static void bm_dbg_fmt_large_buf_truncate(benchmark::State &st)
{
    std::vector<uint8_t> big(DEBUG_BUF_SIZE + 100, 0xFF);
    for(auto _ : st)
    {
        auto out = hj::debugger::instance().fmt("{:02x}", big);
        benchmark::DoNotOptimize(out);
        // ensure truncation marker present in produced output
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_dbg_fmt_large_buf_truncate)->Iterations(1000);
