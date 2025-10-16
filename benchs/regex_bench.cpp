#include <benchmark/benchmark.h>
#include <hj/algo/regex.hpp>
#include <vector>
#include <string>

// Benchmarks for hj::REGEX_* patterns
namespace
{

static void bm_regex_idcard_cn(benchmark::State &state)
{
    std::vector<std::string> samples = {"123456789012345678",
                                        "12345678901234567x",
                                        "432503199408097040abc",
                                        ""};
    for(auto _ : state)
    {
        state.PauseTiming();
        state.ResumeTiming();

        for(const auto &s : samples)
            benchmark::DoNotOptimize(std::regex_match(s, hj::REGEX_IDCARD_CN));
    }
}

static void bm_regex_integer(benchmark::State &state)
{
    std::vector<std::string> samples = {"123", "-123", "0123", "123."};
    for(auto _ : state)
    {
        state.PauseTiming();
        state.ResumeTiming();
        for(const auto &s : samples)
            benchmark::DoNotOptimize(std::regex_match(s, hj::REGEX_INTEGER));
    }
}

static void bm_regex_decimals(benchmark::State &state)
{
    std::vector<std::pair<const std::regex &, std::vector<std::string>>>
        groups = {
            {hj::REGEX_DECIMALS1, {"123.4", "123.45", ""}},
            {hj::REGEX_DECIMALS2, {"123.45", "123.456", ""}},
            {hj::REGEX_DECIMALS3, {"123.456", "123.4567", ""}},
        };

    for(auto _ : state)
    {
        state.PauseTiming();
        state.ResumeTiming();
        for(auto &g : groups)
            for(const auto &s : g.second)
                benchmark::DoNotOptimize(std::regex_match(s, g.first));
    }
}

static void bm_regex_ip_port_mac(benchmark::State &state)
{
    std::vector<std::string> ip_samples   = {"192.168.0.1", "256.0.0.1", ""};
    std::vector<std::string> port_samples = {"0", "65535", "65536", ""};
    std::vector<std::string> mac_samples  = {"DC-21-5C-03-04-1D",
                                             "00-00-00-00-00-00",
                                             ""};

    for(auto _ : state)
    {
        state.PauseTiming();
        state.ResumeTiming();

        for(const auto &s : ip_samples)
            benchmark::DoNotOptimize(std::regex_match(s, hj::REGEX_IP_V4));
        for(const auto &s : port_samples)
            benchmark::DoNotOptimize(std::regex_match(s, hj::REGEX_PORT));
        for(const auto &s : mac_samples)
            benchmark::DoNotOptimize(std::regex_match(s, hj::REGEX_MAC));
    }
}

static void bm_regex_num_latter(benchmark::State &state)
{
    std::vector<std::string> samples = {"a123", "A123b", "1234567890", ""};
    for(auto _ : state)
    {
        state.PauseTiming();
        state.ResumeTiming();
        for(const auto &s : samples)
            benchmark::DoNotOptimize(std::regex_match(s, hj::REGEX_NUM_LATTER));
    }
}

} // namespace

BENCHMARK(bm_regex_idcard_cn)->Iterations(2000);
BENCHMARK(bm_regex_integer)->Iterations(2000);
BENCHMARK(bm_regex_decimals)->Iterations(2000);
BENCHMARK(bm_regex_ip_port_mac)->Iterations(2000);
BENCHMARK(bm_regex_num_latter)->Iterations(2000);
