#include <benchmark/benchmark.h>
#include <hj/net/tcp.hpp>
#include <thread>
#include <chrono>
#include <cstdlib>

using namespace hj;

// Construct / destructor cost
static void bm_tcp_conn_construct(benchmark::State &state)
{
    for(auto _ : state)
    {
        tcp_conn::io_t io;
        tcp_conn       conn{io};
        benchmark::DoNotOptimize(conn);
    }
}
BENCHMARK(bm_tcp_conn_construct)->Iterations(10000);

// Flag set/get ops
static void bm_tcp_conn_flag_ops(benchmark::State &state)
{
    for(auto _ : state)
    {
        tcp_conn::io_t io;
        tcp_conn       conn{io};
        conn.set_flag(0x1);
        conn.set_flag(0x2);
        auto f = conn.get_flag();
        benchmark::DoNotOptimize(f);
        conn.unset_flag(0x2);
        f = conn.get_flag();
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(bm_tcp_conn_flag_ops)->Iterations(100000);

// Toggle w/r closed
static void bm_tcp_conn_w_r_closed(benchmark::State &state)
{
    for(auto _ : state)
    {
        tcp_conn::io_t io;
        tcp_conn       conn{io};
        conn.set_w_closed(true);
        bool w = conn.is_w_closed();
        benchmark::DoNotOptimize(w);
        conn.set_w_closed(false);
        conn.set_r_closed(true);
        bool r = conn.is_r_closed();
        benchmark::DoNotOptimize(r);
        conn.set_r_closed(false);
    }
}
BENCHMARK(bm_tcp_conn_w_r_closed)->Iterations(100000);

// Guarded network connect benchmark. Disabled by default; enable with
// HJ_BENCH_ALLOW_NET=1 in the environment.
static void bm_tcp_conn_connect_guarded(benchmark::State &state)
{
    if(std::getenv("HJ_BENCH_ALLOW_NET") == nullptr)
    {
        state.SkipWithError(
            "network benchmarks disabled; set HJ_BENCH_ALLOW_NET=1 to enable");
        return;
    }

    const int port = 20020;
    for(auto _ : state)
    {
        state.PauseTiming();
        // start a server thread that accepts one connection and then closes
        std::thread server([port]() {
            tcp_conn::io_t io;
            tcp_listener   li{io};
            auto           base = li.accept(port);
            if(base)
            {
                // keep the connection alive briefly
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            li.close();
        });

        // give server time to start
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

        tcp_conn::io_t io;
        tcp_conn       conn{io};
        state.ResumeTiming();

        bool ok = conn.connect("127.0.0.1", static_cast<std::uint16_t>(port));
        benchmark::DoNotOptimize(ok);

        // cleanup
        conn.disconnect();
        state.PauseTiming();
        if(server.joinable())
            server.join();
        state.ResumeTiming();
    }
}
BENCHMARK(bm_tcp_conn_connect_guarded)->Iterations(100);
