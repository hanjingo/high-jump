#include <benchmark/benchmark.h>

#include <hj/encoding/flatbuffers.hpp>
#include <string>
#include "monster_generated.h"

using namespace hj;

static void bm_flatbuffer_build(benchmark::State &state)
{
    for(auto _ : state)
    {
        hj::flatbuffer_builder builder;
        auto                   name = builder.CreateString("Dragon");
        auto monster = MyGame::CreateMonster(builder, 1, name, 100, 35.5f);
        builder.Finish(monster);
        volatile auto size = builder.GetSize();
        (void) size;
        benchmark::ClobberMemory();
    }
}

static void bm_flatbuffer_parse(benchmark::State &state)
{
    // prepare a buffer once
    hj::flatbuffer_builder builder;
    auto                   name = builder.CreateString("Dragon");
    auto monster = MyGame::CreateMonster(builder, 1, name, 100, 35.5f);
    builder.Finish(monster);
    uint8_t *buf  = builder.GetBufferPointer();
    size_t   size = builder.GetSize();

    for(auto _ : state)
    {
        auto           dmonster = MyGame::GetMonster(buf);
        volatile int   id       = dmonster->id();
        volatile auto  n        = dmonster->name()->str();
        volatile int   hp       = dmonster->hp();
        volatile float dmg      = dmonster->damage();
        (void) id;
        (void) n;
        (void) hp;
        (void) dmg;
        benchmark::ClobberMemory();
    }
}

static void bm_flatbuffer_roundtrip(benchmark::State &state)
{
    for(auto _ : state)
    {
        hj::flatbuffer_builder builder;
        auto                   name = builder.CreateString("Dragon");
        auto monster = MyGame::CreateMonster(builder, 1, name, 100, 35.5f);
        builder.Finish(monster);
        uint8_t *buf = builder.GetBufferPointer();

        auto         dmonster = MyGame::GetMonster(buf);
        volatile int id       = dmonster->id();
        (void) id;
        benchmark::ClobberMemory();
    }
}

BENCHMARK(bm_flatbuffer_build);
BENCHMARK(bm_flatbuffer_parse);
BENCHMARK(bm_flatbuffer_roundtrip);
