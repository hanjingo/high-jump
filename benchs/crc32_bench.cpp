#include <benchmark/benchmark.h>
#include <hj/algo/crc32.h>
#include <cstring>
#include <string>

static void bm_crc32_standard_test_vector(benchmark::State &state)
{
    const char *test = "123456789";
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(crc32(test, strlen(test), 0));
    }
}
BENCHMARK(bm_crc32_standard_test_vector);

static void bm_crc32_large_text(benchmark::State &state)
{
    const char *big_data =
        "The class everyone had really been looking forward to was Defence "
        "Against the Dark Arts, but Quirrell's lessons turned out to be a bit "
        "of a joke.His classroom smelled strongly of garlic, which everyone "
        "said was to ward off a vampire he'd met in Romania, and was afraid "
        "would be coming back to get him one of these days.His turban, he told "
        "them, had been given to him by an African prince as a thank-you for "
        "getting rid of a troublesome zombie, but they weren't sure they "
        "believed this story.For one thing, when Seamus Finnegan asked eagerly "
        "to hear how Quirrell had fought off the zombie, Quirrell went pink "
        "and sta";
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(crc32(big_data, strlen(big_data), 0));
    }
}
BENCHMARK(bm_crc32_large_text);

static void bm_crc32_incremental(benchmark::State &state)
{
    const char *part1 = "Hello, ";
    const char *part2 = "World!";
    for(auto _ : state)
    {
        uint32_t crc1 = crc32(part1, strlen(part1), 0);
        benchmark::DoNotOptimize(crc32(part2, strlen(part2), crc1));
    }
}
BENCHMARK(bm_crc32_incremental);

static void bm_crc32_edge_cases(benchmark::State &state)
{
    const char *data = "test";
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(crc32(data, 0, 0));
    }
}
BENCHMARK(bm_crc32_edge_cases);

static void bm_crc32_consistency(benchmark::State &state)
{
    const char *test_data = "The quick brown fox jumps over the lazy dog";
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(crc32(test_data, strlen(test_data), 0));
    }
}
BENCHMARK(bm_crc32_consistency);

static void bm_crc32_high_concurrency(benchmark::State &state)
{
    const std::string base = "abcdefghijklmnopqrstuvwxyz0123456789";
    for(auto _ : state)
    {
        for(int i = 0; i < 1000; ++i)
        {
            std::string s = base + std::to_string(i);
            benchmark::DoNotOptimize(crc32(s.c_str(), s.size(), 0));
        }
    }
}
BENCHMARK(bm_crc32_high_concurrency);