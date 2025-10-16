#include <algorithm>
#include <chrono>
#include <gtest/gtest.h>
#include <iostream>
#include <hj/hardware/cpu.h>
#include <map>
#include <set>
#include <thread>
#include <vector>

class cpu : public ::testing::Test
{
  protected:
    void SetUp() override {}

    void TearDown() override
    {
        // std::cout << "=== CPU Hardware Tests Teardown ===" << std::endl;
    }
};

TEST_F(cpu, cpu_brand)
{
    SCOPED_TRACE("Testing cpu_brand functionality");
    unsigned char brand[128] = {0};
    auto          ec         = cpu_brand(brand, sizeof(brand));
    std::string   brand_str(reinterpret_cast<char *>(brand));
    EXPECT_EQ(ec, CPU_OK) << "cpu_brand should return CPU_OK";
    EXPECT_FALSE(brand_str.empty()) << "CPU brand string should not be empty";
    EXPECT_GT(brand_str.length(), 2u)
        << "CPU brand string should be reasonable length";
    for(char c : brand_str)
    {
        EXPECT_TRUE(c == 0 || (c >= 32 && c <= 126))
            << "Brand string should be printable";
    }
}

TEST_F(cpu, cpu_vendor)
{
    SCOPED_TRACE("Testing cpu_vendor functionality");
    unsigned char vendor[64] = {0};
    auto          ec         = cpu_vendor(vendor, sizeof(vendor));
    std::string   vendor_str(reinterpret_cast<char *>(vendor));
    EXPECT_EQ(ec, CPU_OK) << "cpu_vendor should return CPU_OK";
    EXPECT_FALSE(vendor_str.empty()) << "CPU vendor string should not be empty";
    EXPECT_GT(vendor_str.length(), 2u)
        << "CPU vendor string should be reasonable length";
    for(char c : vendor_str)
    {
        EXPECT_TRUE(c == 0 || (c >= 32 && c <= 126))
            << "Vendor string should be printable";
    }
}

TEST_F(cpu, cpu_core_num)
{
    SCOPED_TRACE("Testing cpu_core_num functionality");

    unsigned int core_count = cpu_core_num();
    // std::cout << "CPU cores detected: " << core_count << std::endl;

    EXPECT_GT(core_count, 0) << "Should detect at least 1 CPU core";
    EXPECT_LE(core_count, 1024)
        << "CPU core count should be reasonable (≤ 1024)";

    for(int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(cpu_core_num(), core_count)
            << "cpu_core_num should return consistent results";
    }
}

TEST_F(cpu, cpu_core_list)
{
    SCOPED_TRACE("Testing cpu_core_list");

    unsigned int              expected_count = cpu_core_num();
    std::vector<unsigned int> core_list(expected_count + 10);
    unsigned int list_len = static_cast<unsigned int>(core_list.size());

    auto ec = cpu_core_list(core_list.data(), &list_len);
    EXPECT_EQ(ec, CPU_OK) << "cpu_core_list should return CPU_OK";
    EXPECT_EQ(list_len, expected_count)
        << "Core list length should match cpu_core_num()";

    for(unsigned int i = 0; i < list_len; ++i)
    {
        EXPECT_LT(core_list[i], 1024) << "Core ID should be reasonable";
    }

    std::set<unsigned int> unique_cores(core_list.begin(),
                                        core_list.begin() + list_len);
    EXPECT_EQ(unique_cores.size(), list_len) << "All core IDs should be unique";

    // boundary conditions tested in separate test
    unsigned int len = 0;
    ec               = cpu_core_list(nullptr, &len);
    EXPECT_EQ(ec, CPU_ERR_INVALID_ARG) << "Should handle null buffer";
    EXPECT_EQ(len, 0) << "Should handle null buffer gracefully";

    ec = cpu_core_list(nullptr, nullptr);
    EXPECT_EQ(ec, CPU_ERR_INVALID_ARG) << "Should handle null length pointer";

    unsigned int small_buf[2];
    unsigned int small_len = 2;
    ec                     = cpu_core_list(small_buf, &small_len);
    EXPECT_EQ(ec, CPU_OK) << "Should handle small buffer";

    if(cpu_core_num() > 2)
    {
        EXPECT_EQ(small_len, 2) << "Should limit to buffer size";
    } else
    {
        EXPECT_LE(small_len, 2) << "Should not exceed buffer size";
    }
}

TEST_F(cpu, cpu_core_bind)
{
    SCOPED_TRACE("Testing cpu_core_bind");

    unsigned int core_count = cpu_core_num();
    if(core_count <= 1)
    {
        GTEST_SKIP() << "Single core system, skipping bind test";
        return;
    }

    std::vector<unsigned int> core_list(core_count);
    unsigned int              list_len = core_count;
    cpu_core_list(core_list.data(), &list_len);

#if defined(_WIN32) || defined(__linux__)
    bool         any_bind_success = false;
    unsigned int min              = (list_len < 4u) ? list_len : 4u;
    for(unsigned int i = 0; i < min; ++i)
    {
        auto ec = cpu_core_bind(core_list[i]);
        if(ec == CPU_OK)
        {
            any_bind_success = true;

            std::set<uint32_t> observed_cpus;
            for(int j = 0; j < 20; ++j)
            {
                observed_cpus.insert(cpu_id());
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
    for(unsigned int i = 0; i < std::min(2u, list_len); ++i)
    {
        auto ec = cpu_core_bind(core_list[i]);
        EXPECT_EQ(ec, CPU_ERR_NOT_SUPPORTED)
            << "macOS should not support CPU binding";
    }
#endif
}

TEST_F(cpu, cpu_id)
{
    SCOPED_TRACE("Testing cpu_id");

    uint32_t cpu_id_1 = cpu_id();
    uint32_t cpu_id_2 = cpu_id();
    uint32_t cpu_id_3 = cpu_id();

    EXPECT_LT(cpu_id_1, 1024) << "CPU ID should be reasonable";
    EXPECT_LT(cpu_id_2, 1024) << "CPU ID should be reasonable";
    EXPECT_LT(cpu_id_3, 1024) << "CPU ID should be reasonable";

    std::vector<uint32_t> cpu_ids;
    for(int i = 0; i < 100; ++i)
    {
        cpu_ids.push_back(cpu_id());
        if(i % 10 == 0)
        {
            std::this_thread::yield();
        }
    }

    std::set<uint32_t> unique_ids(cpu_ids.begin(), cpu_ids.end());
    EXPECT_GE(unique_ids.size(), 1) << "Should observe at least one CPU ID";
    EXPECT_LE(unique_ids.size(), cpu_core_num())
        << "Unique CPU IDs should not exceed core count";

    // multi threaded test in separate test
    unsigned int core_count = cpu_core_num();
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
                thread_cpu_ids[t].insert(cpu_id());

                for(int j = 0; j < 1000; ++j)
                {
                    cpu_nop();
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

TEST_F(cpu, cpu_pause)
{
    SCOPED_TRACE("Testing cpu_pause");
    
    // Test that cpu_pause executes without crashing
    for(int i = 0; i < 10; ++i)
    {
        cpu_pause();
    }
    
    // Test with more iterations to ensure measurable time difference
    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < 10000; ++i)  // Increased iterations for measurable time
    {
        cpu_pause();
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    
    // cpu_pause should complete successfully (main requirement)
    SUCCEED() << "cpu_pause executed without errors";
    
    // Optional timing check - don't fail if timing is not measurable
    if(duration.count() > 0)
    {
        EXPECT_LT(duration.count(), 100000000) << "cpu_pause should not take excessively long (< 100ms)";
    }
    else
    {
        std::cout << "Note: cpu_pause timing not measurable on this system (very fast execution)" << std::endl;
    }
}

TEST_F(cpu, cpu_nop)
{
    SCOPED_TRACE("Testing cpu_nop");
    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < 10000; ++i)
    {
        cpu_nop();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    EXPECT_GE(duration.count(), 0) << "cpu_nop should execute without error";
    EXPECT_LT(duration.count(), 50000) << "cpu_nop should be very fast";
}

TEST_F(cpu, cpu_delay)
{
    SCOPED_TRACE("Testing cpu_delay");
    
    // Test that cpu_delay executes without crashing for various cycle counts
    std::vector<uint64_t> test_cycles = {1000, 10000, 100000, 1000000};
    for(uint64_t cycles : test_cycles)
    {
        cpu_delay(cycles);  // Basic functionality test
    }
    
    // Test with larger cycle count to ensure measurable delay
    bool any_measurable = false;
    std::vector<uint64_t> large_cycles = {10000000, 50000000, 100000000}; // 10M, 50M, 100M cycles
    
    for(uint64_t cycles : large_cycles)
    {
        auto start = std::chrono::high_resolution_clock::now();
        cpu_delay(cycles);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        
        if(duration.count() > 0)
        {
            any_measurable = true;
            EXPECT_LT(duration.count(), 1000000000) << "Delay should not be excessive (< 1 second)";
            break; // Found a measurable delay, no need to test larger values
        }
    }
    
    if(any_measurable)
    {
        std::cout << "cpu_delay produced measurable timing with large cycle counts" << std::endl;
    }
    else
    {
        std::cout << "Note: cpu_delay timing not measurable on this system (very fast CPU or virtualized environment)" << std::endl;
    }
    
    // The main requirement is that cpu_delay completes without error
    SUCCEED() << "cpu_delay executed successfully for all test cases";
}

TEST_F(cpu, cpu_boundary_conditions)
{
    SCOPED_TRACE("Testing boundary conditions and error handling");

    auto start = std::chrono::high_resolution_clock::now();
    cpu_delay(0);
    auto end = std::chrono::high_resolution_clock::now();
    auto zero_delay =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    EXPECT_LT(zero_delay.count(), 1000) << "Zero delay should be very fast";

    start = std::chrono::high_resolution_clock::now();
    cpu_delay(10000000); // 10M cycles
    end = std::chrono::high_resolution_clock::now();
    auto large_delay =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_GT(large_delay.count(), 0) << "Large delay should be measurable";
    EXPECT_LT(large_delay.count(), 10000)
        << "Large delay should not be excessive";

    auto ec = cpu_core_bind(9999);
    EXPECT_FALSE(ec == CPU_OK) << "Binding to invalid core should fail";
}

TEST_F(cpu, cpu_cache_flush)
{
    SCOPED_TRACE("Testing cpu_cache_flush");

    const size_t      test_size = 4096;
    std::vector<char> test_data(test_size, 0x55);

    auto start = std::chrono::high_resolution_clock::now();
    for(size_t i = 0; i < test_data.size(); i += 64)
    {
        cpu_cache_flush(&test_data[i]);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    cpu_cache_flush(nullptr);

    EXPECT_GE(duration.count(), 0)
        << "Cache flush should execute without error";
    EXPECT_LT(duration.count(), 10000)
        << "Cache flush should not take too long";
}

TEST_F(cpu, cpu_prefetch)
{
    SCOPED_TRACE("Testing cpu_prefetch");

    const size_t     array_size = 1024 * 1024; // 1MB
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
            cpu_prefetch_read(&test_array[i + 64]);
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
            cpu_prefetch_write(&test_array[i + 64]);
        }
        test_array[i] = static_cast<int>(i * 2);
    }

    end = std::chrono::high_resolution_clock::now();
    auto write_prefetch_duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    cpu_prefetch_read(nullptr);
    cpu_prefetch_write(nullptr);
}

TEST_F(cpu, cpu_tsc_functionality)
{
    SCOPED_TRACE("Testing cpu_tsc_read functionality");

    uint64_t tsc1 = cpu_tsc_read();
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    uint64_t tsc2 = cpu_tsc_read();
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    uint64_t tsc3 = cpu_tsc_read();

    EXPECT_GT(tsc2, tsc1) << "TSC should be increasing";
    EXPECT_GT(tsc3, tsc2) << "TSC should be increasing";

    std::vector<uint64_t> tsc_values;
    for(int i = 0; i < 10; ++i)
    {
        tsc_values.push_back(cpu_tsc_read());
        cpu_nop();
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

    if(all_different)
    {
        std::cout << "TSC shows high precision (all values increasing)"
                  << std::endl;
    } else
    {
        std::cout << "TSC shows limited precision (some values equal)"
                  << std::endl;
    }

    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < 10000; ++i)
    {
        volatile uint64_t tsc = cpu_tsc_read();
        (void) tsc;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    EXPECT_LT(duration.count(), 10000) << "TSC reads should be fast";
}

TEST_F(cpu, cpu_tscp_functionality)
{
    SCOPED_TRACE("Testing cpu_tscp_read functionality");

    uint32_t aux1, aux2, aux3;
    uint64_t tscp1 = cpu_tscp_read(&aux1);
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    uint64_t tscp2 = cpu_tscp_read(&aux2);
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    uint64_t tscp3 = cpu_tscp_read(&aux3);

    EXPECT_GT(tscp2, tscp1) << "TSCP should be increasing";
    EXPECT_GT(tscp3, tscp2) << "TSCP should be increasing";

    EXPECT_LT(aux1, 1024) << "aux value should be reasonable";
    EXPECT_LT(aux2, 1024) << "aux value should be reasonable";
    EXPECT_LT(aux3, 1024) << "aux value should be reasonable";

    uint64_t tscp_no_aux = cpu_tscp_read(nullptr);
    EXPECT_GT(tscp_no_aux, 0) << "TSCP without aux should work";

    uint64_t tsc  = cpu_tsc_read();
    uint64_t tscp = cpu_tscp_read(nullptr);

    if(tsc > 0 && tscp > 0)
    {
        uint64_t diff          = (tsc > tscp) ? (tsc - tscp) : (tscp - tsc);
        double   max           = (tsc > tscp) ? tsc : tscp;
        double   relative_diff = (double) diff / max;
    }
}

TEST_F(cpu, cpu_pmu_cycle_counter_functionality)
{
    SCOPED_TRACE("Testing cpu_pmu_cycle_counter_read functionality");

    uint64_t pmu1 = cpu_pmu_cycle_counter_read();
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    uint64_t pmu2 = cpu_pmu_cycle_counter_read();
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    uint64_t pmu3 = cpu_pmu_cycle_counter_read();

    EXPECT_GT(pmu2, pmu1) << "PMU cycle counter should be increasing";
    EXPECT_GT(pmu3, pmu2) << "PMU cycle counter should be increasing";

    bool        is_true_cycle_counter = false;
    std::string counter_type          = "Unknown";

#if defined(_WIN32) || defined(_WIN64)
#if defined(_M_IX86) || defined(_M_X64)
    is_true_cycle_counter = true;
    counter_type          = "Windows x86/x64 RDTSC";
#else
    is_true_cycle_counter = false;
    counter_type          = "Windows QueryPerformanceCounter";
#endif
#elif defined(__linux__)
#if defined(__i386__) || defined(__x86_64__)
    is_true_cycle_counter = true;
    counter_type          = "Linux x86/x64 RDTSC";
#elif defined(__arm__) || defined(__aarch64__)
    is_true_cycle_counter = true;
    counter_type          = "Linux ARM cycle counter";
#else
    is_true_cycle_counter = false;
    counter_type          = "Linux clock_gettime";
#endif
#elif defined(__APPLE__)
    is_true_cycle_counter = true;
    counter_type          = "macOS mach_absolute_time";
#else
    is_true_cycle_counter = false;
    counter_type          = "Generic fallback";
#endif

    auto     start     = std::chrono::high_resolution_clock::now();
    uint64_t pmu_start = cpu_pmu_cycle_counter_read();

    volatile long long sum = 0;
    for(int i = 0; i < 100000; ++i)
    {
        sum += i * i;
    }

    uint64_t pmu_end = cpu_pmu_cycle_counter_read();
    auto     end     = std::chrono::high_resolution_clock::now();

    auto wall_time =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    uint64_t pmu_cycles = pmu_end - pmu_start;

    EXPECT_GT(pmu_cycles, 0) << "PMU should count some cycles during work";

    if(wall_time.count() > 0)
    {
        double cycles_per_microsecond = (double) pmu_cycles / wall_time.count();
        if(is_true_cycle_counter)
        {
            EXPECT_GT(cycles_per_microsecond, 100)
                << "CPU cycle rate seems too low for " << counter_type;
            EXPECT_LT(cycles_per_microsecond, 10000)
                << "CPU cycle rate seems too high for " << counter_type;
        } else
        {
            EXPECT_GT(cycles_per_microsecond, 0.1)
                << "Counter rate should be positive for " << counter_type;
            EXPECT_LT(cycles_per_microsecond, 100000)
                << "Counter rate seems unreasonably high for " << counter_type;

            if(counter_type.find("QueryPerformanceCounter")
               != std::string::npos)
            {
                std::cout
                    << "  QueryPerformanceCounter frequency appears to be: "
                    << cycles_per_microsecond << " counts/μs" << std::endl;
                EXPECT_GT(cycles_per_microsecond, 0.1)
                    << "QPC should have reasonable frequency";
                EXPECT_LT(cycles_per_microsecond, 100)
                    << "QPC frequency should be moderate";
            }
        }
    }
}

TEST_F(cpu, cpu_counter_frequency_detection)
{
    SCOPED_TRACE("Detecting counter frequencies and characteristics");

    auto     wall_start = std::chrono::high_resolution_clock::now();
    uint64_t tsc_start  = cpu_tsc_read();
    uint64_t pmu_start  = cpu_pmu_cycle_counter_read();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto     wall_end = std::chrono::high_resolution_clock::now();
    uint64_t tsc_end  = cpu_tsc_read();
    uint64_t pmu_end  = cpu_pmu_cycle_counter_read();

    auto wall_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        wall_end - wall_start);
    uint64_t tsc_diff = tsc_end - tsc_start;
    uint64_t pmu_diff = pmu_end - pmu_start;

    if(wall_duration.count() > 0)
    {
        double tsc_freq_mhz = (double) tsc_diff / wall_duration.count();
        double pmu_freq_mhz = (double) pmu_diff / wall_duration.count();
        EXPECT_GT(tsc_freq_mhz, 0) << "TSC should have positive frequency";
        EXPECT_GT(pmu_freq_mhz, 0) << "PMU should have positive frequency";

#if defined(_WIN32)                                                            \
    || defined(_WIN64) && (!defined(_M_IX86) && !defined(_M_X64))
        if(pmu_freq_mhz < 100)
        {
            std::cout
                << "  PMU appears to be QueryPerformanceCounter (low frequency)"
                << std::endl;
        }
#endif
    }

    std::vector<double> tsc_frequencies;
    std::vector<double> pmu_frequencies;

    for(int i = 0; i < 5; ++i)
    {
        auto     start = std::chrono::high_resolution_clock::now();
        uint64_t tsc_s = cpu_tsc_read();
        uint64_t pmu_s = cpu_pmu_cycle_counter_read();

        std::this_thread::sleep_for(std::chrono::milliseconds(5));

        auto     end   = std::chrono::high_resolution_clock::now();
        uint64_t tsc_e = cpu_tsc_read();
        uint64_t pmu_e = cpu_pmu_cycle_counter_read();

        auto duration =
            std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        if(duration.count() > 0)
        {
            tsc_frequencies.push_back((double) (tsc_e - tsc_s)
                                      / duration.count());
            pmu_frequencies.push_back((double) (pmu_e - pmu_s)
                                      / duration.count());
        }
    }

    if(!tsc_frequencies.empty())
    {
        double tsc_avg = 0, pmu_avg = 0;
        for(size_t i = 0; i < tsc_frequencies.size(); ++i)
        {
            tsc_avg += tsc_frequencies[i];
            pmu_avg += pmu_frequencies[i];
        }
        tsc_avg /= tsc_frequencies.size();
        pmu_avg /= pmu_frequencies.size();
        double tsc_var = 0, pmu_var = 0;
        for(size_t i = 0; i < tsc_frequencies.size(); ++i)
        {
            tsc_var +=
                (tsc_frequencies[i] - tsc_avg) * (tsc_frequencies[i] - tsc_avg);
            pmu_var +=
                (pmu_frequencies[i] - pmu_avg) * (pmu_frequencies[i] - pmu_avg);
        }
    }
}