#include <benchmark/benchmark.h>
#include <hj/net/tcp.hpp>
#include <thread>
#include <chrono>
#include <cstdlib>

using namespace hj;

// Construct/destruct cost
static void bm_tcp_dialer_construct(benchmark::State &state)
{
    for(auto _ : state)
    {
        tcp_dialer::io_t io;
        tcp_dialer       d{io};
        benchmark::DoNotOptimize(d);
    }
}
BENCHMARK(bm_tcp_dialer_construct)->Iterations(10000);

// Guarded sync dial benchmark (requires a server). Enable by setting
// environment variable HJ_BENCH_ALLOW_NET=1
static void bm_tcp_dialer_dial_guarded(benchmark::State &state)
{
    if(std::getenv("HJ_BENCH_ALLOW_NET") == nullptr)
    {
        state.SkipWithError(
            "network benchmarks disabled; set HJ_BENCH_ALLOW_NET=1 to enable");
        return;
    }

    const int port = 12000;
    for(auto _ : state)
    {
        state.PauseTiming();
        std::thread server([port]() {
            tcp_socket::io_t io;
            tcp_listener     li{io};
            auto             sock = li.accept(port);
            if(sock)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                sock->close();
            }
            li.close();
        });

        // give server time to start
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        tcp_dialer::io_t io;
        tcp_dialer       dialer{io};
        state.ResumeTiming();

        auto s = dialer.dial("127.0.0.1", static_cast<std::uint16_t>(port));
        benchmark::DoNotOptimize(s);

        state.PauseTiming();
        if(server.joinable())
            server.join();
        state.ResumeTiming();
    }
}
BENCHMARK(bm_tcp_dialer_dial_guarded)->Iterations(100);

// Guarded async_dial benchmark
static void bm_tcp_dialer_async_dial_guarded(benchmark::State &state)
{
    if(std::getenv("HJ_BENCH_ALLOW_NET") == nullptr)
    {
        state.SkipWithError(
            "network benchmarks disabled; set HJ_BENCH_ALLOW_NET=1 to enable");
        return;
    }

    const int port = 12001;
    for(auto _ : state)
    {
        state.PauseTiming();
        std::thread server([port]() {
            tcp_socket::io_t io;
            tcp_listener     li{io};
            auto             sock = li.accept(port);
            if(sock)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                sock->close();
            }
            li.close();
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        tcp_dialer::io_t io;
        tcp_dialer       dialer{io};
        int              called = 0;
        state.ResumeTiming();

        dialer.async_dial("127.0.0.1",
                          static_cast<std::uint16_t>(port),
                          [&called](const tcp_dialer::err_t &err,
                                    tcp_dialer::sock_ptr_t   sock) {
                              (void) err;
                              (void) sock;
                              ++called;
                          });

        io.run();

        state.PauseTiming();
        if(server.joinable())
            server.join();
        benchmark::DoNotOptimize(called);
        state.ResumeTiming();
    }
}
BENCHMARK(bm_tcp_dialer_async_dial_guarded)->Iterations(50);

// Guarded combined operations: size, range, remove, close
static void bm_tcp_dialer_ops_guarded(benchmark::State &state)
{
    if(std::getenv("HJ_BENCH_ALLOW_NET") == nullptr)
    {
        state.SkipWithError(
            "network benchmarks disabled; set HJ_BENCH_ALLOW_NET=1 to enable");
        return;
    }

    const int port = 12002;
    for(auto _ : state)
    {
        state.PauseTiming();
        // start server threads to accept two connections
        std::thread server([port]() {
            tcp_socket::io_t io;
            tcp_listener     li{io};
            for(int i = 0; i < 2; ++i)
            {
                auto sock = li.accept(port);
                if(sock)
                    sock->close();
            }
            li.close();
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        tcp_dialer::io_t io;
        tcp_dialer       dialer{io};
        state.ResumeTiming();

        auto s1 = dialer.dial("127.0.0.1", static_cast<std::uint16_t>(port));
        auto s2 = dialer.dial("127.0.0.1", static_cast<std::uint16_t>(port));
        benchmark::DoNotOptimize(s1);
        benchmark::DoNotOptimize(s2);

        // size
        auto sz = dialer.size();
        benchmark::DoNotOptimize(sz);

        // range
        int count = 0;
        dialer.range([&count](tcp_dialer::sock_ptr_t sock) -> bool {
            (void) sock;
            ++count;
            return true;
        });
        benchmark::DoNotOptimize(count);

        // remove
        dialer.remove(s1);
        dialer.remove(s2);
        benchmark::DoNotOptimize(dialer.size());

        // close
        dialer.close();

        state.PauseTiming();
        if(server.joinable())
            server.join();
        state.ResumeTiming();
    }
}
BENCHMARK(bm_tcp_dialer_ops_guarded)->Iterations(50);
