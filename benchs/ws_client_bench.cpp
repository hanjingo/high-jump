#include <benchmark/benchmark.h>

#include <hj/net/http/ws_client.hpp>
#include <hj/net/http/ws_server.hpp>
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

// Helper: run a short websocket server on a background thread that accepts one
// connection and echoes a single message sequence; used by sync benchmarks.
static void run_echo_server(uint16_t port, const std::string &reply)
{
    hj::ws_server::io_t  io;
    hj::ws_server::err_t err;
    auto                 ep = hj::ws_server::make_endpoint("127.0.0.1", port);

    auto serv = std::make_shared<hj::ws_server>(io);
    auto ws   = serv->accept(ep, err);
    if(err)
        return;

    std::string msg = serv->recv(ws, err);
    if(!err)
        serv->send(ws, err, reply);

    std::this_thread::sleep_for(5ms);
    serv->close();
}

static void bm_wsclient_connect_send_recv_close(benchmark::State &state)
{
    if(!bench_allow_net())
        state.SkipWithError("HJ_BENCH_ALLOW_NET not set");

    for(auto _ : state)
    {
        uint16_t port =
            static_cast<uint16_t>(20010 + (state.iterations() % 100));
        state.PauseTiming();
        std::thread srv([port]() { run_echo_server(port, "world"); });
        std::this_thread::sleep_for(5ms);
        hj::ws_client::io_t io;
        auto                client = std::make_shared<hj::ws_client>(io);
        state.ResumeTiming();

        bool ok = client->connect("127.0.0.1", std::to_string(port), "/");
        benchmark::DoNotOptimize(ok);
        std::string msg = "hello";
        ok              = client->send(msg);
        benchmark::DoNotOptimize(ok);
        std::string recv_msg;
        ok = client->recv(recv_msg);
        benchmark::DoNotOptimize(recv_msg);
        client->close();

        srv.join();
        benchmark::ClobberMemory();
    }
}

// Async connect/send/recv benchmark (runs io.run to process events)
static void bm_wsclient_async_connect_send_recv(benchmark::State &state)
{
    if(!bench_allow_net())
        state.SkipWithError("HJ_BENCH_ALLOW_NET not set");

    for(auto _ : state)
    {
        uint16_t port =
            static_cast<uint16_t>(20020 + (state.iterations() % 100));
        state.PauseTiming();
        std::atomic_bool done{false};
        std::thread srv([port]() { run_echo_server(port, "async_world"); });
        std::this_thread::sleep_for(5ms);
        hj::ws_client::io_t io;
        auto                client = std::make_shared<hj::ws_client>(io);
        state.ResumeTiming();

        client->async_connect(
            "127.0.0.1",
            std::to_string(port),
            "/",
            [client, &done](const hj::ws_client::err_t &err) {
                if(err)
                {
                    done.store(true);
                    return;
                }
                client->async_send(
                    "async_hello",
                    [client](const hj::ws_client::err_t &err, std::size_t) {
                        if(err)
                            return;
                        client->async_recv(
                            [client](const hj::ws_client::err_t &err,
                                     std::string                 msg) {
                                client->async_close(
                                    [](const hj::ws_client::err_t &) {});
                            });
                    });
            });

        io.run();
        srv.join();
        benchmark::ClobberMemory();
    }
}

// SSL sync connect/send/recv (requires certificate files present)
static void bm_wsclient_ssl_connect_send_recv(benchmark::State &state)
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
            static_cast<uint16_t>(20030 + (state.iterations() % 100));
        state.PauseTiming();
        std::thread srv([port]() {
            // run a minimal SSL ws server using provided server.crt/key
            hj::ws_server_ssl::io_t io;
            auto                    ssl_ctx =
                hj::ws_server_ssl::make_ctx("./server.crt", "./server.key");
            hj::ws_server_ssl        serv(io, ssl_ctx);
            hj::ws_server_ssl::err_t err;
            auto ep = hj::ws_server_ssl::make_endpoint("127.0.0.1", port);
            auto ws = serv.accept(ep, err);
            if(!err)
            {
                std::string msg = serv.recv(ws, err);
                if(!err)
                    serv.send(ws, err, std::string("world_ssl"));
                serv.close();
            }
        });
        std::this_thread::sleep_for(10ms);
        hj::ws_client_ssl::io_t io;
        auto                    client_ctx =
            hj::ws_client_ssl::make_ctx("./client.crt", "./client.key");
        auto client = std::make_shared<hj::ws_client_ssl>(io, client_ctx);
        state.ResumeTiming();

        bool ok = client->connect("127.0.0.1", std::to_string(port), "/");
        benchmark::DoNotOptimize(ok);
        std::string msg = "hello_ssl";
        ok              = client->send(msg);
        benchmark::DoNotOptimize(ok);
        std::string recv_msg;
        ok = client->recv(recv_msg);
        benchmark::DoNotOptimize(recv_msg);
        client->close();

        srv.join();
        benchmark::ClobberMemory();
    }
}

BENCHMARK(bm_wsclient_connect_send_recv_close)->Unit(benchmark::kMillisecond);
BENCHMARK(bm_wsclient_async_connect_send_recv)->Unit(benchmark::kMillisecond);
BENCHMARK(bm_wsclient_ssl_connect_send_recv)->Unit(benchmark::kMillisecond);
