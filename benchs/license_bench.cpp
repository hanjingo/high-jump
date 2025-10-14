#include <benchmark/benchmark.h>
#include <hj/util/license.hpp>
#include <memory>
#include <string>
#include <chrono>
#include <vector>
#include <random>
#include <sstream>
#include <iostream>
#include <fstream>
#include <filesystem>

// Test keys for benchmarking
static const std::string PRIVATE_KEY = R"(-----BEGIN RSA PRIVATE KEY-----
MIIBOgIBAAJAevxSYQggOUn0bfka93jW0E2wkakW9gxE8h/RQTcsk06WYYQJB6CZ
cjLeXwVsoIC5T8Ph3lvRIGmVCEQQ0peBowIDAQABAkBSGbtMt0X7uJkKCS+tYOfW
auaidoSzgIFOOVtR8+k39GEB1yywvTrAspRQ0UB3tV2B9DeKUVlKm51l3UWCXJyx
AiEAzcAxj1Eg/F0EJAqgYj6IRUgJSZgwrKozohqH8e9LV38CIQCZBYtcI+iyT+lP
8UklJzkguLZ6drrtD4lI8pxr2ReH3QIhALUKQhF7L20fY39bIliP8VQU2Kc7FMk5
Ugl3Etuc1Ux9AiA+OYz0CP4lFG3RvlJ6Mzr93V8G6aUVpU20RkPpbzwsWQIhAKXc
afQ0WRgZier6MrdwlXd70JZIpgc6kLeOz2GuV4lX
-----END RSA PRIVATE KEY-----)";

static const std::string PUBLIC_KEY = R"(-----BEGIN PUBLIC KEY-----
MFswDQYJKoZIhvcNAQEBBQADSgAwRwJAevxSYQggOUn0bfka93jW0E2wkakW9gxE
8h/RQTcsk06WYYQJB6CZcjLeXwVsoIC5T8Ph3lvRIGmVCEQQ0peBowIDAQAB
-----END PUBLIC KEY-----)";

// Benchmark utility class for setup
class LicenseBenchmarkFixture : public benchmark::Fixture
{
  public:
    void SetUp(const benchmark::State &state) override
    {
        issuer_ = std::make_unique<hj::license::issuer>("benchmark_issuer",
                                                        hj::license::sign_algo::rsa256,
                                                        std::vector<std::string>{PUBLIC_KEY, PRIVATE_KEY, "", ""},
                                                        10000);
        verifier_ = std::make_unique<hj::license::verifier>("benchmark_issuer",
                                                            hj::license::sign_algo::rsa256,
                                                            std::vector<std::string>{PUBLIC_KEY, PRIVATE_KEY, "", ""});

        // Pre-generate some tokens for verification tests
        for(int i = 0; i < 100; ++i)
        {
            hj::license::token_t             token;
            std::vector<hj::license::pair_t> claims = {
                {"user_id", std::to_string(i)},
                {"role", "user"}};
            auto result = issuer_->issue(token,
                                         "bench_user_" + std::to_string(i),
                                         1, // 1 day leeway
                                         claims);
            if(!result)
            {
                pre_generated_tokens_.push_back(std::move(token));
            }
        }
    }

    void TearDown(const benchmark::State &state) override
    {
        // Clean up resources
        issuer_.reset();
        verifier_.reset();
        pre_generated_tokens_.clear();
    }

  protected:
    std::unique_ptr<hj::license::issuer>   issuer_;
    std::unique_ptr<hj::license::verifier> verifier_;
    std::vector<hj::license::token_t>      pre_generated_tokens_;
};

// Benchmark disk serial number access
static void bm_disk_sn_access(benchmark::State &state)
{
    for(auto _ : state)
    {
        auto disk_sn = hj::license::get_disk_sn();
        benchmark::DoNotOptimize(disk_sn);
    }

    state.SetItemsProcessed(state.iterations());
}

// Benchmark error code operations
static void bm_error_code_creation(benchmark::State &state)
{
    for(auto _ : state)
    {
        std::error_code success_code{};
        std::error_code error_code{static_cast<int>(hj::license::error_code::crypto_error), std::generic_category()};
        benchmark::DoNotOptimize(success_code);
        benchmark::DoNotOptimize(error_code);
    }

    state.SetItemsProcessed(state.iterations() * 2);
}

// Benchmark license issuer creation
static void bm_license_issuer_creation(benchmark::State &state)
{
    for(auto _ : state)
    {
        hj::license::issuer issuer("bench_issuer",
                                   hj::license::sign_algo::rsa256,
                                   std::vector<std::string>{PUBLIC_KEY, PRIVATE_KEY, "", ""},
                                   1000);
        benchmark::DoNotOptimize(&issuer);
    }

    state.SetItemsProcessed(state.iterations());
}

// Benchmark license verifier creation
static void bm_license_verifier_creation(benchmark::State &state)
{
    for(auto _ : state)
    {
        hj::license::verifier verifier("bench_verifier",
                                       hj::license::sign_algo::rsa256,
                                       std::vector<std::string>{PUBLIC_KEY, PRIVATE_KEY, "", ""});
        benchmark::DoNotOptimize(&verifier);
    }

    state.SetItemsProcessed(state.iterations());
}

// Benchmark key setup operations
BENCHMARK_F(LicenseBenchmarkFixture, bm_key_setup)(benchmark::State &state)
{
    for(auto _ : state)
    {
        hj::license::issuer test_issuer("key_setup_issuer",
                                        hj::license::sign_algo::rsa256,
                                        std::vector<std::string>{PUBLIC_KEY, PRIVATE_KEY, "", ""},
                                        100);

        std::vector<std::string> keys{PUBLIC_KEY, PRIVATE_KEY, "", ""};
        test_issuer.set_keys(std::move(keys));
        benchmark::DoNotOptimize(&test_issuer);
    }

    state.SetItemsProcessed(state.iterations());
}

// Benchmark license issuance
BENCHMARK_F(LicenseBenchmarkFixture,
            bm_license_issuance)(benchmark::State &state)
{
    std::random_device              rd;
    std::mt19937                    gen(rd());
    std::uniform_int_distribution<> dis(1, 10000);

    for(auto _ : state)
    {
        hj::license::token_t token;
        std::string          user_id = "bench_user_" + std::to_string(dis(gen));

        std::vector<hj::license::pair_t> claims = {
            {"user_id", user_id},
            {"role", "benchmark_user"},
            {"test_iteration", std::to_string(state.iterations())}};

        auto result = issuer_->issue(token,
                                     user_id,
                                     1, // 1 day leeway
                                     claims);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(token);
    }

    state.SetItemsProcessed(state.iterations());
}

// Benchmark license verification
BENCHMARK_F(LicenseBenchmarkFixture,
            bm_license_verification)(benchmark::State &state)
{
    if(pre_generated_tokens_.empty())
    {
        state.SkipWithError("No pre-generated tokens available");
        return;
    }

    size_t token_index = 0;

    for(auto _ : state)
    {
        const auto &token =
            pre_generated_tokens_[token_index % pre_generated_tokens_.size()];
        std::string user_id = "bench_user_" + std::to_string(token_index % 100);

        std::vector<hj::license::pair_t> expected_claims = {
            {"user_id", std::to_string(token_index % 100)},
            {"role", "user"}};

        auto result = verifier_->verify(token,
                                        user_id,
                                        1, // 1 day leeway
                                        expected_claims);
        benchmark::DoNotOptimize(result);

        ++token_index;
    }

    state.SetItemsProcessed(state.iterations());
}

// Benchmark license verification without caching (same as regular since no caching in the actual API)
BENCHMARK_F(LicenseBenchmarkFixture,
            bm_license_verification_no_cache)(benchmark::State &state)
{
    // Create a separate verifier
    hj::license::verifier no_cache_verifier("benchmark_issuer",
                                             hj::license::sign_algo::rsa256,
                                             std::vector<std::string>{PUBLIC_KEY, PRIVATE_KEY, "", ""});

    if(pre_generated_tokens_.empty())
    {
        state.SkipWithError("No pre-generated tokens available");
        return;
    }

    size_t token_index = 0;

    for(auto _ : state)
    {
        const auto &token =
            pre_generated_tokens_[token_index % pre_generated_tokens_.size()];
        std::string user_id = "bench_user_" + std::to_string(token_index % 100);

        std::vector<hj::license::pair_t> expected_claims = {
            {"user_id", std::to_string(token_index % 100)},
            {"role", "user"}};

        auto result = no_cache_verifier.verify(token,
                                               user_id,
                                               1, // 1 day leeway
                                               expected_claims);
        benchmark::DoNotOptimize(result);

        ++token_index;
    }

    state.SetItemsProcessed(state.iterations());
}

// Benchmark file I/O operations
BENCHMARK_F(LicenseBenchmarkFixture,
            bm_license_file_operations)(benchmark::State &state)
{
    for(auto _ : state)
    {
        std::string filename =
            "bench_license_" + std::to_string(state.iterations()) + ".lic";

        // Issue to file
        auto issue_result = issuer_->issue_file(filename,
                                                "file_bench_user",
                                                1); // 1 day leeway

        if(!issue_result)
        {
            // Verify from file
            auto verify_result = verifier_->verify_file(filename,
                                                        "file_bench_user",
                                                        1); // 1 day leeway
            benchmark::DoNotOptimize(verify_result);
        }

        benchmark::DoNotOptimize(issue_result);

        // Clean up file (optional, for real benchmark you might want to keep them)
        std::remove(filename.c_str());
    }

    state.SetItemsProcessed(state.iterations());
}

// Benchmark stream operations
BENCHMARK_F(LicenseBenchmarkFixture,
            bm_license_stream_operations)(benchmark::State &state)
{
    for(auto _ : state)
    {
        std::ostringstream oss;

        // Issue to stream
        auto issue_result = issuer_->issue(oss,
                                           "stream_bench_user",
                                           1); // 1 day leeway

        if(!issue_result)
        {
            std::istringstream iss(oss.str());

            // Verify from stream
            auto verify_result = verifier_->verify(iss,
                                                   "stream_bench_user",
                                                   1); // 1 day leeway
            benchmark::DoNotOptimize(verify_result);
        }

        benchmark::DoNotOptimize(issue_result);
    }

    state.SetItemsProcessed(state.iterations());
}

// Benchmark license info extraction
BENCHMARK_F(LicenseBenchmarkFixture,
            bm_license_info_extraction)(benchmark::State &state)
{
    if(pre_generated_tokens_.empty())
    {
        state.SkipWithError("No pre-generated tokens available");
        return;
    }

    size_t token_index = 0;

    for(auto _ : state)
    {
        const auto &token =
            pre_generated_tokens_[token_index % pre_generated_tokens_.size()];

        hj::license::license_info license_info;
        auto result = hj::license::verifier::parse(license_info, token);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(license_info);

        ++token_index;
    }

    state.SetItemsProcessed(state.iterations());
}

// Benchmark key release operations
BENCHMARK_F(LicenseBenchmarkFixture,
            bm_issuer_key_release)(benchmark::State &state)
{
    for(auto _ : state)
    {
        std::vector<std::string> new_keys{PUBLIC_KEY, PRIVATE_KEY, "", ""};
        auto result = issuer_->release(hj::license::sign_algo::rsa256, new_keys);
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}

// Benchmark various license limits
static void bm_license_limits(benchmark::State &state)
{
    const int max_licenses = static_cast<int>(state.range(0));

    for(auto _ : state)
    {
        hj::license::issuer limited_issuer("limited_issuer",
                                           hj::license::sign_algo::rsa256,
                                           std::vector<std::string>{PUBLIC_KEY, PRIVATE_KEY, "", ""},
                                           max_licenses);

        // Try to issue licenses up to the limit
        for(int i = 0; i < max_licenses; ++i)
        {
            hj::license::token_t token;
            auto result = limited_issuer.issue(token,
                                               "user_" + std::to_string(i),
                                               1); // 1 day leeway
            benchmark::DoNotOptimize(result);
            if(result)
                break; // Stop if we hit an error
        }

        benchmark::DoNotOptimize(limited_issuer.valid_times());
    }

    state.SetItemsProcessed(state.iterations() * max_licenses);
}

// Register simple benchmarks
BENCHMARK(bm_disk_sn_access);
BENCHMARK(bm_error_code_creation);
BENCHMARK(bm_license_issuer_creation);
BENCHMARK(bm_license_verifier_creation);

// Register benchmark with different license limits
BENCHMARK(bm_license_limits)->Arg(1)->Arg(10)->Arg(100)->Arg(1000)->Arg(10000);

// Set custom benchmark parameters
BENCHMARK(bm_disk_sn_access)->Threads(1)->Threads(2)->Threads(4);
BENCHMARK(bm_license_issuer_creation)->Threads(1)->Threads(2)->Threads(4);