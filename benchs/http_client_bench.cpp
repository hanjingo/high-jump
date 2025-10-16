#include <benchmark/benchmark.h>

#include <hj/net/http/http_client.hpp>
#include <cstdlib>
#include <string>

static bool env_allows_network()
{
    const char *v = std::getenv("BENCH_HTTP_ALLOW_NETWORK");
    return v && std::string(v) == "1";
}

static void bm_http_client_construct(benchmark::State &state)
{
    for(auto _ : state)
    {
        hj::http_client c("http://localhost:8080");
        benchmark::ClobberMemory();
    }
}

static void bm_http_client_get_optional(benchmark::State &state)
{
    bool allow = env_allows_network();
    for(auto _ : state)
    {
        if(!allow)
        {
            state.PauseTiming();
            // simulate cost without network
            std::this_thread::sleep_for(std::chrono::microseconds(10));
            state.ResumeTiming();
            continue;
        }

        hj::http_client c("http://httpbin.org");
        auto            res = c.Get("/get");
        (void) res;
        benchmark::ClobberMemory();
    }
}

BENCHMARK(bm_http_client_construct);
BENCHMARK(bm_http_client_get_optional);
