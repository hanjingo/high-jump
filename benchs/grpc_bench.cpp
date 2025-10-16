#include <benchmark/benchmark.h>

#ifdef GRPC_ENABLE
#include <hj/net/grpc.hpp>
#include <grpcpp/grpcpp.h>
#endif

#include <string>

#ifdef GRPC_ENABLE
static void bm_grpc_channel_connect(benchmark::State &state)
{
    for(auto _ : state)
    {
        hj::grpc_channel ch;
        bool             ok = ch.connect("127.0.0.1:50051");
        (void) ok;
        benchmark::ClobberMemory();
    }
}

static void bm_grpc_server_start_stop(benchmark::State &state)
{
    for(auto _ : state)
    {
        grpc::Service   svc; // base service - no RPCs
        hj::grpc_server server;
        bool            started = server.start("127.0.0.1:0", &svc);
        if(started)
            server.stop();
        benchmark::ClobberMemory();
    }
}

BENCHMARK(bm_grpc_channel_connect);
BENCHMARK(bm_grpc_server_start_stop);
#else
// GRPC not enabled: provide tiny no-op benchmarks so file compiles.
static void bm_grpc_noop(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::ClobberMemory();
    }
}

BENCHMARK(bm_grpc_noop);
#endif