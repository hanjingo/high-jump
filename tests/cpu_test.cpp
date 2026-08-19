#include <algorithm>
#include <chrono>
#include <gtest/gtest.h>
#include <iostream>
#include <map>
#include <set>
#include <thread>
#include <vector>

#define HJ_CPU_IMPL
#include <hj/hardware/cpu.h>

TEST(cpu, hj_cpu_has_feature)
{
    SCOPED_TRACE("Testing hj_cpu_has_feature functionality");

    bool has_tsc      = hj_cpu_has_feature(HJ_CPU_FEATURE_TSC);
    bool has_rdtscp   = hj_cpu_has_feature(HJ_CPU_FEATURE_RDTSCP);
    bool has_clflush  = hj_cpu_has_feature(HJ_CPU_FEATURE_CLFLUSH);
    bool has_prefetch = hj_cpu_has_feature(HJ_CPU_FEATURE_PREFETCH);
    bool has_affinity = hj_cpu_has_feature(HJ_CPU_FEATURE_CPU_AFFINITY);
    bool has_pmu      = hj_cpu_has_feature(HJ_CPU_FEATURE_PMU);
    bool has_pause    = hj_cpu_has_feature(HJ_CPU_FEATURE_PAUSE);

    std::cout << "CPU Features support status:" << std::endl;
    std::cout << "  - TSC: " << (has_tsc ? "yes" : "no") << std::endl;
    std::cout << "  - RDTSCP: " << (has_rdtscp ? "yes" : "no") << std::endl;
    std::cout << "  - CLFLUSH: " << (has_clflush ? "yes" : "no") << std::endl;
    std::cout << "  - PREFETCH: " << (has_prefetch ? "yes" : "no") << std::endl;
    std::cout << "  - CPU_AFFINITY: " << (has_affinity ? "yes" : "no")
              << std::endl;
    std::cout << "  - PMU: " << (has_pmu ? "yes" : "no") << std::endl;
    std::cout << "  - PAUSE: " << (has_pause ? "yes" : "no") << std::endl;

#if defined(__aarch64__) || defined(__arm__) || defined(_M_ARM)                \
    || defined(_M_ARM64)
    EXPECT_TRUE(hj_cpu_has_feature(HJ_CPU_FEATURE_TSC));
    EXPECT_FALSE(hj_cpu_has_feature(HJ_CPU_FEATURE_RDTSCP));
    EXPECT_TRUE(hj_cpu_has_feature(HJ_CPU_FEATURE_PREFETCH));
    EXPECT_TRUE(hj_cpu_has_feature(HJ_CPU_FEATURE_PMU));
    EXPECT_TRUE(hj_cpu_has_feature(HJ_CPU_FEATURE_PAUSE));
#elif defined(_M_IX86) || defined(_M_X64) || defined(__i386__)                 \
    || defined(__x86_64__)
    EXPECT_TRUE(hj_cpu_has_feature(HJ_CPU_FEATURE_PREFETCH));
    EXPECT_TRUE(hj_cpu_has_feature(HJ_CPU_FEATURE_PMU));
    EXPECT_TRUE(hj_cpu_has_feature(HJ_CPU_FEATURE_PAUSE));
#endif

    bool invalid_feature =
        hj_cpu_has_feature(static_cast<hj_cpu_feature_t>(9999));
    EXPECT_FALSE(invalid_feature) << "Invalid feature enum should return false";

    SUCCEED() << "hj_cpu_has_feature executed successfully";
}

TEST(cpu, hj_cpu_brand)
{
    SCOPED_TRACE("Testing hj_cpu_brand functionality");
    char        brand[128] = {0};
    auto        ec         = hj_cpu_brand(brand, sizeof(brand));
    std::string brand_str(brand);
    EXPECT_EQ(ec, HJ_CPU_OK) << "hj_cpu_brand should return HJ_CPU_OK";
    EXPECT_FALSE(brand_str.empty()) << "CPU brand string should not be empty";
    EXPECT_GT(brand_str.length(), 2u)
        << "CPU brand string should be reasonable length";
    for(char c : brand_str)
    {
        EXPECT_TRUE(c == 0 || (c >= 32 && c <= 126))
            << "Brand string should be printable";
    }
}

TEST(cpu, hj_cpu_vendor)
{
    SCOPED_TRACE("Testing hj_cpu_vendor functionality");
    char        vendor[64] = {0};
    auto        ec         = hj_cpu_vendor(vendor, sizeof(vendor));
    std::string vendor_str(vendor);

#if defined(__APPLE__) && defined(__aarch64__)
    // On Apple Silicon (ARM), machdep.cpu.vendor is not available
    // The sysctlbyname call will fail with HJ_CPU_ERR_SYSCTL_FAILED (-8)
    if(ec == HJ_CPU_ERR_NOT_SUPPORTED)
    {
        GTEST_SKIP() << "hj_cpu_vendor not supported on Apple Silicon (ARM64)";
        return;
    }
#endif

    EXPECT_EQ(ec, HJ_CPU_OK) << "hj_cpu_vendor should return HJ_CPU_OK";
    EXPECT_FALSE(vendor_str.empty()) << "CPU vendor string should not be empty";
    EXPECT_GT(vendor_str.length(), 2u)
        << "CPU vendor string should be reasonable length";
    for(char c : vendor_str)
    {
        EXPECT_TRUE(c == 0 || (c >= 32 && c <= 126))
            << "Vendor string should be printable";
    }
}

TEST(cpu, hj_cpu_logical_core_num)
{
    SCOPED_TRACE("Testing hj_cpu_logical_core_num functionality");

    EXPECT_EQ(hj_cpu_logical_core_num(nullptr), HJ_CPU_ERR_INVALID_ARG)
        << "Should handle null pointer parameter";

    unsigned int core_count = 0;
    auto         ec         = hj_cpu_logical_core_num(&core_count);
    EXPECT_EQ(ec, HJ_CPU_OK)
        << "hj_cpu_logical_core_num should return HJ_CPU_OK";

    EXPECT_GT(core_count, 0u) << "Should detect at least 1 CPU core";
    EXPECT_LE(core_count, 1024u)
        << "CPU core count should be reasonable (≤ 1024)";

    for(int i = 0; i < 5; ++i)
    {
        unsigned int temp_count = 0;
        EXPECT_EQ(hj_cpu_logical_core_num(&temp_count), HJ_CPU_OK);
        EXPECT_EQ(temp_count, core_count)
            << "hj_cpu_logical_core_num should return consistent results";
    }
}

TEST(cpu, hj_cpu_logical_core_list)
{
    SCOPED_TRACE("Testing hj_cpu_logical_core_list");

    unsigned int expected_count = 0;
    ASSERT_EQ(hj_cpu_logical_core_num(&expected_count), HJ_CPU_OK);

    std::vector<unsigned int> core_list(expected_count + 10);
    unsigned int list_len = static_cast<unsigned int>(core_list.size());

    auto ec = hj_cpu_logical_core_list(core_list.data(), &list_len);
    EXPECT_EQ(ec, HJ_CPU_OK)
        << "hj_cpu_logical_core_list should return HJ_CPU_OK";
    EXPECT_EQ(list_len, expected_count)
        << "Core list length should match hj_cpu_logical_core_num()";

    for(unsigned int i = 0; i < list_len; ++i)
    {
        EXPECT_LT(core_list[i], 1024u) << "Core ID should be reasonable";
    }

    std::set<unsigned int> unique_cores(core_list.begin(),
                                        core_list.begin() + list_len);
    EXPECT_EQ(unique_cores.size(), list_len) << "All core IDs should be unique";

    unsigned int query_len = 0;
    ec                     = hj_cpu_logical_core_list(nullptr, &query_len);
    EXPECT_EQ(ec, HJ_CPU_OK)
        << "Should allow null buffer to query required length";
    EXPECT_EQ(query_len, expected_count)
        << "Queried length should match expected core count";

    ec = hj_cpu_logical_core_list(nullptr, nullptr);
    EXPECT_EQ(ec, HJ_CPU_ERR_INVALID_ARG)
        << "Should handle null length pointer";

    unsigned int small_buf[2];
    unsigned int small_len = 2;
    ec                     = hj_cpu_logical_core_list(small_buf, &small_len);
    EXPECT_EQ(ec, HJ_CPU_OK) << "Should handle small buffer";

    if(expected_count > 2)
    {
        EXPECT_EQ(small_len, 2u) << "Should limit to buffer size";
    } else
    {
        EXPECT_LE(small_len, 2u) << "Should not exceed buffer size";
    }
}

TEST(cpu, hj_cpu_core_bind)
{
    SCOPED_TRACE("Testing hj_cpu_core_bind");

    unsigned int core_count = 0;
    if(hj_cpu_logical_core_num(&core_count) != HJ_CPU_OK || core_count <= 1)
    {
        GTEST_SKIP() << "Single core system, skipping bind test";
        return;
    }

    std::vector<unsigned int> core_list(core_count);
    unsigned int              list_len = core_count;
    hj_cpu_logical_core_list(core_list.data(), &list_len);

#if defined(_WIN32) || defined(__linux__)
    bool         any_bind_success = false;
    unsigned int min              = (list_len < 4u) ? list_len : 4u;
    for(unsigned int i = 0; i < min; ++i)
    {
        auto ec = hj_cpu_core_bind(core_list[i]);
        if(ec == HJ_CPU_OK)
        {
            any_bind_success = true;

            std::set<uint32_t> observed_cpus;
            for(int j = 0; j < 20; ++j)
            {
                uint32_t id = 0;
                if(hj_cpu_id(&id) == HJ_CPU_OK)
                {
                    observed_cpus.insert(id);
                }
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
        }
    }

    if(!any_bind_success)
    {
        std::cout << "Warning: CPU binding failed (may require administrator "
                     "privileges)"
                  << std::endl;
    }

#elif defined(__APPLE__)
    for(unsigned int i = 0; i < (std::min) (2u, list_len); ++i)
    {
        auto ec = hj_cpu_core_bind(core_list[i]);
        EXPECT_EQ(ec, HJ_CPU_ERR_NOT_SUPPORTED)
            << "macOS should not support CPU binding";
    }
#endif
}

TEST(cpu, hj_cpu_id)
{
    SCOPED_TRACE("Testing hj_cpu_id");

    EXPECT_EQ(hj_cpu_id(nullptr), HJ_CPU_ERR_INVALID_ARG)
        << "Should return HJ_CPU_ERR_INVALID_ARG for null pointer";

    uint32_t cpu_id_1 = 0, cpu_id_2 = 0, cpu_id_3 = 0;
    auto     ec1 = hj_cpu_id(&cpu_id_1);

    if(ec1 == HJ_CPU_ERR_NOT_SUPPORTED)
    {
        GTEST_SKIP() << "hj_cpu_id is not supported on this platform";
        return;
    }

    EXPECT_EQ(ec1, HJ_CPU_OK);
    EXPECT_EQ(hj_cpu_id(&cpu_id_2), HJ_CPU_OK);
    EXPECT_EQ(hj_cpu_id(&cpu_id_3), HJ_CPU_OK);

    EXPECT_LT(cpu_id_1, 1024u) << "CPU ID should be reasonable";
    EXPECT_LT(cpu_id_2, 1024u) << "CPU ID should be reasonable";
    EXPECT_LT(cpu_id_3, 1024u) << "CPU ID should be reasonable";

    std::vector<uint32_t> cpu_ids;
    for(int i = 0; i < 100; ++i)
    {
        uint32_t id = 0;
        if(hj_cpu_id(&id) == HJ_CPU_OK)
        {
            cpu_ids.push_back(id);
        }
        if(i % 10 == 0)
        {
            std::this_thread::yield();
        }
    }

    std::set<uint32_t> unique_ids(cpu_ids.begin(), cpu_ids.end());
    EXPECT_GE(unique_ids.size(), 1u) << "Should observe at least one CPU ID";

    unsigned int core_count = 0;
    ASSERT_EQ(hj_cpu_logical_core_num(&core_count), HJ_CPU_OK);
    EXPECT_LE(unique_ids.size(), core_count)
        << "Unique CPU IDs should not exceed core count";

    if(core_count <= 1)
    {
        GTEST_SKIP() << "Single core system, skipping multithread test";
        return;
    }

    unsigned int                    min = (core_count < 4u) ? core_count : 4u;
    std::vector<std::thread>        threads;
    std::vector<std::set<uint32_t>> thread_cpu_ids(min);
    for(int t = 0; t < min; ++t)
    {
        threads.emplace_back([&, t]() {
            for(int i = 0; i < 50; ++i)
            {
                uint32_t id = 0;
                if(hj_cpu_id(&id) == HJ_CPU_OK)
                {
                    thread_cpu_ids[t].insert(id);
                }

                for(int j = 0; j < 1000; ++j)
                {
                    hj_cpu_nop();
                }

                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        });
    }

    for(auto &thread : threads)
    {
        thread.join();
    }
}

TEST(cpu, hj_cpu_pause)
{
    SCOPED_TRACE("Testing hj_cpu_pause");

    for(int i = 0; i < 10; ++i)
    {
        hj_cpu_pause();
    }

    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < 10000; ++i)
    {
        hj_cpu_pause();
    }
    auto end = std::chrono::high_resolution_clock::now();

    auto duration =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    SUCCEED() << "hj_cpu_pause executed without errors";

    if(duration.count() > 0)
    {
        EXPECT_LT(duration.count(), 500000000)
            << "hj_cpu_pause should not take excessively long (< 500ms)";
    }
}

TEST(cpu, hj_cpu_nop)
{
    SCOPED_TRACE("Testing hj_cpu_nop");
    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < 10000; ++i)
    {
        hj_cpu_nop();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    EXPECT_GE(duration.count(), 0) << "hj_cpu_nop should execute without error";
    EXPECT_LT(duration.count(), 50000) << "hj_cpu_nop should be very fast";
}

TEST(cpu, hj_cpu_delay_ticks)
{
    SCOPED_TRACE("Testing hj_cpu_delay_ticks");

    std::vector<uint64_t> test_ticks = {1000, 10000, 100000, 1000000};
    for(uint64_t ticks : test_ticks)
    {
        hj_cpu_delay_ticks(ticks);
    }

    bool                  any_measurable = false;
    std::vector<uint64_t> large_ticks    = {10000000, 50000000, 100000000};

    for(uint64_t ticks : large_ticks)
    {
        auto start = std::chrono::high_resolution_clock::now();
        hj_cpu_delay_ticks(ticks);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration =
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

        if(duration.count() > 0)
        {
            any_measurable = true;
            EXPECT_LT(duration.count(), 1000000000)
                << "Delay should not be excessive (< 1 second)";
            break;
        }
    }

    SUCCEED() << "hj_cpu_delay_ticks executed successfully for all test cases";
}

TEST(cpu, hj_cpu_boundary_conditions)
{
    SCOPED_TRACE("Testing boundary conditions and error handling");

    auto start = std::chrono::high_resolution_clock::now();
    hj_cpu_delay_ticks(0);
    auto end = std::chrono::high_resolution_clock::now();
    auto zero_delay =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    EXPECT_LT(zero_delay.count(), 1000) << "Zero delay should be very fast";

    start = std::chrono::high_resolution_clock::now();
    hj_cpu_delay_ticks(10000000);
    end = std::chrono::high_resolution_clock::now();
    auto large_delay =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_GT(large_delay.count(), 0) << "Large delay should be measurable";
    EXPECT_LT(large_delay.count(), 10000)
        << "Large delay should not be excessive";
}

TEST(cpu, hj_cpu_cache_flush)
{
    SCOPED_TRACE("Testing hj_cpu_cache_flush");

    const size_t      test_size = 4096;
    std::vector<char> test_data(test_size, 0x55);

    auto start = std::chrono::high_resolution_clock::now();
    for(size_t i = 0; i < test_data.size(); i += 64)
    {
        hj_cpu_cache_flush(&test_data[i]);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    hj_cpu_cache_flush(nullptr);

    EXPECT_GE(duration.count(), 0)
        << "Cache flush should execute without error";
    EXPECT_LT(duration.count(), 10000)
        << "Cache flush should not take too long";
}

TEST(cpu, hj_cpu_prefetch)
{
    SCOPED_TRACE("Testing hj_cpu_prefetch");

    const size_t     array_size = 1024 * 1024;
    std::vector<int> test_array(array_size);
    for(size_t i = 0; i < array_size; ++i)
    {
        test_array[i] = static_cast<int>(i);
    }

    auto start = std::chrono::high_resolution_clock::now();
    int  sum   = 0;
    for(size_t i = 0; i < array_size; i += 16)
    {
        if(i + 64 < array_size)
        {
            hj_cpu_prefetch_read(&test_array[i + 64]);
        }
        sum += test_array[i];
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto prefetch_duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    start    = std::chrono::high_resolution_clock::now();
    int sum2 = 0;
    for(size_t i = 0; i < array_size; i += 16)
    {
        sum2 += test_array[i];
    }

    end = std::chrono::high_resolution_clock::now();
    auto no_prefetch_duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    EXPECT_EQ(sum, sum2) << "Results should be identical";
    start = std::chrono::high_resolution_clock::now();
    for(size_t i = 0; i < array_size; i += 16)
    {
        if(i + 64 < array_size)
        {
            hj_cpu_prefetch_write(&test_array[i + 64]);
        }
        test_array[i] = static_cast<int>(i * 2);
    }

    end = std::chrono::high_resolution_clock::now();
    auto write_prefetch_duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    hj_cpu_prefetch_read(nullptr);
    hj_cpu_prefetch_write(nullptr);
}

TEST(cpu, hj_cpu_tsc_functionality)
{
    SCOPED_TRACE("Testing hj_cpu_tsc_read functionality");

    uint64_t tsc1 = hj_cpu_tsc_read();
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    uint64_t tsc2 = hj_cpu_tsc_read();
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    uint64_t tsc3 = hj_cpu_tsc_read();

    EXPECT_GT(tsc2, tsc1) << "TSC should be increasing";
    EXPECT_GT(tsc3, tsc2) << "TSC should be increasing";

    std::vector<uint64_t> tsc_values;
    for(int i = 0; i < 10; ++i)
    {
        tsc_values.push_back(hj_cpu_tsc_read());
        hj_cpu_nop();
    }

    bool all_different = true;
    for(size_t i = 1; i < tsc_values.size(); ++i)
    {
        if(tsc_values[i] <= tsc_values[i - 1])
        {
            all_different = false;
            break;
        }
    }

    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < 10000; ++i)
    {
        volatile uint64_t tsc = hj_cpu_tsc_read();
        (void) tsc;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    EXPECT_LT(duration.count(), 10000) << "TSC reads should be fast";
}

TEST(cpu, hj_cpu_tsc_frequency)
{
    SCOPED_TRACE("Testing hj_cpu_tsc_frequency functionality");

    uint64_t freq = hj_cpu_tsc_frequency();
    std::cout << "Detected TSC frequency: " << freq << " Hz ("
              << (freq / 1000000.0) << " MHz)" << std::endl;

    if(freq > 0)
    {
        EXPECT_GE(freq, 1000000ULL) << "TSC frequency should be at least 1 MHz";
        EXPECT_LE(freq, 100ULL * 1000 * 1000 * 1000)
            << "TSC frequency should be reasonable (<= 100 GHz)";
    } else
    {
        SUCCEED() << "TSC frequency detection returned 0 (may not be supported "
                     "on this environment)";
    }
}

TEST(cpu, hj_cpu_tscp_functionality)
{
    SCOPED_TRACE("Testing hj_cpu_tscp_read functionality");

    uint32_t aux1, aux2, aux3;
    uint64_t tscp1 = hj_cpu_tscp_read(&aux1);
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    uint64_t tscp2 = hj_cpu_tscp_read(&aux2);
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    uint64_t tscp3 = hj_cpu_tscp_read(&aux3);

    EXPECT_GT(tscp2, tscp1) << "TSCP should be increasing";
    EXPECT_GT(tscp3, tscp2) << "TSCP should be increasing";

    EXPECT_LT(aux1, 1024u) << "aux value should be reasonable";
    EXPECT_LT(aux2, 1024u) << "aux value should be reasonable";
    EXPECT_LT(aux3, 1024u) << "aux value should be reasonable";

    uint64_t tscp_no_aux = hj_cpu_tscp_read(nullptr);
    EXPECT_GT(tscp_no_aux, 0u) << "TSCP without aux should work";
}

TEST(cpu, hj_cpu_tsc_serialization)
{
    SCOPED_TRACE("Testing hj_cpu_tsc_start and hj_cpu_tsc_end serialization "
                 "functionality");

    uint32_t aux       = 0;
    uint64_t start_tsc = hj_cpu_tsc_start();

    volatile uint64_t work = 0;
    for(int i = 0; i < 1000; ++i)
    {
        work += i;
    }
    (void) work;

    uint64_t end_tsc = hj_cpu_tsc_end(&aux);

    EXPECT_GT(end_tsc, start_tsc)
        << "Serialized end TSC should be greater than start TSC";

    uint64_t start_tsc_null = hj_cpu_tsc_start();
    uint64_t end_tsc_null   = hj_cpu_tsc_end(nullptr);
    EXPECT_GT(end_tsc_null, start_tsc_null)
        << "Serialized TSC with nullptr aux should work";

#if defined(_M_IX86) || defined(_M_X64) || defined(__i386__)                   \
    || defined(__x86_64__)
    EXPECT_LT(aux, 1024u)
        << "aux core ID returned by hj_cpu_tsc_end should be reasonable";
#endif
}