#include <benchmark/benchmark.h>

#include <hj/net/http/ws_server.hpp>
#include <hj/net/http/ws_client.hpp>
#include <thread>
#include <chrono>
#include <filesystem>
#include <atomic>
#include <cstdlib>

using namespace std::chrono_literals;

static bool bench_allow_net()
{
    return std::getenv("HJ_BENCH_ALLOW_NET") != nullptr;
}

static bool bench_allow_ssl()
{
    return std::getenv("HJ_BENCH_ALLOW_SSL") != nullptr;
}

// Helper client used by sync server benchmark: connect, send "hello", recv "world", close
static void run_client_sync(uint16_t port)
{
    std::this_thread::sleep_for(10ms);
    hj::ws_client::io_t io;
    auto                client = std::make_shared<hj::ws_client>(io);
    if(!client->connect("127.0.0.1", std::to_string(port), "/"))
        return;
    client->send(std::string("hello"));
    std::string recv_msg;
    client->recv(recv_msg);
    client->close();
}

static void bm_wsserver_accept_recv_send_close(benchmark::State &state)
{
    if(!bench_allow_net())
        state.SkipWithError("HJ_BENCH_ALLOW_NET not set");

    for(auto _ : state)
    {
        uint16_t port =
            static_cast<uint16_t>(21000 + (state.iterations() % 100));
        state.PauseTiming();

        std::thread client_thr(run_client_sync, port);

        hj::ws_server::io_t  io;
        hj::ws_server::err_t err;
        auto ep = hj::ws_server::make_endpoint("127.0.0.1", port);

        auto serv = std::make_shared<hj::ws_server>(io);
        state.ResumeTiming();

        auto ws = serv->accept(ep, err);
        benchmark::DoNotOptimize(err);

        std::string msg = serv->recv(ws, err);
        benchmark::DoNotOptimize(msg);

        serv->send(ws, err, std::string("world"));
        serv->close();

        client_thr.join();
        benchmark::ClobberMemory();
    }
}

// Async server benchmark: server async_accept + async_recv/send; client async_connect/send/recv
static void bm_wsserver_async_accept_recv_send_close(benchmark::State &state)
{
    if(!bench_allow_net())
        state.SkipWithError("HJ_BENCH_ALLOW_NET not set");

    for(auto _ : state)
    {
        uint16_t port =
            static_cast<uint16_t>(21010 + (state.iterations() % 100));
        state.PauseTiming();

        hj::ws_server::io_t  io;
        hj::ws_server::err_t err;
        auto                 serv = std::make_shared<hj::ws_server>(io);

        std::atomic_bool done{false};

        // prepare client async connect
        auto client = std::make_shared<hj::ws_client>(io);
        client->async_connect(
            "127.0.0.1",
            std::to_string(port),
            "/",
            [client](const hj::ws_client::err_t &err) {
                if(err)
                    return;
                client->async_send(
                    "async_hello",
                    [client](const hj::ws_client::err_t &err, std::size_t) {
                        if(err)
                            return;
                        client->async_recv(
                            [riend = client](const hj::ws_client::err_t &err,
                                             std::string                 msg) {
                                (void) msg;
                                (void) err; // ignore here
                            });
                    });
            });

        // server accept + handlers
        serv->async_accept(
            hj::ws_server::make_endpoint("127.0.0.1", port),
            [serv, &done](const hj::ws_server::err_t    &err,
                          hj::ws_server::ws_stream_ptr_t ws) {
                if(err)
                    return;
                serv->async_recv(
                    ws,
                    [serv, ws, &done](const hj::ws_server::err_t    &err,
                                      hj::ws_server::ws_stream_ptr_t ws2,
                                      std::string                    msg) {
                        if(err)
                            return;
                        serv->async_send(
                            ws2,
                            "async_world",
                            [serv, ws2, &done](const hj::ws_server::err_t &err,
                                               hj::ws_server::ws_stream_ptr_t,
                                               std::size_t) {
                                (void) err;
                                serv->async_close(
                                    [](const hj::ws_server::err_t &) {},
                                    {ws2});
                                done.store(true);
                            });
                    });
            });

        state.ResumeTiming();
        io.run();

        benchmark::DoNotOptimize(done.load());
        benchmark::ClobberMemory();
    }
}

// SSL sync server benchmark
static void bm_wsserver_ssl_accept_recv_send_close(benchmark::State &state)
{
    if(!bench_allow_net() || !bench_allow_ssl())
        state.SkipWithError("HJ_BENCH_ALLOW_NET or HJ_BENCH_ALLOW_SSL not set");

    const std::string server_crt = "./server.crt";
    const std::string server_key = "./server.key";
    const std::string client_crt = "./client.crt";
    const std::string client_key = "./client.key";

    if(!std::filesystem::exists(server_crt)
       || !std::filesystem::exists(server_key)
       || !std::filesystem::exists(client_crt)
       || !std::filesystem::exists(client_key))
    {
        state.SkipWithError("SSL certificate files not present");
        return;
    }

    for(auto _ : state)
    {
        uint16_t port =
            static_cast<uint16_t>(21020 + (state.iterations() % 100));
        state.PauseTiming();

        // client thread using ws_client_ssl
        std::thread client_thr([port]() {
            std::this_thread::sleep_for(10ms);
            hj::ws_client_ssl::io_t io;
            auto                    client_ctx =
                hj::ws_client_ssl::make_ctx("./client.crt", "./client.key");
            auto client = std::make_shared<hj::ws_client_ssl>(io, client_ctx);
            if(!client->connect("127.0.0.1", std::to_string(port), "/"))
                return;
            client->send(std::string("hello_ssl"));
            std::string recv_msg;
            client->recv(recv_msg);
            client->close();
        });

        hj::ws_server_ssl::io_t io;
        auto                    ssl_ctx =
            hj::ws_server_ssl::make_ctx("./server.crt", "./server.key");
        hj::ws_server_ssl        serv(io, ssl_ctx);
        hj::ws_server_ssl::err_t err;
        auto ep = hj::ws_server_ssl::make_endpoint("127.0.0.1", port);

        state.ResumeTiming();
        auto ws = serv.accept(ep, err);
        benchmark::DoNotOptimize(err);

        std::string msg = serv.recv(ws, err);
        benchmark::DoNotOptimize(msg);

        serv.send(ws, err, std::string("world_ssl"));
        serv.close();

        client_thr.join();
        benchmark::ClobberMemory();
    }
}

BENCHMARK(bm_wsserver_accept_recv_send_close)->Unit(benchmark::kMillisecond);
BENCHMARK(bm_wsserver_async_accept_recv_send_close)
    ->Unit(benchmark::kMillisecond);
BENCHMARK(bm_wsserver_ssl_accept_recv_send_close)
    ->Unit(benchmark::kMillisecond);
