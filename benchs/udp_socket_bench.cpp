#include <benchmark/benchmark.h>

#include <hj/net/udp/udp_socket.hpp>
#include <boost/asio.hpp>
#include <vector>
#include <memory>
#include <atomic>
#include <thread>
#include <chrono>
#include <cstdlib>

using namespace std::chrono_literals;

static std::atomic_uint_fast16_t s_port_base{30000};

static bool bench_allow_net()
{
    return std::getenv("HJ_BENCH_ALLOW_NET") != nullptr;
}

// Measure construct + close
static void bm_udp_construct_close(benchmark::State &state)
{
    if(!bench_allow_net())
        state.SkipWithError("HJ_BENCH_ALLOW_NET not set");

    boost::asio::io_context io;

    for(auto _ : state)
    {
        state.PauseTiming();
        hj::udp::socket s(io.get_executor());
        state.ResumeTiming();

        benchmark::DoNotOptimize(s);
        s.close();
        benchmark::ClobberMemory();
    }
}

// Measure bind + set_option (set_option timed; bind done outside timing)
static void bm_udp_bind_and_set_option(benchmark::State &state)
{
    if(!bench_allow_net())
        state.SkipWithError("HJ_BENCH_ALLOW_NET not set");

    for(auto _ : state)
    {
        state.PauseTiming();
        boost::asio::io_context io;
        hj::udp::socket         s(io.get_executor());

        // allocate two unique ports to reduce conflicts
        uint16_t        port = static_cast<uint16_t>(s_port_base.fetch_add(1));
        std::error_code ec;
        s.bind(port, ec);
        state.ResumeTiming();

        s.set_option(hj::udp::opt::reuse_addr(true), ec);
        benchmark::DoNotOptimize(ec);
        benchmark::ClobberMemory();
    }
}

// Measure synchronous send/recv roundtrip
static void bm_udp_send_recv(benchmark::State &state)
{
    if(!bench_allow_net())
        state.SkipWithError("HJ_BENCH_ALLOW_NET not set");

    for(auto _ : state)
    {
        state.PauseTiming();
        boost::asio::io_context io;
        hj::udp::socket         p1(io.get_executor());
        hj::udp::socket         p2(io.get_executor());

        uint16_t        base = static_cast<uint16_t>(s_port_base.fetch_add(2));
        uint16_t        p1_port = base;
        uint16_t        p2_port = static_cast<uint16_t>(base + 1);
        std::error_code ec1, ec2;
        p1.bind(p1_port, ec1);
        p2.bind(p2_port, ec2);

        char   buf[64] = {0};
        size_t len     = sizeof(buf);
        auto   ep      = hj::udp::socket::endpoint("127.0.0.1", p1_port);
        state.ResumeTiming();

        size_t sent = p2.send(buf, len, ep, ec2);
        benchmark::DoNotOptimize(sent);
        benchmark::DoNotOptimize(ec2);

        hj::udp::socket::endpoint_t sender;
        size_t                      recvd = p1.recv(buf, len, sender, ec1);
        benchmark::DoNotOptimize(recvd);
        benchmark::DoNotOptimize(ec1);
        benchmark::ClobberMemory();
    }
}

// Measure async send/recv roundtrip (includes running io_context)
static void bm_udp_async_send_recv(benchmark::State &state)
{
    if(!bench_allow_net())
        state.SkipWithError("HJ_BENCH_ALLOW_NET not set");

    for(auto _ : state)
    {
        state.PauseTiming();
        boost::asio::io_context io;
        hj::udp::socket         p1(io.get_executor(), false);
        hj::udp::socket         p2(io.get_executor(), false);

        uint16_t        base = static_cast<uint16_t>(s_port_base.fetch_add(2));
        uint16_t        p1_port = base;
        uint16_t        p2_port = static_cast<uint16_t>(base + 1);
        std::error_code ec1, ec2;
        p1.bind(p1_port, ec1);
        p2.bind(p2_port, ec2);

        char   buf[64] = {0};
        size_t len     = sizeof(buf);
        auto   ep      = hj::udp::socket::endpoint("127.0.0.1", p1_port);

        // handlers
        std::atomic_bool send_called{false}, recv_called{false};

        state.ResumeTiming();

        p1.async_recv(
            buf,
            len,
            ep,
            [&recv_called, len](const std::error_code &ec, std::size_t sz) {
                benchmark::DoNotOptimize(ec);
                benchmark::DoNotOptimize(sz);
                recv_called.store(true);
            });

        p2.async_send(
            buf,
            len,
            ep,
            [&send_called, len](const std::error_code &ec, std::size_t sz) {
                benchmark::DoNotOptimize(ec);
                benchmark::DoNotOptimize(sz);
                send_called.store(true);
            });

        // run until operations complete (small timeout guard)
        // run() will return once no more work remains
        io.run();

        benchmark::DoNotOptimize(send_called.load());
        benchmark::DoNotOptimize(recv_called.load());
        benchmark::ClobberMemory();
    }
}

// Measure invalid-argument path (null buffer)
static void bm_udp_invalid_argument(benchmark::State &state)
{
    if(!bench_allow_net())
        state.SkipWithError("HJ_BENCH_ALLOW_NET not set");

    for(auto _ : state)
    {
        state.PauseTiming();
        boost::asio::io_context io;
        hj::udp::socket         s(io.get_executor());
        state.ResumeTiming();

        std::error_code ec;
        const char     *buf  = nullptr;
        size_t          len  = 0;
        auto            ep   = hj::udp::socket::endpoint("127.0.0.1", 30000);
        size_t          sent = s.send(buf, len, ep, ec);
        benchmark::DoNotOptimize(sent);
        benchmark::DoNotOptimize(ec);
        benchmark::ClobberMemory();
    }
}

// Measure constructing from existing raw socket (executor + socket reuse)
static void bm_udp_executor_and_socket_reuse(benchmark::State &state)
{
    if(!bench_allow_net())
        state.SkipWithError("HJ_BENCH_ALLOW_NET not set");

    for(auto _ : state)
    {
        state.PauseTiming();
        boost::asio::io_context      io;
        boost::asio::ip::udp::socket raw_sock(io.get_executor());
        raw_sock.open(boost::asio::ip::udp::v4());
        auto uptr =
            std::make_unique<boost::asio::ip::udp::socket>(std::move(raw_sock));
        state.ResumeTiming();

        hj::udp::socket s(io.get_executor(), std::move(uptr));
        benchmark::DoNotOptimize(s);
        s.close();
        benchmark::ClobberMemory();
    }
}

BENCHMARK(bm_udp_construct_close)->Unit(benchmark::kMicrosecond);
BENCHMARK(bm_udp_bind_and_set_option)->Unit(benchmark::kMicrosecond);
BENCHMARK(bm_udp_send_recv)->Unit(benchmark::kMicrosecond);
BENCHMARK(bm_udp_async_send_recv)->Unit(benchmark::kMicrosecond);
BENCHMARK(bm_udp_invalid_argument)->Unit(benchmark::kMicrosecond);
BENCHMARK(bm_udp_executor_and_socket_reuse)->Unit(benchmark::kMicrosecond);
