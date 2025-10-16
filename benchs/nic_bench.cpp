#include <benchmark/benchmark.h>
#include <hj/hardware/nic.h>
#include <vector>
#include <cstring>
#include <cstdlib>

static void bm_nic_init_cleanup(benchmark::State &state)
{
    for(auto _ : state)
    {
        nic_error_t r = nic_init();
        benchmark::DoNotOptimize(r);
        nic_cleanup();
    }
}

static void bm_nic_get_interface_count(benchmark::State &state)
{
    for(auto _ : state)
    {
        uint32_t    cnt = 0;
        nic_error_t r   = nic_get_interface_count(&cnt);
        benchmark::DoNotOptimize(r);
        benchmark::DoNotOptimize(cnt);
    }
}

static void bm_nic_enumerate_interfaces(benchmark::State &state)
{
    for(auto _ : state)
    {
        nic_info_t  ifs[NIC_MAX_INTERFACES];
        uint32_t    actual = 0;
        nic_error_t r =
            nic_enumerate_interfaces(ifs, NIC_MAX_INTERFACES, &actual);
        benchmark::DoNotOptimize(r);
        benchmark::DoNotOptimize(actual);
    }
}

static void bm_nic_get_interface_info(benchmark::State &state)
{
    // prepare one interface name if available (not timed)
    nic_info_t ifs[NIC_MAX_INTERFACES];
    uint32_t   actual = 0;
    nic_enumerate_interfaces(ifs, NIC_MAX_INTERFACES, &actual);
    const char *name = (actual > 0) ? ifs[0].name : "lo";

    for(auto _ : state)
    {
        nic_info_t  info;
        nic_error_t r = nic_get_interface_info(name, &info);
        benchmark::DoNotOptimize(r);
        benchmark::DoNotOptimize(info.index);
    }
}

static void bm_nic_get_statistics(benchmark::State &state)
{
    nic_info_t ifs[NIC_MAX_INTERFACES];
    uint32_t   actual = 0;
    nic_enumerate_interfaces(ifs, NIC_MAX_INTERFACES, &actual);
    const char *name = (actual > 0) ? ifs[0].name : "lo";

    for(auto _ : state)
    {
        nic_statistics_t stats;
        nic_error_t      r = nic_get_statistics(name, &stats);
        benchmark::DoNotOptimize(r);
        benchmark::DoNotOptimize(stats.bytes_received);
    }
}

static void bm_nic_enable_disable_interface(benchmark::State &state)
{
    // Enabling/disabling interfaces is intrusive; require explicit opt-in
    const char *allow = std::getenv("HJ_BENCH_ALLOW_HARDWARE");
    if(!allow)
    {
        state.counters["hardware_skipped"] = 1;
        return;
    }

    nic_info_t ifs[NIC_MAX_INTERFACES];
    uint32_t   actual = 0;
    nic_enumerate_interfaces(ifs, NIC_MAX_INTERFACES, &actual);
    if(actual == 0)
    {
        state.counters["no_interface"] = 1;
        return;
    }

    const char *name = ifs[0].name;

    for(auto _ : state)
    {
        nic_error_t r1 = nic_disable_interface(name);
        benchmark::DoNotOptimize(r1);
        nic_error_t r2 = nic_enable_interface(name);
        benchmark::DoNotOptimize(r2);
    }
}

BENCHMARK(bm_nic_init_cleanup)->Iterations(2000);
BENCHMARK(bm_nic_get_interface_count)->Iterations(5000);
BENCHMARK(bm_nic_enumerate_interfaces)->Iterations(2000);
BENCHMARK(bm_nic_get_interface_info)->Iterations(2000);
BENCHMARK(bm_nic_get_statistics)->Iterations(2000);
BENCHMARK(bm_nic_enable_disable_interface)->Iterations(200);