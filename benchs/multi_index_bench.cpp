#include <benchmark/benchmark.h>
#include <hj/algo/multi_index.hpp>
#include <string>
#include <vector>
#include <random>

HJ_INDEX_TAG(id_tag)
HJ_INDEX_TAG(name_tag)

struct person
{
    int         id;
    std::string name;
    int         age;
};

using person_index =
    hj::multi_index<person,
                    HJ_UNIQUE_INDEX(int, &person::id, id_tag)
                        HJ_NON_UNIQUE_INDEX(
                            std::string, &person::name, name_tag)>;

static void bm_multi_index_insert_find(benchmark::State &state)
{
    const int N = 5000;
    for(auto _ : state)
    {
        person_index idx;
        // prepare data (not measured very strictly here)
        for(int i = 0; i < N; ++i)
        {
            idx.insert(person{i,
                              std::string("name_") + std::to_string(i % 50),
                              i % 100});
        }

        // unique lookup
        auto &id_view = idx.get<id_tag>();
        auto  it      = id_view.find(N / 2);
        benchmark::DoNotOptimize(it);
    }
}

static void bm_multi_index_equal_range(benchmark::State &state)
{
    const int    N = 5000;
    person_index idx;
    for(int i = 0; i < N; ++i)
        idx.insert(
            person{i, std::string("name_") + std::to_string(i % 100), i % 100});

    for(auto _ : state)
    {
        auto &name_view = idx.get<name_tag>();
        auto  range     = name_view.equal_range("name_42");
        int   count     = 0;
        for(auto it = range.first; it != range.second; ++it)
            ++count;
        benchmark::DoNotOptimize(count);
    }
}

static void bm_multi_index_erase_modify(benchmark::State &state)
{
    const int N = 2000;
    for(auto _ : state)
    {
        person_index idx;
        for(int i = 0; i < N; ++i)
            idx.insert(
                person{i, std::string("nm") + std::to_string(i % 20), i});

        auto &id_view = idx.get<id_tag>();
        // erase some
        for(int i = 0; i < 100; ++i)
            id_view.erase(i);

        // modify some
        auto it = id_view.find(150);
        if(it != id_view.end())
            id_view.modify(it, [](person &p) { p.age += 10; });

        benchmark::DoNotOptimize(idx.size());
    }
}

static void bm_multi_index_unique_constraint(benchmark::State &state)
{
    for(auto _ : state)
    {
        person_index idx;
        auto         r1 = idx.insert(person{1, "A", 10});
        auto         r2 = idx.insert(person{1, "B", 20});
        benchmark::DoNotOptimize(r1.second);
        benchmark::DoNotOptimize(r2.second);
    }
}

BENCHMARK(bm_multi_index_insert_find)->Iterations(200);
BENCHMARK(bm_multi_index_equal_range)->Iterations(500);
BENCHMARK(bm_multi_index_erase_modify)->Iterations(200);
BENCHMARK(bm_multi_index_unique_constraint)->Iterations(10000);