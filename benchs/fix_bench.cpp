#include <benchmark/benchmark.h>

#include <hj/misc/fix.hpp>
#include <string>
#include <vector>

using namespace hj;

static void bm_fix_builder_basic(benchmark::State &state)
{
    for(auto _ : state)
    {
        hj::fix::builder b;
        b.begin();
        b.add_string(35, "D");
        b.add_string(49, "SENDER");
        b.add_string(56, "TARGET");
        b.add_int(34, 123);
        b.add_char(54, '1');
        b.end();
        auto s = b.str();
        benchmark::ClobberMemory();
    }
}

static void bm_fix_builder_external_buf(benchmark::State &state)
{
    for(auto _ : state)
    {
        char             buf[256] = {};
        hj::fix::builder b(buf, sizeof(buf));
        b.begin();
        b.add_string(35, "8");
        b.add_int(34, 999);
        b.end();
        auto s = b.str();
        benchmark::ClobberMemory();
    }
}

static void bm_fix_parser_only(benchmark::State &state)
{
    // prepare a sample message once
    hj::fix::builder b;
    b.begin();
    b.add_string(35, "D");
    b.add_string(49, "SENDER");
    b.add_string(56, "TARGET");
    b.add_int(34, 123);
    b.add_char(54, '1');
    b.end();
    std::string msg = b.str();

    for(auto _ : state)
    {
        hj::fix::parser p(msg.data(), msg.size());
        volatile bool   v = p.valid();
        volatile bool   c = p.complete();
        (void) v;
        (void) c;
        auto s35  = p.get_string(35);
        auto i34  = p.get_int<int>(34);
        auto ch54 = p.get_char(54);
        (void) s35;
        (void) i34;
        (void) ch54;
        benchmark::ClobberMemory();
    }
}

static void bm_fix_build_and_parse_roundtrip(benchmark::State &state)
{
    for(auto _ : state)
    {
        hj::fix::builder b;
        b.begin();
        b.add_string(35, "D");
        b.add_string(49, "SENDER");
        b.add_string(56, "TARGET");
        b.add_int(34, 123);
        b.add_char(54, '1');
        b.end();
        std::string msg = b.str();

        hj::fix::parser p(msg.data(), msg.size());
        volatile bool   v = p.valid();
        (void) v;
        benchmark::ClobberMemory();
    }
}

BENCHMARK(bm_fix_builder_basic);
BENCHMARK(bm_fix_builder_external_buf);
BENCHMARK(bm_fix_parser_only);
BENCHMARK(bm_fix_build_and_parse_roundtrip);
