#include <benchmark/benchmark.h>

#include <hj/net/http/http_request.hpp>
#include <string>
#include <vector>

using namespace hj;

static void bm_http_request_construct(benchmark::State &state)
{
    for(auto _ : state)
    {
        http_request req;
        benchmark::ClobberMemory();
    }
}

static void bm_http_request_set_method_path(benchmark::State &state)
{
    for(auto _ : state)
    {
        http_request req;
        req.method = "POST";
        req.path   = "/api/test";
        benchmark::ClobberMemory();
    }
}

static void bm_http_request_set_headers(benchmark::State &state)
{
    for(auto _ : state)
    {
        http_request req;
        req.headers.insert({"Content-Type", "application/json"});
        req.headers.insert({"Accept", "*/*"});
        req.headers.insert({"X-Test", "1"});
        benchmark::ClobberMemory();
    }
}

static void bm_http_params_insert(benchmark::State &state)
{
    for(auto _ : state)
    {
        http_params params;
        params.insert({"key1", "value1"});
        params.insert({"key2", "value2"});
        params.insert({"key3", "value3"});
        benchmark::ClobberMemory();
    }
}

static void bm_http_multipart_build(benchmark::State &state)
{
    for(auto _ : state)
    {
        http_multipart_form_data_items items = {
            {"field1", "value1", "file1.txt", "text/plain"},
            {"field2", "value2", "file2.bin", "application/octet-stream"}};

        // iterate to simulate processing
        size_t total_len = 0;
        for(const auto &it : items)
            total_len += it.content.size();
        (void) total_len;
        benchmark::ClobberMemory();
    }
}

BENCHMARK(bm_http_request_construct);
BENCHMARK(bm_http_request_set_method_path);
BENCHMARK(bm_http_request_set_headers);
BENCHMARK(bm_http_params_insert);
BENCHMARK(bm_http_multipart_build);
