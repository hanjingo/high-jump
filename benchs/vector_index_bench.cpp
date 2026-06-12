#include <benchmark/benchmark.h>
#include <hj/ai/vector_index.hpp>

#include <array>
#include <cstdio>
#include <string>

namespace
{

constexpr std::array<float, 6> kVectors = {
    1.0f,
    0.0f,
    0.0f,
    1.0f,
    1.0f,
    1.0f,
};

constexpr std::array<float, 2> kQuery = {
    1.0f,
    0.0f,
};

constexpr std::array<float, 4> kQueries = {
    1.0f,
    0.0f,
    0.0f,
    1.0f,
};

} // namespace

static void bm_vector_index_build(benchmark::State &state)
{
    for(auto _ : state)
    {
        hj::vector_index<hj::vindex_flat_l2_t> index;
        index.build(2);
        benchmark::DoNotOptimize(index.total());
    }
}
BENCHMARK(bm_vector_index_build)->Iterations(1000);

static void bm_vector_index_add(benchmark::State &state)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    index.build(2);

    for(auto _ : state)
    {
        index.reset();
        index.add(3, kVectors.data());
        benchmark::DoNotOptimize(index.total());
    }
}
BENCHMARK(bm_vector_index_add)->Iterations(1000);

static void bm_vector_index_search(benchmark::State &state)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    index.build(2);
    index.add(3, kVectors.data());

    std::array<float, 2>        distances{};
    std::array<faiss::idx_t, 2> indices{};

    for(auto _ : state)
    {
        index.search(1, kQuery.data(), 2, distances.data(), indices.data());
        benchmark::DoNotOptimize(distances.data());
        benchmark::DoNotOptimize(indices.data());
    }
}
BENCHMARK(bm_vector_index_search)->Iterations(1000);

static void bm_vector_index_range_search(benchmark::State &state)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    index.build(2);
    index.add(3, kVectors.data());

    for(auto _ : state)
    {
        hj::vindex_range_search_result_t result(1, true);
        index.range_search(1, kQuery.data(), 1.1f, &result);
        benchmark::DoNotOptimize(result.lims);
    }
}
BENCHMARK(bm_vector_index_range_search)->Iterations(1000);

static void bm_vector_index_multiple_queries(benchmark::State &state)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    index.build(2);
    index.add(3, kVectors.data());

    std::array<float, 4>        distances{};
    std::array<faiss::idx_t, 4> indices{};

    for(auto _ : state)
    {
        index.search(2, kQueries.data(), 2, distances.data(), indices.data());
        benchmark::DoNotOptimize(distances.data());
        benchmark::DoNotOptimize(indices.data());
    }
}
BENCHMARK(bm_vector_index_multiple_queries)->Iterations(1000);

static void bm_vector_index_save(benchmark::State &state)
{
    hj::vector_index<hj::vindex_flat_l2_t> index;
    index.build(2);
    index.add(3, kVectors.data());

    for(auto _ : state)
    {
        const std::string filename =
            "./vector_index_bench_save_" + std::to_string(state.iterations())
            + "_" + std::to_string(state.range(0)) + ".faissindex";

        index.save(filename.c_str());
        benchmark::DoNotOptimize(filename);
        std::remove(filename.c_str());
    }
}
BENCHMARK(bm_vector_index_save)->Arg(1)->Iterations(100);

static void bm_vector_index_load(benchmark::State &state)
{
    const std::string filename = "./vector_index_bench_load.faissindex";

    {
        hj::vector_index<hj::vindex_flat_l2_t> src;
        src.build(2);
        src.add(3, kVectors.data());
        src.save(filename.c_str());
    }

    for(auto _ : state)
    {
        hj::vector_index<hj::vindex_flat_l2_t> dst;
        const bool loaded = dst.load(filename.c_str());
        benchmark::DoNotOptimize(loaded);
        benchmark::DoNotOptimize(dst.total());
    }

    std::remove(filename.c_str());
}
BENCHMARK(bm_vector_index_load)->Iterations(1000);