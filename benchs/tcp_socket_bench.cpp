#include <benchmark/benchmark.h>
#include <hj/net/tcp.hpp>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <array>
#include <memory>

using namespace hj;

// Construct/destruct
static void bm_tcp_socket_construct(benchmark::State &state)
{
    for(auto _ : state)
    {
        tcp_socket::io_t io;
        tcp_socket       sock{io};
        benchmark::DoNotOptimize(sock);
    }
}
BENCHMARK(bm_tcp_socket_construct)->Iterations(10000);

// set_option on a not-open socket (fast, no network)
static void bm_tcp_socket_set_option_no_open(benchmark::State &state)
{
    for(auto _ : state)
    {
        tcp_socket::io_t io;
        tcp_socket       sock{io};
        bool             ok = sock.set_option(tcp_socket::opt_no_delay(true));
        benchmark::DoNotOptimize(ok);
    }
}
BENCHMARK(bm_tcp_socket_set_option_no_open)->Iterations(100000);

// set_option after opening the underlying native socket (guarded)
static void bm_tcp_socket_set_option_open_guarded(benchmark::State &state)
{
    if(std::getenv("HJ_BENCH_ALLOW_NET") == nullptr)
    {
        state.SkipWithError(
            "network benchmarks disabled; set HJ_BENCH_ALLOW_NET=1 to enable");
        return;
    }

    for(auto _ : state)
    {
        state.PauseTiming();
        tcp_socket::io_t io;
        auto             base = new tcp_socket::sock_t(io);
        base->open(boost::asio::ip::tcp::v4());
        tcp_socket sock{io, base};
        state.ResumeTiming();

        bool ok = sock.set_option(tcp_socket::opt_no_delay(true));
        benchmark::DoNotOptimize(ok);

        state.PauseTiming();
        sock.close();
        state.ResumeTiming();
    }
}
BENCHMARK(bm_tcp_socket_set_option_open_guarded)->Iterations(100);

// connect (guarded) - start listener and connect
static void bm_tcp_socket_connect_guarded(benchmark::State &state)
{
    if(std::getenv("HJ_BENCH_ALLOW_NET") == nullptr)
    {
        state.SkipWithError(
            "network benchmarks disabled; set HJ_BENCH_ALLOW_NET=1 to enable");
        return;
    }

    const int port = 14000;
    for(auto _ : state)
    {
        state.PauseTiming();
        std::thread server([port]() {
            tcp_socket::io_t io;
            tcp_listener     li{io};
            auto             sock = li.accept(port);
            if(sock)
                sock->close();
            li.close();
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        tcp_socket::io_t io;
        tcp_socket       sock{io};
        state.ResumeTiming();

        bool ok = sock.connect("127.0.0.1", static_cast<uint16_t>(port));
        benchmark::DoNotOptimize(ok);

        state.PauseTiming();
        sock.close();
        if(server.joinable())
            server.join();
        state.ResumeTiming();
    }
}
BENCHMARK(bm_tcp_socket_connect_guarded)->Iterations(50);

// async_connect (guarded)
static void bm_tcp_socket_async_connect_guarded(benchmark::State &state)
{
    if(std::getenv("HJ_BENCH_ALLOW_NET") == nullptr)
    {
        state.SkipWithError(
            "network benchmarks disabled; set HJ_BENCH_ALLOW_NET=1 to enable");
        return;
    }

    const int port = 14001;
    for(auto _ : state)
    {
        state.PauseTiming();
        tcp_socket::io_t io;
        tcp_listener     li{io};
        li.async_accept(port,
                        [](const tcp_listener::err_t  &err,
                           std::shared_ptr<tcp_socket> sock) {
                            (void) err;
                            (void) sock;
                        });

        std::thread client([port]() {
            tcp_socket::io_t io_c;
            tcp_socket       sock{io_c};
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            sock.connect("127.0.0.1", static_cast<uint16_t>(port));
            sock.close();
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        tcp_socket sock{io};
        int        called = 0;
        state.ResumeTiming();

        sock.async_connect("127.0.0.1",
                           static_cast<uint16_t>(port),
                           [&called](const tcp_socket::err_t &err) {
                               (void) err;
                               ++called;
                           });

        io.run_for(std::chrono::milliseconds(100));

        state.PauseTiming();
        if(client.joinable())
            client.join();
        benchmark::DoNotOptimize(called);
        state.ResumeTiming();
    }
}
BENCHMARK(bm_tcp_socket_async_connect_guarded)->Iterations(20);

// send/recv (guarded)
static void bm_tcp_socket_send_recv_guarded(benchmark::State &state)
{
    if(std::getenv("HJ_BENCH_ALLOW_NET") == nullptr)
    {
        state.SkipWithError(
            "network benchmarks disabled; set HJ_BENCH_ALLOW_NET=1 to enable");
        return;
    }

    const int port = 14002;
    for(auto _ : state)
    {
        state.PauseTiming();
        std::thread server([port]() {
            tcp_socket::io_t io;
            tcp_listener     li{io};
            for(int i = 0; i < 2; ++i)
            {
                auto s = li.accept(port);
                if(!s)
                    continue;
                unsigned char buf[1024];
                s->recv(buf, 1024);
                s->send(reinterpret_cast<const unsigned char *>("pong"), 5);
                s->close();
            }
            li.close();
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        tcp_socket::io_t io;
        tcp_socket       sock{io};
        state.ResumeTiming();

        bool ok = sock.connect("127.0.0.1", static_cast<uint16_t>(port));
        benchmark::DoNotOptimize(ok);

        const unsigned char data[] = "ping";
        size_t              sent   = sock.send(data, 5);
        benchmark::DoNotOptimize(sent);

        unsigned char buf[1024];
        size_t        rec = sock.recv(buf, 1024);
        benchmark::DoNotOptimize(rec);

        state.PauseTiming();
        sock.close();
        if(server.joinable())
            server.join();
        state.ResumeTiming();
    }
}
BENCHMARK(bm_tcp_socket_send_recv_guarded)->Iterations(20);

// recv_until (guarded)
static void bm_tcp_socket_recv_until_guarded(benchmark::State &state)
{
    if(std::getenv("HJ_BENCH_ALLOW_NET") == nullptr)
    {
        state.SkipWithError(
            "network benchmarks disabled; set HJ_BENCH_ALLOW_NET=1 to enable");
        return;
    }

    const int port = 14003;
    for(auto _ : state)
    {
        state.PauseTiming();
        std::thread server([port]() {
            tcp_socket::io_t io;
            tcp_listener     li{io};
            auto             s = li.accept(port);
            if(s)
            {
                s->send(reinterpret_cast<const unsigned char *>("hello"), 6);
                s->send(reinterpret_cast<const unsigned char *>("world"), 6);
                s->close();
            }
            li.close();
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        tcp_socket::io_t io;
        tcp_socket       sock{io};
        state.ResumeTiming();

        bool ok = sock.connect("127.0.0.1", static_cast<uint16_t>(port));
        benchmark::DoNotOptimize(ok);

        tcp_socket::streambuf_t buf;
        size_t                  s1 = sock.recv_until(buf, 6);
        benchmark::DoNotOptimize(s1);
        buf.consume(6);
        size_t s2 = sock.recv_until(buf, 6);
        benchmark::DoNotOptimize(s2);

        state.PauseTiming();
        sock.close();
        if(server.joinable())
            server.join();
        state.ResumeTiming();
    }
}
BENCHMARK(bm_tcp_socket_recv_until_guarded)->Iterations(10);

// async_recv (guarded)
static void bm_tcp_socket_async_recv_guarded(benchmark::State &state)
{
    if(std::getenv("HJ_BENCH_ALLOW_NET") == nullptr)
    {
        state.SkipWithError(
            "network benchmarks disabled; set HJ_BENCH_ALLOW_NET=1 to enable");
        return;
    }

    const int port = 14004;
    for(auto _ : state)
    {
        state.PauseTiming();
        std::thread server([port]() {
            tcp_socket::io_t io;
            tcp_listener     li{io};
            auto             s = li.accept(port);
            if(s)
            {
                s->send(reinterpret_cast<const unsigned char *>("hello"), 6);
                s->send(reinterpret_cast<const unsigned char *>("world"), 6);
                s->close();
            }
            li.close();
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        tcp_socket::io_t io;
        tcp_socket       sock{io};
        state.ResumeTiming();

        bool ok = sock.connect("127.0.0.1", static_cast<uint16_t>(port));
        benchmark::DoNotOptimize(ok);

        auto buf_ptr = std::make_shared<std::array<unsigned char, 1024>>();
        int  nrecved = 0;
        sock.async_recv(
            buf_ptr->data(),
            5,
            [&nrecved](const tcp_socket::err_t &err, std::size_t sz) {
                (void) err;
                nrecved += (int) sz;
            });
        sock.async_recv(
            buf_ptr->data() + 5,
            5,
            [&nrecved](const tcp_socket::err_t &err, std::size_t sz) {
                (void) err;
                nrecved += (int) sz;
            });

        io.run_for(std::chrono::milliseconds(100));

        state.PauseTiming();
        sock.close();
        if(server.joinable())
            server.join();
        benchmark::DoNotOptimize(nrecved);
        state.ResumeTiming();
    }
}
BENCHMARK(bm_tcp_socket_async_recv_guarded)->Iterations(10);

// set_conn_status (fast)
static void bm_tcp_socket_set_conn_status(benchmark::State &state)
{
    for(auto _ : state)
    {
        tcp_socket::io_t io;
        auto             base = new tcp_socket::sock_t(io);
        tcp_socket       sock{io, base};
        bool             old = sock.set_conn_status(false);
        benchmark::DoNotOptimize(old);
        bool changed = sock.set_conn_status(true);
        benchmark::DoNotOptimize(changed);
    }
}
BENCHMARK(bm_tcp_socket_set_conn_status)->Iterations(10000);

// close (idempotent)
static void bm_tcp_socket_close(benchmark::State &state)
{
    for(auto _ : state)
    {
        tcp_socket::io_t io;
        tcp_socket       sock{io};
        sock.close();
        sock.close();
        benchmark::DoNotOptimize(sock.is_connected());
    }
}
BENCHMARK(bm_tcp_socket_close)->Iterations(10000);
