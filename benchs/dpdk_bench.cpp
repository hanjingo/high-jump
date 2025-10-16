#include <benchmark/benchmark.h>
#include <hj/hardware/dpdk.h>
#include <cstring>

// Benchmark that simply checks whether DPDK support was compiled in
static void bm_dpdk_compiled_flag(benchmark::State &st)
{
    for(auto _ : st)
    {
#if defined(DPDK_ENABLE)
        volatile bool enabled = true;
#else
        volatile bool enabled = false;
#endif
        benchmark::DoNotOptimize(enabled);
    }
}
BENCHMARK(bm_dpdk_compiled_flag)->Iterations(100000);

// Simulate a tiny packet processing loop (memcpy) to approximate per-packet work
static void bm_simulated_packet_processing(benchmark::State &st)
{
    constexpr size_t pkt_size = 64;
    uint8_t          src[pkt_size];
    uint8_t          dst[pkt_size];
    // fill source
    for(size_t i = 0; i < pkt_size; ++i)
        src[i] = static_cast<uint8_t>(i);

    for(auto _ : st)
    {
        // simulate processing N packets per iteration
        for(int i = 0; i < 16; ++i)
            memcpy(dst, src, pkt_size);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_simulated_packet_processing)->Iterations(2000);
