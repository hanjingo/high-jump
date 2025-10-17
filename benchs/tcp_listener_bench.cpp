#include <benchmark/benchmark.h>
#include <hj/net/tcp.hpp>
#include <thread>
#include <chrono>
#include <cstdlib>

using namespace hj;

// Construct/destruct
static void bm_tcp_listener_construct(benchmark::State &state)
{
    for(auto _ : state)
    {
        tcp_listener::io_t io;
        tcp_listener       li{io};
        benchmark::DoNotOptimize(li);
    }
}
BENCHMARK(bm_tcp_listener_construct)->Iterations(10000);

// is_closed
static void bm_tcp_listener_is_closed(benchmark::State &state)
{
    for(auto _ : state)
    {
        tcp_listener::io_t io;
        tcp_listener       li{io};
        bool               c1 = li.is_closed();
        benchmark::DoNotOptimize(c1);
        li.close();
        bool c2 = li.is_closed();
        benchmark::DoNotOptimize(c2);
    }
}
BENCHMARK(bm_tcp_listener_is_closed)->Iterations(100000);

// set_option requires an acceptor to be created; use async_accept to bootstrap
static void bm_tcp_listener_set_option(benchmark::State &state)
{
    // This involves network resources; guard by env var
    if(std::getenv("HJ_BENCH_ALLOW_NET") == nullptr)
    {
        state.SkipWithError(
            "network benchmarks disabled; set HJ_BENCH_ALLOW_NET=1 to enable");
        return;
    }

    const int port = 13000;
    for(auto _ : state)
    {
        state.PauseTiming();
        tcp_listener::io_t io;
        tcp_listener       li{io};

        // start a server thread to call async_accept so acceptor is created
        std::thread server([&]() {
            // accept once and close
            auto sock = li.accept(port);
            if(sock)
                sock->close();
            li.close();
        });

        // wait for server to set up
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        state.ResumeTiming();

        // before acceptor created, set_option may be false; after acceptor it should be true
        bool ok_before = li.set_option(tcp_socket::opt_reuse_addr(true));
        benchmark::DoNotOptimize(ok_before);

        // let server accept
        state.PauseTiming();
        if(server.joinable())
            server.join();
        state.ResumeTiming();

        bool ok_after = li.set_option(tcp_socket::opt_reuse_addr(true));
        benchmark::DoNotOptimize(ok_after);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_tcp_listener_set_option)->Iterations(50);

// accept (guarded) - sync accept with a client connecting
static void bm_tcp_listener_accept_guarded(benchmark::State &state)
{
    if(std::getenv("HJ_BENCH_ALLOW_NET") == nullptr)
    {
        state.SkipWithError(
            "network benchmarks disabled; set HJ_BENCH_ALLOW_NET=1 to enable");
        return;
    }

    const int port = 13001;
    for(auto _ : state)
    {
        state.PauseTiming();
        tcp_listener::io_t io;
        tcp_listener       li{io};

        // start client thread that connects to listener
        std::thread client([port]() {
            tcp_socket::io_t io_c;
            tcp_socket       sock{io_c};
            // wait briefly to allow acceptor setup
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            sock.connect("127.0.0.1", static_cast<uint16_t>(port));
            sock.close();
        });

        // give server a moment
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        state.ResumeTiming();

        auto s = li.accept(port);
        benchmark::DoNotOptimize(s);

        state.PauseTiming();
        if(client.joinable())
            client.join();
        state.ResumeTiming();

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_tcp_listener_accept_guarded)->Iterations(50);

// async_accept (guarded)
static void bm_tcp_listener_async_accept_guarded(benchmark::State &state)
{
    if(std::getenv("HJ_BENCH_ALLOW_NET") == nullptr)
    {
        state.SkipWithError(
            "network benchmarks disabled; set HJ_BENCH_ALLOW_NET=1 to enable");
        return;
    }

    const int port = 13002;
    for(auto _ : state)
    {
        state.PauseTiming();
        tcp_listener::io_t io;
        tcp_listener       li{io};
        int                called = 0;

        // prepare async accept
        li.async_accept(port,
                        [&called](const tcp_listener::err_t  &err,
                                  std::shared_ptr<tcp_socket> sock) {
                            (void) err;
                            if(sock)
                                ++called;
                        });

        // client thread
        std::thread client([port]() {
            tcp_socket::io_t io_c;
            tcp_socket       sock{io_c};
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            sock.connect("127.0.0.1", static_cast<uint16_t>(port));
            sock.close();
        });

        state.ResumeTiming();
        io.run();

        state.PauseTiming();
        if(client.joinable())
            client.join();
        benchmark::DoNotOptimize(called);
        state.ResumeTiming();

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_tcp_listener_async_accept_guarded)->Iterations(50);

// close repeated
static void bm_tcp_listener_close(benchmark::State &state)
{
    for(auto _ : state)
    {
        tcp_listener::io_t io;
        tcp_listener       li{io};
        li.close();
        li.close();
        benchmark::DoNotOptimize(li.is_closed());
    }
}
BENCHMARK(bm_tcp_listener_close)->Iterations(10000);
