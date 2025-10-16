#include <benchmark/benchmark.h>
#include <hj/testing/error_handler.hpp>
#include <system_error>

enum class err1
{
    unknow = -1,
    ok     = 0,
    timeout,
    mem_leak,
};

static std::error_code e_fn()
{
    return std::error_code(static_cast<int>(err1::timeout),
                           std::generic_category());
}

static std::error_code e_fn_fix_timeout()
{
    return std::error_code(static_cast<int>(err1::mem_leak),
                           std::generic_category());
}

static std::error_code e_fn_fix_memleak()
{
    return std::error_code();
}

static void bm_error_handler_std_error(benchmark::State &st)
{
    for(auto _ : st)
    {
        hj::error_handler<std::error_code> h([](const char *, const char *) {});
        std::error_code                    ok;
        std::error_code timeout(static_cast<int>(err1::timeout),
                                std::generic_category());
        std::error_code mem_leak(static_cast<int>(err1::mem_leak),
                                 std::generic_category());

        h.match(ok);
        h.match(timeout, [](const std::error_code &) {});
        h.match(mem_leak, [](const std::error_code &) {});
        h.match(ok);
        h.abort();
        h.reset();

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_error_handler_std_error)->Iterations(15000);

static void bm_error_handler_linked(benchmark::State &st)
{
    for(auto _ : st)
    {
        hj::error_handler<std::error_code> h([](const char *, const char *) {});
        std::error_code                    last = e_fn();

        h.match(last,
                [&](const std::error_code &e) {
                    if(e.value() == static_cast<int>(err1::timeout))
                        last = e_fn_fix_timeout();
                    h.reset();
                })
            .match(last,
                   [&](const std::error_code &e) {
                       if(e.value() == static_cast<int>(err1::mem_leak))
                           last = e_fn_fix_memleak();
                       h.reset();
                   })
            .match(last, [&](const std::error_code &e) {
                if(!e)
                    h.abort();
            });

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_error_handler_linked)->Iterations(10000);