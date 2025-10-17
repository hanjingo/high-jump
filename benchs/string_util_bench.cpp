#include <benchmark/benchmark.h>
#include <hj/util/string_util.hpp>
#include <string>
#include <vector>

// Test data setup
static const std::string small_text = "hello world";
static const std::string medium_text =
    "The quick brown fox jumps over the lazy dog. This sentence contains every "
    "letter of the English alphabet.";
static const std::string large_text = []() {
    std::string result;
    for(int i = 0; i < 1000; ++i)
    {
        result += "The quick brown fox jumps over the lazy dog. This sentence "
                  "contains every letter of the English alphabet. ";
    }
    return result;
}();

static const std::string regex_pattern  = R"(\b\w+\b)";
static const std::string number_pattern = R"(\d+)";
static const std::string email_pattern =
    R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})";

static const std::string email_text =
    "Contact us at support@example.com or sales@company.org for more "
    "information. You can also reach admin@test.net.";
static const std::string number_text =
    "The order ID is 12345, shipped on 2023-10-15, tracking number: 987654321.";

static const std::string whitespace_text =
    "   \t\n  hello world with spaces   \r\f  ";
static const std::string mixed_case_text =
    "Hello World This Is A Mixed Case String";

// Benchmark: contains function
static void bm_contains_small(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(
            hj::string_util::contains(small_text, "world"));
        benchmark::DoNotOptimize(hj::string_util::contains(small_text, "xyz"));
    }
}
BENCHMARK(bm_contains_small);

static void bm_contains_large(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(
            hj::string_util::contains(large_text, "alphabet"));
        benchmark::DoNotOptimize(
            hj::string_util::contains(large_text, "notfound"));
    }
}
BENCHMARK(bm_contains_large);

// Benchmark: starts_with function
static void bm_starts_with(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(
            hj::string_util::starts_with(medium_text, "The quick"));
        benchmark::DoNotOptimize(
            hj::string_util::starts_with(medium_text, "brown fox"));
    }
}
BENCHMARK(bm_starts_with);

// Benchmark: ends_with function
static void bm_ends_with(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(
            hj::string_util::ends_with(medium_text, "alphabet."));
        benchmark::DoNotOptimize(
            hj::string_util::ends_with(medium_text, "quick"));
    }
}
BENCHMARK(bm_ends_with);

// Benchmark: split function
static void bm_split_small(benchmark::State &state)
{
    const std::string text = "apple,banana,orange,grape,kiwi";
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(hj::string_util::split(text, ","));
    }
}
BENCHMARK(bm_split_small);

static void bm_split_large(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(hj::string_util::split(large_text, " "));
    }
}
BENCHMARK(bm_split_large);

// Benchmark: replace_all function
static void bm_replace_all_small(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(
            hj::string_util::replace_all(small_text, "o", "0"));
    }
}
BENCHMARK(bm_replace_all_small);

static void bm_replace_all_large(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(
            hj::string_util::replace_all(large_text, "the", "THE"));
    }
}
BENCHMARK(bm_replace_all_large);

static void bm_replace_all_inplace(benchmark::State &state)
{
    for(auto _ : state)
    {
        std::string text = medium_text;
        hj::string_util::replace_all_inplace(text, "o", "0");
        benchmark::DoNotOptimize(text);
    }
}
BENCHMARK(bm_replace_all_inplace);

// Benchmark: regex functions
static void bm_regex_search(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(
            hj::string_util::regex_search(number_text, number_pattern));
    }
}
BENCHMARK(bm_regex_search);

static void bm_regex_searchAll(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(
            hj::string_util::regex_search_all(email_text, email_pattern));
    }
}
BENCHMARK(bm_regex_searchAll);

static void bm_regex_split(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(
            hj::string_util::regex_split(medium_text, R"(\s+)"));
    }
}
BENCHMARK(bm_regex_split);

static void bm_regex_replace(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(
            hj::string_util::regex_replace(number_text, number_pattern, "XXX"));
    }
}
BENCHMARK(bm_regex_replace);

// Benchmark: case conversion functions
static void bm_to_upper_small(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(hj::string_util::to_upper(small_text));
    }
}
BENCHMARK(bm_to_upper_small);

static void bm_to_upper_large(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(hj::string_util::to_upper(large_text));
    }
}
BENCHMARK(bm_to_upper_large);

static void bm_to_lower_small(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(hj::string_util::to_lower(mixed_case_text));
    }
}
BENCHMARK(bm_to_lower_small);

static void bm_to_lower_large(benchmark::State &state)
{
    const std::string upper_large = hj::string_util::to_upper(large_text);
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(hj::string_util::to_lower(upper_large));
    }
}
BENCHMARK(bm_to_lower_large);

// Benchmark: trimming functions
static void bm_trim_left(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(hj::string_util::trim_left(whitespace_text));
    }
}
BENCHMARK(bm_trim_left);

static void bm_trim_right(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(hj::string_util::trim_right(whitespace_text));
    }
}
BENCHMARK(bm_trim_right);

static void bm_trim_both(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(hj::string_util::trim(whitespace_text));
    }
}
BENCHMARK(bm_trim_both);

static void bm_trim_inplace(benchmark::State &state)
{
    for(auto _ : state)
    {
        std::string text = whitespace_text;
        hj::string_util::trim_inplace(text);
        benchmark::DoNotOptimize(text);
    }
}
BENCHMARK(bm_trim_inplace);

// Benchmark: comparison functions
static void bm_equal_cstring(benchmark::State &state)
{
    const char *str1 = "hello world";
    const char *str2 = "hello world";
    const char *str3 = "hello universe";

    for(auto _ : state)
    {
        benchmark::DoNotOptimize(hj::string_util::equal(str1, str2));
        benchmark::DoNotOptimize(hj::string_util::equal(str1, str3));
    }
}
BENCHMARK(bm_equal_cstring);

static void bm_iequal_small(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(
            hj::string_util::iequal("Hello World", "hello world"));
        benchmark::DoNotOptimize(
            hj::string_util::iequal("Hello World", "hello universe"));
    }
}
BENCHMARK(bm_iequal_small);

static void bm_iequal_large(benchmark::State &state)
{
    const std::string upper_large = hj::string_util::to_upper(large_text);
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(
            hj::string_util::iequal(upper_large, large_text));
    }
}
BENCHMARK(bm_iequal_large);

// Benchmark: Unicode conversion functions
static void bm_to_wstring_ascii(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(hj::string_util::to_wstring_safe(small_text));
    }
}
BENCHMARK(bm_to_wstring_ascii);

static void bm_from_wstring_ascii(benchmark::State &state)
{
    const auto wide_str = hj::string_util::to_wstring_safe(small_text);
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(hj::string_util::from_wstring_safe(wide_str));
    }
}
BENCHMARK(bm_from_wstring_ascii);

static void bm_unicode_round_trip(benchmark::State &state)
{
    for(auto _ : state)
    {
        auto wide = hj::string_util::to_wstring_safe(medium_text);
        benchmark::DoNotOptimize(hj::string_util::from_wstring_safe(wide));
    }
}
BENCHMARK(bm_unicode_round_trip);

// Benchmark: Legacy wrapper functions
static void bm_search_legacy(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(
            hj::string_util::search(number_text, number_pattern));
    }
}
BENCHMARK(bm_search_legacy);

static void bm_search_n_legacy(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(
            hj::string_util::search_n(email_text, email_pattern));
    }
}
BENCHMARK(bm_search_n_legacy);

static void bm_split_regex_legacy(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(
            hj::string_util::split_regex(medium_text, R"(\s+)"));
    }
}
BENCHMARK(bm_split_regex_legacy);

static void bm_replace_legacy(benchmark::State &state)
{
    for(auto _ : state)
    {
        std::string text = medium_text;
        hj::string_util::replace(text, "o", "0");
        benchmark::DoNotOptimize(text);
    }
}
BENCHMARK(bm_replace_legacy);

// Benchmark: Pointer address conversion
static void bm_rrom_ptr_addr_hex(benchmark::State &state)
{
    const void *ptr = static_cast<const void *>(&small_text);
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(hj::string_util::from_ptr_addr(ptr, true));
    }
}
BENCHMARK(bm_rrom_ptr_addr_hex);

static void bm_from_ptr_addr_decimal(benchmark::State &state)
{
    const void *ptr = static_cast<const void *>(&small_text);
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(hj::string_util::from_ptr_addr(ptr, false));
    }
}
BENCHMARK(bm_from_ptr_addr_decimal);

// Benchmark: Regex cache performance
static void bm_regex_cache_hit(benchmark::State &state)
{
    // Warm up the cache
    hj::string_util::regex_search(number_text, number_pattern);

    for(auto _ : state)
    {
        // This should hit the cache
        benchmark::DoNotOptimize(
            hj::string_util::regex_search(number_text, number_pattern));
    }
}
BENCHMARK(bm_regex_cache_hit);

static void bm_regex_cache_miss(benchmark::State &state)
{
    int counter = 0;
    for(auto _ : state)
    {
        // Generate unique patterns to force cache misses
        std::string pattern = R"(\d{)" + std::to_string(counter % 50) + R"(})";
        benchmark::DoNotOptimize(
            hj::string_util::regex_search(number_text, pattern));
        ++counter;
    }
}
BENCHMARK(bm_regex_cache_miss);

// Benchmark comparison with std library functions
static void bm_std_string_find(benchmark::State &state)
{
    for(auto _ : state)
    {
        benchmark::DoNotOptimize(large_text.find("alphabet")
                                 != std::string::npos);
        benchmark::DoNotOptimize(large_text.find("notfound")
                                 != std::string::npos);
    }
}
BENCHMARK(bm_std_string_find);

// Custom arguments for parametric benchmarks
static void bm_split_various_sizes(benchmark::State &state)
{
    std::string text;
    for(int i = 0; i < state.range(0); ++i)
    {
        text += "word" + std::to_string(i) + ",";
    }

    for(auto _ : state)
    {
        benchmark::DoNotOptimize(hj::string_util::split(text, ","));
    }
}
BENCHMARK(bm_split_various_sizes)->Arg(10)->Arg(100)->Arg(1000);

static void bm_replace_various_sizes(benchmark::State &state)
{
    std::string text;
    for(int i = 0; i < state.range(0); ++i)
    {
        text += "hello world ";
    }

    for(auto _ : state)
    {
        benchmark::DoNotOptimize(hj::string_util::replace_all(text, "o", "0"));
    }
}
BENCHMARK(bm_replace_various_sizes)->Arg(10)->Arg(100)->Arg(1000);