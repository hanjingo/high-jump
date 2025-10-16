#include <benchmark/benchmark.h>
#include <hj/hardware/disk.h>
#include <vector>
#include <string>
#include <cstring>

static void bm_disk_format_size_small(benchmark::State &st)
{
    char buf[64];
    for(auto _ : st)
    {
        auto r = disk_format_size(512, buf, sizeof(buf));
        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(bm_disk_format_size_small)->Iterations(5000);

static void bm_disk_format_size_large(benchmark::State &st)
{
    char buf[128];
    for(auto _ : st)
    {
        auto r =
            disk_format_size(1024ULL * 1024 * 1024 * 5ULL, buf, sizeof(buf));
        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(bm_disk_format_size_large)->Iterations(2000);

static void bm_disk_format_size_small_buffer_fail(benchmark::State &st)
{
    char buf[4];
    for(auto _ : st)
    {
        auto r = disk_format_size(1024ULL * 1024 * 1024, buf, sizeof(buf));
        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(bm_disk_format_size_small_buffer_fail)->Iterations(2000);

static void bm_filesystem_from_string_common(benchmark::State &st)
{
    const char *names[] =
        {"ext4", "NTFS", "FAT32", "exFAT", "btrfs", "APFS", "unknown"};
    for(auto _ : st)
    {
        for(auto n : names)
        {
            auto t = disk_filesystem_type_from_string(n);
            benchmark::DoNotOptimize(t);
        }
    }
}
BENCHMARK(bm_filesystem_from_string_common)->Iterations(2000);

static void bm_filesystem_to_string_enum(benchmark::State &st)
{
    for(auto _ : st)
    {
        for(int i = 0; i <= FILESYSTEM_ZFS; ++i)
        {
            auto s = disk_filesystem_type_to_string(
                static_cast<filesystem_type_t>(i));
            benchmark::DoNotOptimize(s);
        }
    }
}
BENCHMARK(bm_filesystem_to_string_enum)->Iterations(2000);
