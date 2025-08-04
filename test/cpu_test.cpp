#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <set>
#include <thread>
#include <vector>

#include <gtest/gtest.h>
#include <libcpp/hardware/cpu.h>

class CpuTest : public ::testing::Test
{
  protected:
    void SetUp () override
    {
        std::cout << "\n=== CPU Hardware Tests Setup ===" << std::endl;
        std::cout << "Platform: ";
#if defined(_WIN32) || defined(_WIN64)
        std::cout << "Windows" << std::endl;
#elif defined(__linux__)
        std::cout << "Linux" << std::endl;
#elif defined(__APPLE__)
        std::cout << "macOS" << std::endl;
#else
        std::cout << "Unknown" << std::endl;
#endif

        std::cout << "Compiler: ";
#if defined(_MSC_VER)
        std::cout << "MSVC " << _MSC_VER << std::endl;
#elif defined(__GNUC__)
        std::cout << "GCC " << __GNUC__ << "." << __GNUC_MINOR__ << std::endl;
#elif defined(__clang__)
        std::cout << "Clang " << __clang_major__ << "." << __clang_minor__
                  << std::endl;
#else
        std::cout << "Unknown" << std::endl;
#endif

        std::cout << "=================================" << std::endl;
    }

    void TearDown () override
    {
        std::cout << "=== CPU Hardware Tests Teardown ===" << std::endl;
    }
};

TEST_F (CpuTest, cpu_core_num_basic)
{
    SCOPED_TRACE ("Testing cpu_core_num basic functionality");

    unsigned int core_count = cpu_core_num ();
    std::cout << "CPU cores detected: " << core_count << std::endl;

    EXPECT_GT (core_count, 0) << "Should detect at least 1 CPU core";
    EXPECT_LE (core_count, 1024)
      << "CPU core count should be reasonable (≤ 1024)";

    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ (cpu_core_num (), core_count)
          << "cpu_core_num should return consistent results";
    }
}

TEST_F (CpuTest, cpu_core_list_functionality)
{
    SCOPED_TRACE ("Testing cpu_core_list functionality");

    unsigned int expected_count = cpu_core_num ();
    std::vector<unsigned int> core_list (expected_count + 10);
    unsigned int list_len = static_cast<unsigned int> (core_list.size ());

    cpu_core_list (core_list.data (), &list_len);

    EXPECT_EQ (list_len, expected_count)
      << "Core list length should match cpu_core_num()";

    std::cout << "CPU core list (" << list_len << " cores): ";
    unsigned int min = (list_len < 16u) ? list_len : 16u;
    for (unsigned int i = 0; i < min; ++i) {
        std::cout << core_list[i] << " ";
    }
    if (list_len > 16)
        std::cout << "...";
    std::cout << std::endl;

    for (unsigned int i = 0; i < list_len; ++i) {
        EXPECT_LT (core_list[i], 1024) << "Core ID should be reasonable";
    }

    std::set<unsigned int> unique_cores (core_list.begin (),
                                         core_list.begin () + list_len);
    EXPECT_EQ (unique_cores.size (), list_len)
      << "All core IDs should be unique";
}

TEST_F (CpuTest, cpu_core_list_boundary_conditions)
{
    SCOPED_TRACE ("Testing cpu_core_list boundary conditions");

    unsigned int len = 0;
    cpu_core_list (nullptr, &len);
    EXPECT_EQ (len, 0) << "Should handle null buffer gracefully";

    cpu_core_list (nullptr, nullptr);

    unsigned int small_buf[2];
    unsigned int small_len = 2;
    cpu_core_list (small_buf, &small_len);

    if (cpu_core_num () > 2) {
        EXPECT_EQ (small_len, 2) << "Should limit to buffer size";
    } else {
        EXPECT_LE (small_len, 2) << "Should not exceed buffer size";
    }
}

TEST_F (CpuTest, cpu_core_bind_functionality)
{
    SCOPED_TRACE ("Testing cpu_core_bind functionality");

    unsigned int core_count = cpu_core_num ();

    if (core_count <= 1) {
        GTEST_SKIP () << "Single core system, skipping bind test";
        return;
    }

    std::vector<unsigned int> core_list (core_count);
    unsigned int list_len = core_count;
    cpu_core_list (core_list.data (), &list_len);

#if defined(_WIN32) || defined(__linux__)
    std::cout << "Testing CPU binding on supported platform..." << std::endl;

    bool any_bind_success = false;
    unsigned int min = (list_len < 4u) ? list_len : 4u;
    for (unsigned int i = 0; i < min; ++i) {
        bool bind_result = cpu_core_bind (core_list[i]);
        std::cout << "Bind to core " << core_list[i] << ": "
                  << (bind_result ? "Success" : "Failed") << std::endl;

        if (bind_result) {
            any_bind_success = true;

            std::set<uint32_t> observed_cpus;
            for (int j = 0; j < 20; ++j) {
                observed_cpus.insert (cpu_id ());
                std::this_thread::sleep_for (std::chrono::microseconds (100));
            }

            std::cout << "  After binding, observed CPU IDs: ";
            for (uint32_t cpu : observed_cpus) {
                std::cout << cpu << " ";
            }
            std::cout << std::endl;
        }
    }

    if (!any_bind_success) {
        std::cout << "Warning: CPU binding failed (may require administrator "
                     "privileges)"
                  << std::endl;
    }

#elif defined(__APPLE__)
    std::cout << "Testing CPU binding on macOS (should return false)..."
              << std::endl;

    for (unsigned int i = 0; i < std::min (2u, list_len); ++i) {
        bool bind_result = cpu_core_bind (core_list[i]);
        EXPECT_FALSE (bind_result) << "macOS should not support CPU binding";
        std::cout << "Bind to core " << core_list[i] << ": "
                  << (bind_result ? "Success" : "Failed (expected)")
                  << std::endl;
    }
#endif
}

TEST_F (CpuTest, cpu_id_functionality)
{
    SCOPED_TRACE ("Testing cpu_id functionality");

    uint32_t cpu_id_1 = cpu_id ();
    uint32_t cpu_id_2 = cpu_id ();
    uint32_t cpu_id_3 = cpu_id ();

    std::cout << "CPU ID readings: " << cpu_id_1 << ", " << cpu_id_2 << ", "
              << cpu_id_3 << std::endl;

    EXPECT_LT (cpu_id_1, 1024) << "CPU ID should be reasonable";
    EXPECT_LT (cpu_id_2, 1024) << "CPU ID should be reasonable";
    EXPECT_LT (cpu_id_3, 1024) << "CPU ID should be reasonable";

    std::vector<uint32_t> cpu_ids;
    for (int i = 0; i < 100; ++i) {
        cpu_ids.push_back (cpu_id ());
        if (i % 10 == 0) {
            std::this_thread::yield ();
        }
    }

    std::set<uint32_t> unique_ids (cpu_ids.begin (), cpu_ids.end ());
    std::cout << "Collected " << unique_ids.size ()
              << " unique CPU IDs in 100 samples" << std::endl;

    EXPECT_GE (unique_ids.size (), 1) << "Should observe at least one CPU ID";
    EXPECT_LE (unique_ids.size (), cpu_core_num ())
      << "Unique CPU IDs should not exceed core count";
}

TEST_F (CpuTest, cpu_id_multithreaded)
{
    SCOPED_TRACE ("Testing cpu_id in multithreaded environment");

    unsigned int core_count = cpu_core_num ();
    if (core_count <= 1) {
        GTEST_SKIP () << "Single core system, skipping multithread test";
        return;
    }

    unsigned int min = (core_count < 4u) ? core_count : 4u;
    std::vector<std::thread> threads;
    std::vector<std::set<uint32_t> > thread_cpu_ids (min);
    std::cout << "Starting " << min << " threads to test CPU ID..."
              << std::endl;

    for (int t = 0; t < min; ++t) {
        threads.emplace_back ([&, t] () {
            for (int i = 0; i < 50; ++i) {
                thread_cpu_ids[t].insert (cpu_id ());

                for (int j = 0; j < 1000; ++j) {
                    cpu_nop ();
                }

                std::this_thread::sleep_for (std::chrono::microseconds (10));
            }
        });
    }

    for (auto &thread : threads) {
        thread.join ();
    }

    std::set<uint32_t> all_observed_cpus;
    for (int t = 0; t < min; ++t) {
        std::cout << "Thread " << t << " observed " << thread_cpu_ids[t].size ()
                  << " unique CPU IDs: ";
        for (uint32_t cpu : thread_cpu_ids[t]) {
            std::cout << cpu << " ";
            all_observed_cpus.insert (cpu);
        }
        std::cout << std::endl;

        EXPECT_GE (thread_cpu_ids[t].size (), 1)
          << "Each thread should see at least one CPU ID";
    }

    std::cout << "Total unique CPU IDs observed across all threads: "
              << all_observed_cpus.size () << std::endl;
}

TEST_F (CpuTest, cpu_pause_functionality)
{
    SCOPED_TRACE ("Testing cpu_pause functionality");

    std::cout << "Testing cpu_pause()..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now ();

    for (int i = 0; i < 1000; ++i) {
        cpu_pause ();
    }

    auto end = std::chrono::high_resolution_clock::now ();
    auto duration =
      std::chrono::duration_cast<std::chrono::microseconds> (end - start);

    std::cout << "1000 cpu_pause() calls took " << duration.count ()
              << " microseconds" << std::endl;

    EXPECT_GT (duration.count (), 0) << "cpu_pause should take some time";
    EXPECT_LT (duration.count (), 100000)
      << "cpu_pause should not take too long";
}

TEST_F (CpuTest, cpu_nop_functionality)
{
    SCOPED_TRACE ("Testing cpu_nop functionality");

    std::cout << "Testing cpu_nop()..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now ();

    for (int i = 0; i < 10000; ++i) {
        cpu_nop ();
    }

    auto end = std::chrono::high_resolution_clock::now ();
    auto duration =
      std::chrono::duration_cast<std::chrono::microseconds> (end - start);

    std::cout << "10000 cpu_nop() calls took " << duration.count ()
              << " microseconds" << std::endl;

    EXPECT_GE (duration.count (), 0) << "cpu_nop should execute without error";
    EXPECT_LT (duration.count (), 50000) << "cpu_nop should be very fast";
}

TEST_F (CpuTest, cpu_delay_functionality)
{
    SCOPED_TRACE ("Testing cpu_delay functionality");

    std::cout << "Testing cpu_delay() with different cycle counts..."
              << std::endl;

    std::vector<uint64_t> test_cycles = {1000, 10000, 100000, 1000000};

    for (uint64_t cycles : test_cycles) {
        auto start = std::chrono::high_resolution_clock::now ();

        cpu_delay (cycles);

        auto end = std::chrono::high_resolution_clock::now ();
        auto duration =
          std::chrono::duration_cast<std::chrono::microseconds> (end - start);

        std::cout << "cpu_delay(" << cycles << ") took " << duration.count ()
                  << " microseconds" << std::endl;

        EXPECT_GT (duration.count (), 0) << "Delay should be measurable";
        EXPECT_LT (duration.count (), 100000)
          << "Delay should not be excessive";
    }
}

TEST_F (CpuTest, cpu_delay_precision)
{
    SCOPED_TRACE ("Testing cpu_delay precision");

    std::cout << "Testing cpu_delay precision..." << std::endl;

    const uint64_t test_cycles = 50000;
    const int num_tests = 5;
    std::vector<long long> durations;

    for (int i = 0; i < num_tests; ++i) {
        auto start = std::chrono::high_resolution_clock::now ();
        cpu_delay (test_cycles);
        auto end = std::chrono::high_resolution_clock::now ();

        auto duration =
          std::chrono::duration_cast<std::chrono::microseconds> (end - start);
        durations.push_back (duration.count ());

        std::cout << "Test " << (i + 1) << ": " << duration.count ()
                  << " microseconds" << std::endl;
    }

    long long sum = 0;
    for (long long d : durations) {
        sum += d;
    }
    long long avg = sum / num_tests;

    long long variance_sum = 0;
    for (long long d : durations) {
        long long diff = d - avg;
        variance_sum += diff * diff;
    }
    double std_dev = std::sqrt (static_cast<double> (variance_sum) / num_tests);

    std::cout << "Average: " << avg << " μs, Standard deviation: " << std_dev
              << " μs" << std::endl;

    if (avg > 0) {
        double cv = std_dev / avg;
        std::cout << "Coefficient of variation: " << cv << std::endl;
        EXPECT_LT (cv, 1.0) << "Delay should be relatively consistent";
    }
}

TEST_F (CpuTest, cpu_boundary_conditions)
{
    SCOPED_TRACE ("Testing boundary conditions and error handling");

    std::cout << "Testing boundary conditions..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now ();
    cpu_delay (0);
    auto end = std::chrono::high_resolution_clock::now ();
    auto zero_delay =
      std::chrono::duration_cast<std::chrono::microseconds> (end - start);

    std::cout << "cpu_delay(0) took " << zero_delay.count () << " microseconds"
              << std::endl;
    EXPECT_LT (zero_delay.count (), 1000) << "Zero delay should be very fast";

    start = std::chrono::high_resolution_clock::now ();
    cpu_delay (10000000); // 10M cycles
    end = std::chrono::high_resolution_clock::now ();
    auto large_delay =
      std::chrono::duration_cast<std::chrono::milliseconds> (end - start);

    std::cout << "cpu_delay(10000000) took " << large_delay.count ()
              << " milliseconds" << std::endl;
    EXPECT_GT (large_delay.count (), 0) << "Large delay should be measurable";
    EXPECT_LT (large_delay.count (), 10000)
      << "Large delay should not be excessive";

    bool invalid_bind = cpu_core_bind (9999);
    EXPECT_FALSE (invalid_bind) << "Binding to invalid core should fail";

    std::cout << "Boundary condition tests completed" << std::endl;
}

TEST (cpu, cpu_core_num)
{
    ASSERT_EQ (cpu_core_num () > 0, true);
}

TEST (cpu, cpu_core_bind)
{
#if defined(_WIN32) || defined(__linux__)
    unsigned int buf[1024];
    unsigned int sz = 1024;
    cpu_core_list (buf, &sz);
    ASSERT_EQ (sz != 0, true);

    unsigned int test_count = (sz < 4u) ? sz : 4u;
    bool any_success = false;

    for (unsigned int i = 0; i < test_count; ++i) {
        bool result = cpu_core_bind (buf[i]);
        if (result) {
            any_success = true;
        }
        std::cout << "Bind to core " << buf[i] << ": "
                  << (result ? "Success" : "Failed") << std::endl;
    }

    if (!any_success) {
        std::cout << "Warning: All CPU bindings failed (may require "
                     "administrator privileges)"
                  << std::endl;
    }
#endif
}

TEST (cpu, cpu_core_list)
{
    unsigned int buf[1024];
    unsigned int sz = 1024;
    cpu_core_list (buf, &sz);
    ASSERT_EQ (sz != 0, true);

    std::cout << "CPU cores (" << sz << " total): ";
    for (unsigned int i = 0; i < sz; ++i) {
        std::cout << "cpu core:" << buf[i];
        if (i < sz - 1)
            std::cout << ", ";
    }
    std::cout << std::endl;
}

TEST_F (CpuTest, cpu_cache_flush_functionality)
{
    SCOPED_TRACE ("Testing cpu_cache_flush functionality");

    std::cout << "Testing cpu_cache_flush()..." << std::endl;

    const size_t test_size = 4096;
    std::vector<char> test_data (test_size, 0x55);

    auto start = std::chrono::high_resolution_clock::now ();
    for (size_t i = 0; i < test_data.size (); i += 64) {
        cpu_cache_flush (&test_data[i]);
    }
    auto end = std::chrono::high_resolution_clock::now ();
    auto duration =
      std::chrono::duration_cast<std::chrono::microseconds> (end - start);

    std::cout << "Cache flush for " << test_size << " bytes took "
              << duration.count () << " microseconds" << std::endl;

    cpu_cache_flush (nullptr);

    EXPECT_GE (duration.count (), 0)
      << "Cache flush should execute without error";
    EXPECT_LT (duration.count (), 10000)
      << "Cache flush should not take too long";

    std::cout << "cpu_cache_flush test completed" << std::endl;
}

TEST_F (CpuTest, cpu_prefetch_functionality)
{
    SCOPED_TRACE ("Testing cpu_prefetch functionality");

    std::cout << "Testing cpu_prefetch_read() and cpu_prefetch_write()..."
              << std::endl;

    const size_t array_size = 1024 * 1024; // 1MB
    std::vector<int> test_array (array_size);

    for (size_t i = 0; i < array_size; ++i) {
        test_array[i] = static_cast<int> (i);
    }

    auto start = std::chrono::high_resolution_clock::now ();

    int sum = 0;
    for (size_t i = 0; i < array_size; i += 16) {
        if (i + 64 < array_size) {
            cpu_prefetch_read (&test_array[i + 64]);
        }
        sum += test_array[i];
    }

    auto end = std::chrono::high_resolution_clock::now ();
    auto prefetch_duration =
      std::chrono::duration_cast<std::chrono::microseconds> (end - start);

    start = std::chrono::high_resolution_clock::now ();

    int sum2 = 0;
    for (size_t i = 0; i < array_size; i += 16) {
        sum2 += test_array[i];
    }

    end = std::chrono::high_resolution_clock::now ();
    auto no_prefetch_duration =
      std::chrono::duration_cast<std::chrono::microseconds> (end - start);

    std::cout << "Array sum with prefetch: " << sum << " (took "
              << prefetch_duration.count () << " μs)" << std::endl;
    std::cout << "Array sum without prefetch: " << sum2 << " (took "
              << no_prefetch_duration.count () << " μs)" << std::endl;

    EXPECT_EQ (sum, sum2) << "Results should be identical";

    start = std::chrono::high_resolution_clock::now ();

    for (size_t i = 0; i < array_size; i += 16) {
        if (i + 64 < array_size) {
            cpu_prefetch_write (&test_array[i + 64]);
        }
        test_array[i] = static_cast<int> (i * 2);
    }

    end = std::chrono::high_resolution_clock::now ();
    auto write_prefetch_duration =
      std::chrono::duration_cast<std::chrono::microseconds> (end - start);

    std::cout << "Array write with prefetch took "
              << write_prefetch_duration.count () << " μs" << std::endl;

    cpu_prefetch_read (nullptr);
    cpu_prefetch_write (nullptr);

    std::cout << "cpu_prefetch tests completed" << std::endl;
}

TEST_F (CpuTest, cpu_tsc_functionality)
{
    SCOPED_TRACE ("Testing cpu_tsc_read functionality");

    std::cout << "Testing cpu_tsc_read()..." << std::endl;

    uint64_t tsc1 = cpu_tsc_read ();
    std::this_thread::sleep_for (std::chrono::microseconds (100));
    uint64_t tsc2 = cpu_tsc_read ();
    std::this_thread::sleep_for (std::chrono::microseconds (100));
    uint64_t tsc3 = cpu_tsc_read ();

    std::cout << "TSC readings: " << tsc1 << ", " << tsc2 << ", " << tsc3
              << std::endl;

    EXPECT_GT (tsc2, tsc1) << "TSC should be increasing";
    EXPECT_GT (tsc3, tsc2) << "TSC should be increasing";

    std::vector<uint64_t> tsc_values;
    for (int i = 0; i < 10; ++i) {
        tsc_values.push_back (cpu_tsc_read ());
        cpu_nop ();
    }

    bool all_different = true;
    for (size_t i = 1; i < tsc_values.size (); ++i) {
        if (tsc_values[i] <= tsc_values[i - 1]) {
            all_different = false;
            break;
        }
    }

    if (all_different) {
        std::cout << "TSC shows high precision (all values increasing)"
                  << std::endl;
    } else {
        std::cout << "TSC shows limited precision (some values equal)"
                  << std::endl;
    }

    auto start = std::chrono::high_resolution_clock::now ();
    for (int i = 0; i < 10000; ++i) {
        volatile uint64_t tsc = cpu_tsc_read ();
        (void) tsc;
    }
    auto end = std::chrono::high_resolution_clock::now ();
    auto duration =
      std::chrono::duration_cast<std::chrono::microseconds> (end - start);

    std::cout << "10000 TSC reads took " << duration.count () << " microseconds"
              << std::endl;
    EXPECT_LT (duration.count (), 10000) << "TSC reads should be fast";
}

TEST_F (CpuTest, cpu_tscp_functionality)
{
    SCOPED_TRACE ("Testing cpu_tscp_read functionality");

    std::cout << "Testing cpu_tscp_read()..." << std::endl;

    uint32_t aux1, aux2, aux3;
    uint64_t tscp1 = cpu_tscp_read (&aux1);
    std::this_thread::sleep_for (std::chrono::microseconds (100));
    uint64_t tscp2 = cpu_tscp_read (&aux2);
    std::this_thread::sleep_for (std::chrono::microseconds (100));
    uint64_t tscp3 = cpu_tscp_read (&aux3);

    std::cout << "TSCP readings: " << tscp1 << " (aux=" << aux1 << "), "
              << tscp2 << " (aux=" << aux2 << "), " << tscp3 << " (aux=" << aux3
              << ")" << std::endl;

    EXPECT_GT (tscp2, tscp1) << "TSCP should be increasing";
    EXPECT_GT (tscp3, tscp2) << "TSCP should be increasing";

    EXPECT_LT (aux1, 1024) << "aux value should be reasonable";
    EXPECT_LT (aux2, 1024) << "aux value should be reasonable";
    EXPECT_LT (aux3, 1024) << "aux value should be reasonable";

    uint64_t tscp_no_aux = cpu_tscp_read (nullptr);
    EXPECT_GT (tscp_no_aux, 0) << "TSCP without aux should work";

    std::cout << "TSCP without aux: " << tscp_no_aux << std::endl;

    uint64_t tsc = cpu_tsc_read ();
    uint64_t tscp = cpu_tscp_read (nullptr);

    std::cout << "TSC vs TSCP comparison: TSC=" << tsc << ", TSCP=" << tscp
              << std::endl;

    if (tsc > 0 && tscp > 0) {
        uint64_t diff = (tsc > tscp) ? (tsc - tscp) : (tscp - tsc);
        double max = (tsc > tscp) ? tsc : tscp;
        double relative_diff = (double) diff / max;
        std::cout << "Relative difference between TSC and TSCP: "
                  << (relative_diff * 100) << "%" << std::endl;
    }
}

TEST_F (CpuTest, cpu_pmu_cycle_counter_functionality)
{
    SCOPED_TRACE ("Testing cpu_pmu_cycle_counter_read functionality");

    std::cout << "Testing cpu_pmu_cycle_counter_read()..." << std::endl;

    uint64_t pmu1 = cpu_pmu_cycle_counter_read ();
    std::this_thread::sleep_for (std::chrono::microseconds (100));
    uint64_t pmu2 = cpu_pmu_cycle_counter_read ();
    std::this_thread::sleep_for (std::chrono::microseconds (100));
    uint64_t pmu3 = cpu_pmu_cycle_counter_read ();

    std::cout << "PMU cycle counter readings: " << pmu1 << ", " << pmu2 << ", "
              << pmu3 << std::endl;

    EXPECT_GT (pmu2, pmu1) << "PMU cycle counter should be increasing";
    EXPECT_GT (pmu3, pmu2) << "PMU cycle counter should be increasing";

    bool is_true_cycle_counter = false;
    std::string counter_type = "Unknown";

#if defined(_WIN32) || defined(_WIN64)
#if defined(_M_IX86) || defined(_M_X64)
    is_true_cycle_counter = true;
    counter_type = "Windows x86/x64 RDTSC";
#else
    is_true_cycle_counter = false;
    counter_type = "Windows QueryPerformanceCounter";
#endif
#elif defined(__linux__)
#if defined(__i386__) || defined(__x86_64__)
    is_true_cycle_counter = true;
    counter_type = "Linux x86/x64 RDTSC";
#elif defined(__arm__) || defined(__aarch64__)
    is_true_cycle_counter = true;
    counter_type = "Linux ARM cycle counter";
#else
    is_true_cycle_counter = false;
    counter_type = "Linux clock_gettime";
#endif
#elif defined(__APPLE__)
    is_true_cycle_counter = true;
    counter_type = "macOS mach_absolute_time";
#else
    is_true_cycle_counter = false;
    counter_type = "Generic fallback";
#endif

    std::cout << "Detected counter type: " << counter_type << std::endl;
    std::cout << "Is true cycle counter: "
              << (is_true_cycle_counter ? "Yes" : "No") << std::endl;

    auto start = std::chrono::high_resolution_clock::now ();
    uint64_t pmu_start = cpu_pmu_cycle_counter_read ();

    volatile long long sum = 0;
    for (int i = 0; i < 100000; ++i) {
        sum += i * i;
    }

    uint64_t pmu_end = cpu_pmu_cycle_counter_read ();
    auto end = std::chrono::high_resolution_clock::now ();

    auto wall_time =
      std::chrono::duration_cast<std::chrono::microseconds> (end - start);
    uint64_t pmu_cycles = pmu_end - pmu_start;

    std::cout << "Workload results:" << std::endl;
    std::cout << "  Wall time: " << wall_time.count () << " microseconds"
              << std::endl;
    std::cout << "  PMU cycles: " << pmu_cycles << std::endl;
    std::cout << "  Sum result: " << sum << std::endl;

    EXPECT_GT (pmu_cycles, 0) << "PMU should count some cycles during work";

    if (wall_time.count () > 0) {
        double cycles_per_microsecond =
          (double) pmu_cycles / wall_time.count ();
        std::cout << "  Estimated cycles per microsecond: "
                  << cycles_per_microsecond << std::endl;

        if (is_true_cycle_counter) {
            EXPECT_GT (cycles_per_microsecond, 100)
              << "CPU cycle rate seems too low for " << counter_type;
            EXPECT_LT (cycles_per_microsecond, 10000)
              << "CPU cycle rate seems too high for " << counter_type;
        } else {
            std::cout << "  Note: This is not a true CPU cycle counter, so "
                         "frequency checks are relaxed"
                      << std::endl;
            EXPECT_GT (cycles_per_microsecond, 0.1)
              << "Counter rate should be positive for " << counter_type;
            EXPECT_LT (cycles_per_microsecond, 100000)
              << "Counter rate seems unreasonably high for " << counter_type;

            if (counter_type.find ("QueryPerformanceCounter")
                != std::string::npos) {
                std::cout
                  << "  QueryPerformanceCounter frequency appears to be: "
                  << cycles_per_microsecond << " counts/μs" << std::endl;
                EXPECT_GT (cycles_per_microsecond, 0.1)
                  << "QPC should have reasonable frequency";
                EXPECT_LT (cycles_per_microsecond, 100)
                  << "QPC frequency should be moderate";
            }
        }
    }
}

TEST_F (CpuTest, cpu_counter_frequency_detection)
{
    SCOPED_TRACE ("Detecting counter frequencies and characteristics");

    std::cout << "Detecting counter frequencies..." << std::endl;

    auto wall_start = std::chrono::high_resolution_clock::now ();
    uint64_t tsc_start = cpu_tsc_read ();
    uint64_t pmu_start = cpu_pmu_cycle_counter_read ();

    std::this_thread::sleep_for (std::chrono::milliseconds (10));

    auto wall_end = std::chrono::high_resolution_clock::now ();
    uint64_t tsc_end = cpu_tsc_read ();
    uint64_t pmu_end = cpu_pmu_cycle_counter_read ();

    auto wall_duration = std::chrono::duration_cast<std::chrono::microseconds> (
      wall_end - wall_start);
    uint64_t tsc_diff = tsc_end - tsc_start;
    uint64_t pmu_diff = pmu_end - pmu_start;

    std::cout << "Frequency detection results (over " << wall_duration.count ()
              << " μs):" << std::endl;

    if (wall_duration.count () > 0) {
        double tsc_freq_mhz = (double) tsc_diff / wall_duration.count ();
        double pmu_freq_mhz = (double) pmu_diff / wall_duration.count ();

        std::cout << "  TSC frequency: " << tsc_freq_mhz << " MHz" << std::endl;
        std::cout << "  PMU frequency: " << pmu_freq_mhz << " MHz" << std::endl;

        EXPECT_GT (tsc_freq_mhz, 0) << "TSC should have positive frequency";
        EXPECT_GT (pmu_freq_mhz, 0) << "PMU should have positive frequency";

        if (tsc_freq_mhz > 100 && pmu_freq_mhz > 100) {
            double freq_ratio = tsc_freq_mhz / pmu_freq_mhz;
            std::cout << "  TSC/PMU frequency ratio: " << freq_ratio
                      << std::endl;
            if (freq_ratio > 0.5 && freq_ratio < 2.0) {
                std::cout << "  Both counters appear to be CPU cycle counters"
                          << std::endl;
            }
        }

#if defined(_WIN32)                                                            \
  || defined(_WIN64) && (!defined(_M_IX86) && !defined(_M_X64))
        if (pmu_freq_mhz < 100) {
            std::cout
              << "  PMU appears to be QueryPerformanceCounter (low frequency)"
              << std::endl;
        }
#endif
    }

    std::vector<double> tsc_frequencies;
    std::vector<double> pmu_frequencies;

    for (int i = 0; i < 5; ++i) {
        auto start = std::chrono::high_resolution_clock::now ();
        uint64_t tsc_s = cpu_tsc_read ();
        uint64_t pmu_s = cpu_pmu_cycle_counter_read ();

        std::this_thread::sleep_for (std::chrono::milliseconds (5));

        auto end = std::chrono::high_resolution_clock::now ();
        uint64_t tsc_e = cpu_tsc_read ();
        uint64_t pmu_e = cpu_pmu_cycle_counter_read ();

        auto duration =
          std::chrono::duration_cast<std::chrono::microseconds> (end - start);
        if (duration.count () > 0) {
            tsc_frequencies.push_back ((double) (tsc_e - tsc_s)
                                       / duration.count ());
            pmu_frequencies.push_back ((double) (pmu_e - pmu_s)
                                       / duration.count ());
        }
    }

    if (!tsc_frequencies.empty ()) {
        double tsc_avg = 0, pmu_avg = 0;
        for (size_t i = 0; i < tsc_frequencies.size (); ++i) {
            tsc_avg += tsc_frequencies[i];
            pmu_avg += pmu_frequencies[i];
        }
        tsc_avg /= tsc_frequencies.size ();
        pmu_avg /= pmu_frequencies.size ();

        std::cout << "Consistency test results:" << std::endl;
        std::cout << "  TSC average frequency: " << tsc_avg << " MHz"
                  << std::endl;
        std::cout << "  PMU average frequency: " << pmu_avg << " MHz"
                  << std::endl;

        double tsc_var = 0, pmu_var = 0;
        for (size_t i = 0; i < tsc_frequencies.size (); ++i) {
            tsc_var +=
              (tsc_frequencies[i] - tsc_avg) * (tsc_frequencies[i] - tsc_avg);
            pmu_var +=
              (pmu_frequencies[i] - pmu_avg) * (pmu_frequencies[i] - pmu_avg);
        }

        if (tsc_avg > 0) {
            double tsc_cv =
              std::sqrt (tsc_var / tsc_frequencies.size ()) / tsc_avg;
            std::cout << "  TSC frequency stability (CV): " << (tsc_cv * 100)
                      << "%" << std::endl;
            EXPECT_LT (tsc_cv, 0.1) << "TSC frequency should be stable";
        }

        if (pmu_avg > 0) {
            double pmu_cv =
              std::sqrt (pmu_var / pmu_frequencies.size ()) / pmu_avg;
            std::cout << "  PMU frequency stability (CV): " << (pmu_cv * 100)
                      << "%" << std::endl;
            EXPECT_LT (pmu_cv, 0.1) << "PMU frequency should be stable";
        }
    }
}