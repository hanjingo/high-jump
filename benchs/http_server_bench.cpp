#include <benchmark/benchmark.h>

#include <hj/net/http/http_server.hpp>
#include <cstdlib>
#include <thread>
#include <chrono>

static bool env_allows_network()
{
    const char *v = std::getenv("BENCH_HTTP_ALLOW_NETWORK");
    return v && std::string(v) == "1";
}

static void bm_http_server_construct(benchmark::State &state)
{
    for(auto _ : state)
    {
        hj::http_server s;
        benchmark::ClobberMemory();
    }
}

static void bm_http_server_set_route(benchmark::State &state)
{
    for(auto _ : state)
    {
        hj::http_server s;
        s.Get("/hello", [](const httplib::Request &, httplib::Response &res) {
            res.set_content("Hello, World!", "text/plain");
        });
        benchmark::ClobberMemory();
    }
}

static void bm_http_server_start_stop_optional(benchmark::State &state)
{
    bool allow = env_allows_network();
    for(auto _ : state)
    {
        if(!allow)
        {
            // simulate small cost without binding network
            state.PauseTiming();
            std::this_thread::sleep_for(std::chrono::microseconds(10));
            state.ResumeTiming();
            continue;
        }

        hj::http_server s;
        // Use port 0 or OS-assigned if supported; here we avoid blocking by
        // starting in background thread and stopping quickly.
        std::thread t([&s]() { s.listen("127.0.0.1", 0); });
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        s.stop();
        if(t.joinable())
            t.join();
        benchmark::ClobberMemory();
    }
}

BENCHMARK(bm_http_server_construct);
BENCHMARK(bm_http_server_set_route);
BENCHMARK(bm_http_server_start_stop_optional);
