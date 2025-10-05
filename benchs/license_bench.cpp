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
        issuer_ =
            hj::license::create_license_issuer("benchmark_issuer",
                                               hj::license::sign_algo::rsa256,
                                               10000);
        verifier_ =
            hj::license::create_license_verifier("benchmark_issuer",
                                                 hj::license::sign_algo::rsa256,
                                                 true);

        // Set up keys
        std::vector<hj::license::secure_string> issuer_keys;
        issuer_keys.emplace_back(PUBLIC_KEY);
        issuer_keys.emplace_back(PRIVATE_KEY);
        issuer_->set_keys(std::move(issuer_keys));

        std::vector<hj::license::secure_string> verifier_keys;
        verifier_keys.emplace_back(PUBLIC_KEY);
        verifier_keys.emplace_back(PRIVATE_KEY);
        verifier_->set_keys(std::move(verifier_keys));

        // Pre-generate some tokens for verification tests
        for(int i = 0; i < 100; ++i)
        {
            hj::license::token_t             token;
            std::vector<hj::license::pair_t> claims = {
                {"user_id", std::to_string(i)},
                {"role", "user"}};
            auto result =
                issuer_->issue_license(token,
                                       "bench_user_" + std::to_string(i),
                                       std::chrono::hours{24},
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
    std::unique_ptr<hj::license::license_issuer>   issuer_;
    std::unique_ptr<hj::license::license_verifier> verifier_;
    std::vector<hj::license::token_t>              pre_generated_tokens_;
};

// Benchmark secure_string operations
static void bm_secure_string_construction(benchmark::State &state)
{
    const std::string test_data = "This is a test secret that should be "
                                  "securely handled by secure_string class";

    for(auto _ : state)
    {
        hj::license::secure_string secure_str(test_data);
        benchmark::DoNotOptimize(secure_str.get());
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations());
}

static void bm_secure_string_move(benchmark::State &state)
{
    const std::string test_data =
        "This is a test secret for move operations benchmark";

    for(auto _ : state)
    {
        hj::license::secure_string secure_str1(test_data);
        hj::license::secure_string secure_str2 = std::move(secure_str1);
        benchmark::DoNotOptimize(secure_str2.get());
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations());
}

// Benchmark hardware info caching
static void bm_hardware_info_access(benchmark::State &state)
{
    for(auto _ : state)
    {
        auto hw_id    = hj::license::get_hardware_id();
        auto mac_addr = hj::license::get_system_mac_address();
        benchmark::DoNotOptimize(hw_id);
        benchmark::DoNotOptimize(mac_addr);
    }

    state.SetItemsProcessed(state.iterations()
                            * 2); // Two function calls per iteration
}

// Benchmark error code operations
static void bm_error_code_creation(benchmark::State &state)
{
    for(auto _ : state)
    {
        auto success_code =
            hj::license::make_error_code(hj::license::error_code::success);
        auto error_code =
            hj::license::make_error_code(hj::license::error_code::crypto_error);
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
        auto issuer =
            hj::license::create_license_issuer("bench_issuer",
                                               hj::license::sign_algo::rsa256,
                                               1000);
        benchmark::DoNotOptimize(issuer);
    }

    state.SetItemsProcessed(state.iterations());
}

// Benchmark license verifier creation
static void bm_license_verifier_creation(benchmark::State &state)
{
    for(auto _ : state)
    {
        auto verifier =
            hj::license::create_license_verifier("bench_verifier",
                                                 hj::license::sign_algo::rsa256,
                                                 true);
        benchmark::DoNotOptimize(verifier);
    }

    state.SetItemsProcessed(state.iterations());
}

// Benchmark key setup operations
BENCHMARK_F(LicenseBenchmarkFixture, bm_key_setup)(benchmark::State &state)
{
    for(auto _ : state)
    {
        auto test_issuer =
            hj::license::create_license_issuer("key_setup_issuer",
                                               hj::license::sign_algo::rsa256,
                                               100);

        std::vector<hj::license::secure_string> keys;
        keys.emplace_back(PUBLIC_KEY);
        keys.emplace_back(PRIVATE_KEY);

        auto result = test_issuer->set_keys(std::move(keys));
        benchmark::DoNotOptimize(result);
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

        auto result = issuer_->issue_license(token,
                                             user_id,
                                             std::chrono::hours{24},
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

        auto result = verifier_->verify_license(token,
                                                user_id,
                                                std::chrono::hours{1},
                                                expected_claims);
        benchmark::DoNotOptimize(result);

        ++token_index;
    }

    state.SetItemsProcessed(state.iterations());
}

// Benchmark license verification with caching disabled
BENCHMARK_F(LicenseBenchmarkFixture,
            bm_license_verification_no_cache)(benchmark::State &state)
{
    // Create a verifier with caching disabled
    auto no_cache_verifier =
        hj::license::create_license_verifier("benchmark_issuer",
                                             hj::license::sign_algo::rsa256,
                                             false);

    std::vector<hj::license::secure_string> verifier_keys;
    verifier_keys.emplace_back(PUBLIC_KEY);
    verifier_keys.emplace_back(PRIVATE_KEY);
    no_cache_verifier->set_keys(std::move(verifier_keys));

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

        auto result = no_cache_verifier->verify_license(token,
                                                        user_id,
                                                        std::chrono::hours{1},
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
        auto issue_result = issuer_->issue_to_file(filename,
                                                   "file_bench_user",
                                                   std::chrono::hours{24});

        if(!issue_result)
        {
            // Verify from file
            auto verify_result =
                verifier_->verify_from_file(filename,
                                            "file_bench_user",
                                            std::chrono::hours{1});
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
        auto issue_result = issuer_->issue_to_stream(oss,
                                                     "stream_bench_user",
                                                     std::chrono::hours{24});

        if(!issue_result)
        {
            std::istringstream iss(oss.str());

            // Verify from stream
            auto verify_result =
                verifier_->verify_from_stream(iss,
                                              "stream_bench_user",
                                              std::chrono::hours{1});
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

        auto license_info = verifier_->get_license_info(token);
        benchmark::DoNotOptimize(license_info);

        ++token_index;
    }

    state.SetItemsProcessed(state.iterations());
}

// Benchmark cache operations
BENCHMARK_F(LicenseBenchmarkFixture,
            bm_verifier_cache_operations)(benchmark::State &state)
{
    for(auto _ : state)
    {
        // Get cache stats
        auto stats = verifier_->get_cache_stats();
        benchmark::DoNotOptimize(stats);

        // Clear cache periodically
        if(state.iterations() % 100 == 0)
        {
            verifier_->clear_cache();
        }
    }

    state.SetItemsProcessed(state.iterations());
}

// Benchmark issuer state reset
BENCHMARK_F(LicenseBenchmarkFixture,
            bm_issuer_state_reset)(benchmark::State &state)
{
    for(auto _ : state)
    {
        std::vector<hj::license::secure_string> new_keys;
        new_keys.emplace_back(PUBLIC_KEY);
        new_keys.emplace_back(PRIVATE_KEY);

        auto result = issuer_->reset_state(std::move(new_keys));
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
        auto limited_issuer =
            hj::license::create_license_issuer("limited_issuer",
                                               hj::license::sign_algo::rsa256,
                                               max_licenses);

        std::vector<hj::license::secure_string> keys;
        keys.emplace_back(PUBLIC_KEY);
        keys.emplace_back(PRIVATE_KEY);
        limited_issuer->set_keys(std::move(keys));

        // Try to issue licenses up to the limit
        for(int i = 0; i < max_licenses; ++i)
        {
            hj::license::token_t token;
            auto                 result =
                limited_issuer->issue_license(token,
                                              "user_" + std::to_string(i),
                                              std::chrono::hours{1});
            benchmark::DoNotOptimize(result);
            if(result)
                break; // Stop if we hit an error
        }

        benchmark::DoNotOptimize(limited_issuer->issued_count());
        benchmark::DoNotOptimize(limited_issuer->remaining_licenses());
    }

    state.SetItemsProcessed(state.iterations() * max_licenses);
}

// Register simple benchmarks
BENCHMARK(bm_secure_string_construction);
BENCHMARK(bm_secure_string_move);
BENCHMARK(bm_hardware_info_access);
BENCHMARK(bm_error_code_creation);
BENCHMARK(bm_license_issuer_creation);
BENCHMARK(bm_license_verifier_creation);

// Register benchmark with different license limits
BENCHMARK(bm_license_limits)->Arg(1)->Arg(10)->Arg(100)->Arg(1000)->Arg(10000);

// Set custom benchmark parameters
BENCHMARK(bm_secure_string_construction)->Threads(1)->Threads(2)->Threads(4);
BENCHMARK(bm_hardware_info_access)->Threads(1)->Threads(2)->Threads(4);