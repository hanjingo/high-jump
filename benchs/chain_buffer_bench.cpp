#include <benchmark/benchmark.h>
#include <hj/io/chain_buffer.hpp>

#include <string>
#include <vector>
#include <cstring>

using hj::chain_buffer;

// Append many small chunks into the chain buffer.
static void bm_append_many_small(benchmark::State &st)
{
    const size_t      append_count = static_cast<size_t>(st.range(0));
    const std::string chunk(64, 'x');
    chain_buffer      buf(1024);

    for(auto _ : st)
    {
        buf.clear();
        for(size_t i = 0; i < append_count; ++i)
            buf.append(chunk.data(), chunk.size());
        benchmark::DoNotOptimize(buf.size());
    }
}
BENCHMARK(bm_append_many_small)->Arg(100)->Arg(1000)->Arg(10000);

// Append one large block repeatedly (measures large memcpy behavior).
static void bm_append_large_block(benchmark::State &st)
{
    const size_t         block_size = static_cast<size_t>(st.range(0));
    std::vector<uint8_t> data(block_size, 0xAA);
    chain_buffer         buf(4096);

    for(auto _ : st)
    {
        buf.clear();
        buf.append(data.data(), data.size());
        benchmark::DoNotOptimize(buf.size());
    }
}
BENCHMARK(bm_append_large_block)->Arg(256)->Arg(4096)->Arg(65536);

// Fill buffer and repeatedly read small chunks (read is const) then consume.
static void bm_read_and_consume(benchmark::State &st)
{
    const size_t         total_size = static_cast<size_t>(st.range(0));
    chain_buffer         buf(4096);
    std::vector<uint8_t> data(total_size, 'd');
    buf.append(data.data(), data.size());

    std::vector<char> out(512);
    for(auto _ : st)
    {
        // read then consume in small steps until empty
        while(buf.size() > 0)
        {
            size_t n = buf.read(out.data(), out.size());
            benchmark::DoNotOptimize(n);
            size_t c = buf.consume(n);
            benchmark::DoNotOptimize(c);
        }
        // refill for next iteration
        buf.append(data.data(), data.size());
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_read_and_consume)->Arg(1024)->Arg(16384)->Arg(131072);

// Append one chain_buffer into another repeatedly (measures move/append path).
static void bm_append_chain_buffer(benchmark::State &st)
{
    const size_t         src_size = static_cast<size_t>(st.range(0));
    chain_buffer         src(1024);
    std::vector<uint8_t> data(src_size, 0x55);
    src.append(data.data(), data.size());

    for(auto _ : st)
    {
        chain_buffer dst(1024);
        // append the same src multiple times to stress internal path
        for(int i = 0; i < 16; ++i)
            dst.append(src);
        benchmark::DoNotOptimize(dst.size());
    }
}
BENCHMARK(bm_append_chain_buffer)->Arg(64)->Arg(512)->Arg(4096);

// Clear and reuse pattern: append, clear, append again
static void bm_clear_and_reuse(benchmark::State &st)
{
    const size_t         chunk = 256;
    std::vector<uint8_t> data(chunk, 0x99);
    chain_buffer         buf(1024);
    for(auto _ : st)
    {
        buf.append(data.data(), data.size());
        benchmark::DoNotOptimize(buf.size());
        buf.clear();
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_clear_and_reuse);

// Consume that forces block erasure (append many small then consume large)
static void bm_consume_block_erasure(benchmark::State &st)
{
    const size_t total = static_cast<size_t>(st.range(0));
    chain_buffer buf(1024);
    std::string  chunk(256, 'z');
    size_t       repeats = total / chunk.size();
    for(size_t i = 0; i < repeats; ++i)
        buf.append(chunk.data(), chunk.size());

    for(auto _ : st)
    {
        // consume more than one block to trigger erase path
        size_t to_consume = buf.size();
        size_t c          = buf.consume(to_consume);
        benchmark::DoNotOptimize(c);
        // rebuild
        for(size_t i = 0; i < repeats; ++i)
            buf.append(chunk.data(), chunk.size());
    }
}
BENCHMARK(bm_consume_block_erasure)->Arg(1024)->Arg(8192)->Arg(65536);
