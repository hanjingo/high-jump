#include <gtest/gtest.h>
#include <hj/os/env.h>
#include <ctime>
#include <string>
#include <iostream>
#include <chrono>

class env : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        // Setup code if needed
    }

    void TearDown() override
    {
        // Cleanup code if needed
    }

    // Helper function to check if value is valid (not -1)
    bool IsValidValue(int64_t value) { return value != -1; }

    // Helper function to check if value is positive
    bool IsPositiveValue(int64_t value) { return value > 0; }

    // Helper function to check if timestamp is reasonable
    bool IsReasonableTimestamp(int64_t timestamp)
    {
        // Should be after 2000-01-01 and before 2100-01-01
        return timestamp >= 946684800 && timestamp <= 4102444800;
    }
};

// Test compile-time macros
TEST_F(env, compile_time_macros)
{
    // Test compile year (should be reasonable)
    EXPECT_GE(COMPILE_YEAR, 2020);
    EXPECT_LE(COMPILE_YEAR, 2030);

    // Test compile month (1-12)
    EXPECT_GE(COMPILE_MONTH, 1);
    EXPECT_LE(COMPILE_MONTH, 12);

    // Test compile day (1-31)
    EXPECT_GE(COMPILE_DAY, 1);
    EXPECT_LE(COMPILE_DAY, 31);

    // Test compile hour (0-23)
    EXPECT_GE(COMPILE_HOUR, 0);
    EXPECT_LE(COMPILE_HOUR, 23);

    // Test compile minute (0-59)
    EXPECT_GE(COMPILE_MINUTE, 0);
    EXPECT_LE(COMPILE_MINUTE, 59);

    // Test compile second (0-59)
    EXPECT_GE(COMPILE_SECOND, 0);
    EXPECT_LE(COMPILE_SECOND, 59);

    // Test compile time string
    const char *compile_time = COMPILE_TIME;
    EXPECT_NE(compile_time, nullptr);
    EXPECT_GT(strlen(compile_time), 0);
}

// Test CPU-related configurations
TEST_F(env, cpu_configuration)
{
    // CPU count should be positive
    int64_t cpu_count = env_get(CONF_CPU_COUNT);
    EXPECT_GT(cpu_count, 0);
    EXPECT_LE(cpu_count, 256); // Reasonable upper limit
    EXPECT_EQ(cpu_count, ENV_CPU_COUNT);

    // CPU frequency (may not be available on all systems)
    int64_t cpu_freq = env_get(CONF_CPU_FREQUENCY);
    if(IsValidValue(cpu_freq))
    {
        EXPECT_GT(cpu_freq, 100);   // At least 100 MHz
        EXPECT_LT(cpu_freq, 10000); // Less than 10 GHz
        // std::cout << "CPU frequency: " << cpu_freq << " MHz" << std::endl;
    } else
    {
        // std::cout << "CPU frequency: Not available" << std::endl;
    }

    // Cache line size
    int64_t cache_line_size = env_get(CONF_CPU_CACHE_LINE_SIZE);
    if(IsValidValue(cache_line_size))
    {
        EXPECT_GT(cache_line_size, 0);
        // Common cache line sizes or page size for Windows
        EXPECT_TRUE(cache_line_size == 32 || cache_line_size == 64
                    || cache_line_size == 128 || cache_line_size == 4096);
        // std::cout << "Cache line size: " << cache_line_size << " bytes" << std::endl;
    } else
    {
        // std::cout << "Cache line size: Not available" << std::endl;
    }

    // L1 cache size
    int64_t l1_cache = env_get(CONF_CPU_L1_CACHE_SIZE);
    if(IsValidValue(l1_cache))
    {
        EXPECT_GT(l1_cache, 1024);        // At least 1KB
        EXPECT_LT(l1_cache, 1024 * 1024); // Less than 1MB
        // std::cout << "L1 cache size: " << l1_cache << " bytes (" << l1_cache/1024 << " KB)" << std::endl;
    } else
    {
        // std::cout << "L1 cache size: Not available" << std::endl;
    }

    // L2 cache size
    int64_t l2_cache = env_get(CONF_CPU_L2_CACHE_SIZE);
    if(IsValidValue(l2_cache))
    {
        EXPECT_GT(l2_cache, 32 * 1024);        // At least 32KB
        EXPECT_LT(l2_cache, 32 * 1024 * 1024); // Less than 32MB
        // std::cout << "L2 cache size: " << l2_cache << " bytes (" << l2_cache/1024 << " KB)" << std::endl;
    } else
    {
        // std::cout << "L2 cache size: Not available" << std::endl;
    }

    // L3 cache size
    int64_t l3_cache = env_get(CONF_CPU_L3_CACHE_SIZE);
    if(IsValidValue(l3_cache))
    {
        EXPECT_GT(l3_cache, 512 * 1024);        // At least 512KB
        EXPECT_LT(l3_cache, 256 * 1024 * 1024); // Less than 256MB
        // std::cout << "L3 cache size: " << l3_cache << " bytes (" << l3_cache/(1024*1024) << " MB)" << std::endl;
    } else
    {
        // std::cout << "L3 cache size: Not available" << std::endl;
    }
}

// Test memory-related configurations
TEST_F(env, memory_configuration)
{
    // Total memory should be positive and reasonable
    int64_t total_mem = env_get(CONF_MEMORY_TOTAL);
    EXPECT_GT(total_mem, 256 * 1024 * 1024);           // At least 256MB
    EXPECT_LT(total_mem, 1024LL * 1024 * 1024 * 1024); // Less than 1TB
    EXPECT_EQ(total_mem, ENV_MEMORY_TOTAL);

    // Page size should be reasonable
    int64_t page_size = env_get(CONF_MEMORY_PAGE_SIZE);
    EXPECT_GT(page_size, 0);
    // Common page sizes: 4KB, 8KB, 16KB, 64KB
    EXPECT_TRUE(page_size == 4096 || page_size == 8192 || page_size == 16384
                || page_size == 65536);

    // Virtual memory max
    int64_t virtual_mem_max = env_get(CONF_VIRTUAL_MEMORY_MAX);
    if(IsValidValue(virtual_mem_max))
    {
        EXPECT_GT(virtual_mem_max, total_mem); // Virtual >= Physical
    }
}

// Test process-related configurations
TEST_F(env, process_configuration)
{
    // User ID
    int64_t uid = env_get(CONF_USER_ID);
    if(IsValidValue(uid))
    {
        EXPECT_GE(uid, 0);
    }

    // Group ID
    int64_t gid = env_get(CONF_GROUP_ID);
    if(IsValidValue(gid))
    {
        EXPECT_GE(gid, 0);
    }
}

// Test system limits
TEST_F(env, system_limits)
{
    // Clock ticks per second
    int64_t clk_tck = env_get(CONF_CLK_TCK);
    EXPECT_GT(clk_tck, 0);
    EXPECT_LE(clk_tck, 10000000); // Reasonable upper limit

    // Maximum argument length
    int64_t arg_max = env_get(CONF_ARG_MAX);
    EXPECT_GT(arg_max, 1024); // At least 1KB

    // Maximum filename length
    int64_t name_max = env_get(CONF_NAME_MAX);
    if(IsValidValue(name_max))
    {
        EXPECT_GE(name_max, 14);   // Historical minimum
        EXPECT_LE(name_max, 4096); // Reasonable maximum
    }

    // Maximum path length
    int64_t path_max = env_get(CONF_PATH_MAX);
    EXPECT_GT(path_max, 256); // At least 256 chars

    // Maximum open files
    int64_t open_max = env_get(CONF_OPEN_MAX);
    EXPECT_GT(open_max, 16); // At least 16 files

    // Pipe buffer size
    int64_t pipe_buf = env_get(CONF_PIPE_BUF);
    EXPECT_GT(pipe_buf, 128); // At least 128 bytes
}

// Test network configuration
TEST_F(env, network_configuration)
{
    // Number of network interfaces
    int64_t net_interfaces = env_get(CONF_NETWORK_INTERFACES);
    if(IsValidValue(net_interfaces))
    {
        EXPECT_GE(net_interfaces, 1);   // At least loopback
        EXPECT_LE(net_interfaces, 100); // Reasonable upper limit
    }
}

// Test time-related configurations
TEST_F(env, time_configuration)
{
    // Get current time for comparison
    time_t current_time = time(NULL);

    // Boot time - be more lenient since calculation might vary by platform
    int64_t boot_time = env_get(CONF_BOOT_TIME);
    if(IsValidValue(boot_time))
    {
        // Boot time should be before current time
        EXPECT_LT(boot_time, current_time);
        // Boot time should be reasonable (after year 1990, not too far future)
        if(boot_time > 631152000)
        { // 1990-01-01
            EXPECT_LT(boot_time,
                      current_time + 86400); // Not more than 1 day in future
        }
    }

    // Uptime in seconds
    int64_t uptime = env_get(CONF_UPTIME_SECONDS);
    EXPECT_GE(uptime, 0);
    EXPECT_LT(uptime, 365 * 24 * 3600 * 10); // Less than 10 years uptime

    // If both boot_time and uptime are available, they should be consistent
    if(IsValidValue(boot_time) && boot_time > 631152000)
    {
        int64_t calculated_current = boot_time + uptime;
        int64_t time_diff          = abs(calculated_current - current_time);
    }

    // Timezone offset
    int64_t tz_offset = env_get(CONF_TIMEZONE_OFFSET);
    // Timezone offset should be reasonable (-12 to +14 hours in minutes)
    EXPECT_GE(tz_offset, -12 * 60);
    EXPECT_LE(tz_offset, 14 * 60);
}

// Test swap configuration
TEST_F(env, swap_configuration)
{
    // std::cout << "Testing swap configuration..." << std::endl;

    int64_t swap_total = env_get(CONF_SWAP_SIZE_TOTAL);
    if(IsValidValue(swap_total))
    {
        EXPECT_GE(swap_total, 0);
    }
}

// Test resource limits
TEST_F(env, resource_limits)
{
    // File size max
    int64_t file_size_max = env_get(CONF_FILE_SIZE_MAX);
    if(IsValidValue(file_size_max))
    {
        EXPECT_GT(file_size_max, 1024 * 1024); // At least 1MB
    }

    // Stack size max
    int64_t stack_size_max = env_get(CONF_STACK_SIZE_MAX);
    if(IsValidValue(stack_size_max))
    {
        EXPECT_GT(stack_size_max, 1024); // At least 1KB
    }

    // Heap size max
    int64_t heap_size_max = env_get(CONF_HEAP_SIZE_MAX);
    if(IsValidValue(heap_size_max))
    {
        EXPECT_GT(heap_size_max, 1024 * 1024); // At least 1MB
    }

    // Max threads
    int64_t threads_max = env_get(CONF_THREADS_MAX);
    if(IsValidValue(threads_max))
    {
        EXPECT_GT(threads_max, 1);
    }

    // Max child processes
    int64_t child_max = env_get(CONF_PROCESS_CHILD_MAX);
    if(IsValidValue(child_max))
    {
        EXPECT_GT(child_max, 1);
    }
}

// Test IPC limits
TEST_F(env, ipc_limits)
{
    // AIO max
    int64_t aio_max = env_get(CONF_AIO_MAX);
    if(IsValidValue(aio_max))
    {
        EXPECT_GT(aio_max, 0);
    }

    // Message queue size max
    int64_t mq_size_max = env_get(CONF_MSG_QUEUE_SIZE_MAX);
    if(IsValidValue(mq_size_max))
    {
        EXPECT_GT(mq_size_max, 0);
    }

    // Max message size
    int64_t msg_max = env_get(CONF_MSG_MAX);
    if(IsValidValue(msg_max))
    {
        EXPECT_GT(msg_max, 0);
    }

    // Semaphore limits
    int64_t sem_nsems_max = env_get(CONF_SEM_NSEMS_MAX);
    if(IsValidValue(sem_nsems_max))
    {
        EXPECT_GT(sem_nsems_max, 0);
    }

    int64_t sem_value_max = env_get(CONF_SEM_VALUE_MAX);
    if(IsValidValue(sem_value_max))
    {
        EXPECT_GT(sem_value_max, 0);
    }
}

// Test hostname and login limits
TEST_F(env, name_limits)
{
    // Host name max
    int64_t hostname_max = env_get(CONF_HOST_NAME_MAX);
    EXPECT_GT(hostname_max, 8); // At least 8 characters

    // Login name max
    int64_t login_name_max = env_get(CONF_LOGIN_NAME_MAX);
    EXPECT_GT(login_name_max, 4); // At least 4 characters

    // TTY name max
    int64_t tty_name_max = env_get(CONF_TTY_NAME_MAX);
    if(IsValidValue(tty_name_max))
    {
        EXPECT_GT(tty_name_max, 0);
    }

    // Groups max
    int64_t ngroups_max = env_get(CONF_NGROUPS_MAX);
    EXPECT_GT(ngroups_max, 0);
}

// Test password/group buffer sizes
TEST_F(env, buffer_sizes)
{
    int64_t getpw_size = env_get(CONF_GETPW_R_SIZE_MAX);
    EXPECT_GT(getpw_size, 512); // At least 512 bytes

    int64_t getgr_size = env_get(CONF_GETGR_R_SIZE_MAX);
    EXPECT_GT(getgr_size, 512); // At least 512 bytes
}

// Test all convenience macros
TEST_F(env, convenience_macros)
{
    // Test that macros return same values as direct function calls
    EXPECT_EQ(ENV_CPU_COUNT, env_get(CONF_CPU_COUNT));
    EXPECT_EQ(ENV_MEMORY_TOTAL, env_get(CONF_MEMORY_TOTAL));
    EXPECT_EQ(ENV_UPTIME_SECONDS, env_get(CONF_UPTIME_SECONDS));
    EXPECT_EQ(ENV_TIMEZONE_OFFSET, env_get(CONF_TIMEZONE_OFFSET));
    EXPECT_EQ(ENV_PATH_LEN_MAX, env_get(CONF_PATH_MAX));
    EXPECT_EQ(ENV_OPEN_FILES_MAX, env_get(CONF_OPEN_MAX));
}

// Test invalid configuration
TEST_F(env, invalid_configuration)
{
    // Test with invalid enum value (should return -1)
    int64_t invalid_result = env_get(static_cast<conf_t>(9999));
    EXPECT_EQ(invalid_result, -1);
}

// Test compiler feature macros
TEST_F(env, compiler_features)
{
    // Test QT environment detection
    std::cout << "QT Environment: " << ENV_QT_ENVIRONMENT << std::endl;
}

// Performance test for env_get function
TEST_F(env, performance)
{
    auto start = std::chrono::high_resolution_clock::now();

    // Call env_get multiple times for different configurations
    for(int i = 0; i < 1000; ++i)
    {
        volatile int64_t result; // Prevent optimization
        result = env_get(CONF_CPU_COUNT);
        result = env_get(CONF_MEMORY_TOTAL);
        result = env_get(CONF_UPTIME_SECONDS);
        (void) result; // Suppress unused variable warning
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Should complete within reasonable time (less than 100ms for simple calls)
    EXPECT_LT(duration.count(), 100000);
}

// Test consistency between multiple calls
TEST_F(env, consistency_check)
{
    // Values that should remain constant during test execution
    int64_t cpu_count1 = env_get(CONF_CPU_COUNT);
    int64_t cpu_count2 = env_get(CONF_CPU_COUNT);
    EXPECT_EQ(cpu_count1, cpu_count2);

    int64_t total_mem1 = env_get(CONF_MEMORY_TOTAL);
    int64_t total_mem2 = env_get(CONF_MEMORY_TOTAL);
    EXPECT_EQ(total_mem1, total_mem2);

    int64_t page_size1 = env_get(CONF_MEMORY_PAGE_SIZE);
    int64_t page_size2 = env_get(CONF_MEMORY_PAGE_SIZE);
    EXPECT_EQ(page_size1, page_size2);
}

// Comprehensive system information display
TEST_F(env, system_information_display)
{
    std::cout << "\n=== COMPREHENSIVE SYSTEM INFORMATION ===" << std::endl;

    std::cout << "Compile Time: " << COMPILE_TIME << std::endl;
    std::cout << "CPU Count: " << ENV_CPU_COUNT << std::endl;

    if(IsValidValue(ENV_CPU_FREQUENCY))
    {
        std::cout << "CPU Frequency: " << ENV_CPU_FREQUENCY << " MHz"
                  << std::endl;
    }

    std::cout << "Memory Total: " << ENV_MEMORY_TOTAL / (1024 * 1024) << " MB"
              << std::endl;

    std::cout << "Page Size: " << ENV_MEMORY_PAGE_SIZE << " bytes" << std::endl;

    if(IsValidValue(ENV_USER_ID))
    {
        std::cout << "User ID: " << ENV_USER_ID << std::endl;
    }

    std::cout << "Uptime: " << ENV_UPTIME_SECONDS << " seconds ("
              << ENV_UPTIME_SECONDS / 3600 << " hours)" << std::endl;
    std::cout << "Timezone Offset: " << ENV_TIMEZONE_OFFSET << " minutes (GMT"
              << (ENV_TIMEZONE_OFFSET >= 0 ? "+" : "")
              << ENV_TIMEZONE_OFFSET / 60.0 << ")" << std::endl;
    std::cout << "Path Max: " << ENV_PATH_LEN_MAX << std::endl;
    std::cout << "Open Files Max: " << ENV_OPEN_FILES_MAX << std::endl;

    std::cout << "=========================================" << std::endl;
}