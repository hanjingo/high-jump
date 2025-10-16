#include <benchmark/benchmark.h>

#include <hj/net/http/http_response.hpp>
#include <string>
#include <vector>

using namespace hj;

static void bm_http_response_construct(benchmark::State &state)
{
    for(auto _ : state)
    {
        http_response res;
        benchmark::ClobberMemory();
    }
}

static void bm_http_response_set_status_body(benchmark::State &state)
{
    for(auto _ : state)
    {
        http_response res;
        res.status = 404;
        res.body   = "Not Found";
        benchmark::ClobberMemory();
    }
}

static void bm_http_response_set_headers(benchmark::State &state)
{
    for(auto _ : state)
    {
        http_response res;
        res.headers.insert({"Content-Type", "application/json"});
        res.headers.insert({"Cache-Control", "no-cache"});
        res.headers.insert({"X-Test", "1"});
        benchmark::ClobberMemory();
    }
}

static void bm_http_response_copy_move(benchmark::State &state)
{
    http_response base;
    base.status = 200;
    base.body   = std::string(1024, 'x');
    base.headers.insert({"Content-Type", "text/plain"});

    for(auto _ : state)
    {
        // copy
        http_response cpy = base;
        // move
        http_response mv = std::move(cpy);
        (void) mv;
        benchmark::ClobberMemory();
    }
}

BENCHMARK(bm_http_response_construct);
BENCHMARK(bm_http_response_set_status_body);
BENCHMARK(bm_http_response_set_headers);
BENCHMARK(bm_http_response_copy_move);
