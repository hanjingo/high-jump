#include <benchmark/benchmark.h>
#include <hj/encoding/protobuf.hpp>
#include "person.pb.h"
#include <sstream>
#include <string>

namespace
{

static test::Person make_person()
{
    test::Person p;
    p.set_name("benchmark-user");
    p.set_id(123456);
    p.set_email("bench@example.com");
    return p;
}

// Serialize to std::string
static void bm_pb_serialize_to_string(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        test::Person p = make_person();
        state.ResumeTiming();

        std::string out;
        auto        err = hj::pb::serialize(out, p);
        benchmark::DoNotOptimize(out);
        benchmark::DoNotOptimize(err);
    }
}

// Deserialize from std::string
static void bm_pb_deserialize_from_string(benchmark::State &state)
{
    state.PauseTiming();
    test::Person p = make_person();
    std::string  buf;
    hj::pb::serialize(buf, p);
    state.ResumeTiming();

    for(auto _ : state)
    {
        test::Person out;
        auto         err = hj::pb::deserialize(out, buf);
        benchmark::DoNotOptimize(out);
        benchmark::DoNotOptimize(err);
    }
}

// Serialize to ostream (stringstream)
static void bm_pb_serialize_to_ostream(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        test::Person p = make_person();
        state.ResumeTiming();

        std::ostringstream ss(std::ios::binary);
        auto               err = hj::pb::serialize(ss, p);
        benchmark::DoNotOptimize(ss.str());
        benchmark::DoNotOptimize(err);
    }
}

// Deserialize from istream (stringstream)
static void bm_pb_deserialize_from_istream(benchmark::State &state)
{
    state.PauseTiming();
    test::Person p = make_person();
    std::string  buf;
    hj::pb::serialize(buf, p);
    state.ResumeTiming();

    for(auto _ : state)
    {
        std::istringstream ss(buf, std::ios::binary);
        test::Person       out;
        auto               err = hj::pb::deserialize(out, ss);
        benchmark::DoNotOptimize(out);
        benchmark::DoNotOptimize(err);
    }
}

// Roundtrip serialize->deserialize using string
static void bm_pb_roundtrip(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        test::Person p = make_person();
        state.ResumeTiming();

        std::string  buf;
        auto         serr = hj::pb::serialize(buf, p);
        test::Person out;
        auto         derr = hj::pb::deserialize(out, buf);
        benchmark::DoNotOptimize(out);
        benchmark::DoNotOptimize(serr);
        benchmark::DoNotOptimize(derr);
    }
}

} // namespace

BENCHMARK(bm_pb_serialize_to_string)->Iterations(2000);
BENCHMARK(bm_pb_deserialize_from_string)->Iterations(2000);
BENCHMARK(bm_pb_serialize_to_ostream)->Iterations(1000);
BENCHMARK(bm_pb_deserialize_from_istream)->Iterations(1000);
BENCHMARK(bm_pb_roundtrip)->Iterations(1000);
