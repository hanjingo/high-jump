#include <benchmark/benchmark.h>
#include <hj/db/redis.hpp>
#include <string>
#include <cstdlib>

namespace
{

static bool env_bool(const char *name)
{
    const char *v = std::getenv(name);
    return v && (v[0] == '1' || v[0] == 'y' || v[0] == 'Y');
}

static std::string env_str(const char *name)
{
    const char *v = std::getenv(name);
    return v ? std::string(v) : std::string();
}

// Always-run: try to connect to an invalid port/host and measure failure cost
static void bm_redis_connect_fail(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        const std::string host = "127.0.0.1";
        const int         port = 6399; // likely closed port
        state.ResumeTiming();

        hj::redis r(host, port, 100);
        bool      ok = r.is_connected();
        benchmark::DoNotOptimize(ok);
    }
}

// If allowed, perform a real connect/disconnect using env vars
static void bm_redis_connect_disconnect_real(benchmark::State &state)
{
    if(!env_bool("HJ_BENCH_ALLOW_REDIS"))
    {
        state.SkipWithError("HJ_BENCH_ALLOW_REDIS not set");
        return;
    }

    const std::string host = env_str("HJ_REDIS_HOST").empty()
                                 ? "127.0.0.1"
                                 : env_str("HJ_REDIS_HOST");
    const int         port = env_str("HJ_REDIS_PORT").empty()
                                 ? 6379
                                 : std::stoi(env_str("HJ_REDIS_PORT"));

    for(auto _ : state)
    {
        state.PauseTiming();
        state.ResumeTiming();

        hj::redis r(host, port, 2000);
        bool      ok = r.is_connected();
        benchmark::DoNotOptimize(ok);
        r = hj::redis(); // ensure disconnect
    }
}

// If allowed, perform simple SET/GET roundtrip
static void bm_redis_set_get_real(benchmark::State &state)
{
    if(!env_bool("HJ_BENCH_ALLOW_REDIS"))
    {
        state.SkipWithError("HJ_BENCH_ALLOW_REDIS not set");
        return;
    }

    const std::string host = env_str("HJ_REDIS_HOST").empty()
                                 ? "127.0.0.1"
                                 : env_str("HJ_REDIS_HOST");
    const int         port = env_str("HJ_REDIS_PORT").empty()
                                 ? 6379
                                 : std::stoi(env_str("HJ_REDIS_PORT"));

    hj::redis r(host, port, 2000);
    if(!r.is_connected())
    {
        state.SkipWithError("Cannot open Redis connection");
        return;
    }

    for(auto _ : state)
    {
        bool set_ok = r.set("hj_bench_key", "v");
        benchmark::DoNotOptimize(set_ok);
        std::string v = r.get("hj_bench_key");
        benchmark::DoNotOptimize(v);
    }
}

} // namespace

BENCHMARK(bm_redis_connect_fail)->Iterations(2000);
BENCHMARK(bm_redis_connect_disconnect_real)->Iterations(500);
BENCHMARK(bm_redis_set_get_real)->Iterations(1000);
