#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <vector>

#define HJ_DISK_IMPL
#include <hj/hardware/disk.h>

class disk : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        hj_disk_err_t result = hj_disk_init();
        ASSERT_EQ(result, HJ_DISK_OK);
    }

    void TearDown() override {}
};

// Test disk subsystem initialization and cleanup
TEST_F(disk, initialization_and_cleanup)
{
    SCOPED_TRACE("Testing disk subsystem initialization and cleanup");

    // Test multiple initializations (should be safe)
    EXPECT_EQ(hj_disk_init(), HJ_DISK_OK)
        << "Multiple initialization should be safe";

    // Re-initialize for other tests
    EXPECT_EQ(hj_disk_init(), HJ_DISK_OK) << "Re-initialization should work";

    // std::cout << "Disk subsystem management test completed" << std::endl;
}

// Test size formatting function
TEST_F(disk, size_formatting)
{
    SCOPED_TRACE("Testing size formatting");
    char buffer[256];

    // Test various sizes
    const uint64_t test_sizes[] = {0,
                                   512,
                                   1024,
                                   1024 * 1024,
                                   1024ULL * 1024 * 1024,
                                   1024ULL * 1024 * 1024 * 1024,
                                   1024ULL * 1024 * 1024 * 1024 * 1024};

    for(size_t i = 0; i < sizeof(test_sizes) / sizeof(test_sizes[0]); i++)
    {
        hj_disk_err_t result =
            hj_disk_format_size(test_sizes[i], buffer, sizeof(buffer));
        EXPECT_EQ(result, HJ_DISK_OK)
            << "Size formatting should succeed for " << test_sizes[i];
        EXPECT_GT(strlen(buffer), 0) << "Formatted size should not be empty";
    }

    // Test error conditions
    EXPECT_EQ(hj_disk_format_size(1024, nullptr, 0),
              HJ_DISK_ERROR_INVALID_PARAMETER);

    // Test insufficient buffer
    char          small_buffer[5];
    hj_disk_err_t result = hj_disk_format_size(1024ULL * 1024 * 1024,
                                               small_buffer,
                                               sizeof(small_buffer));
    EXPECT_EQ(result, HJ_DISK_ERROR_BUFFER_TOO_SMALL)
        << "Should detect insufficient buffer";

    // std::cout << "Size formatting test completed" << std::endl;
}

// Test disk count retrieval
TEST_F(disk, disk_count_retrieval)
{
    SCOPED_TRACE("Testing disk count retrieval");

    uint32_t count = hj_disk_count();

    // There should be at least one disk on any system
    EXPECT_GT(count, 0) << "Should find at least one disk";
    EXPECT_LE(count, HJ_DISK_MAX_DISKS) << "Count should not exceed maximum";
}

// Test disk enumeration
TEST_F(disk, disk_enumeration)
{
    SCOPED_TRACE("Testing disk enumeration");

    uint32_t count = hj_disk_count();
    ASSERT_GT(count, 0) << "Should get disk count successfully";
    if(count == 0)
    {
        return;
    }

    // Allocate array for disk information
    std::vector<hj_disk_info_t> disks(count);
    uint32_t                    actual_count = 0;

    auto result = hj_disk_enumerate(disks.data(), count, &actual_count);
    EXPECT_EQ(result, HJ_DISK_OK) << "Disk enumeration should succeed";
    EXPECT_LE(actual_count, count)
        << "Actual count should not exceed requested count";

    for(uint32_t i = 0; i < actual_count; i++)
    {
        const hj_disk_info_t &disk = disks[i];

        // Validate disk information
        EXPECT_GT(strlen(disk.device_name), 0)
            << "Device name should not be empty for disk " << i;
        EXPECT_GE(disk.type, HJ_DISK_TYPE_UNKNOWN)
            << "Disk type should be valid for disk " << i;
        EXPECT_LE(disk.type, HJ_DISK_TYPE_RAM)
            << "Disk type should be valid for disk " << i;
    }

    // Test invalid parameters
    EXPECT_EQ(hj_disk_enumerate(nullptr, count, &actual_count),
              HJ_DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(hj_disk_enumerate(disks.data(), count, nullptr),
              HJ_DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(hj_disk_enumerate(disks.data(), 0, &actual_count),
              HJ_DISK_ERROR_INVALID_PARAMETER);
}

// Test disk information retrieval
TEST_F(disk, disk_info_retrieval)
{
    SCOPED_TRACE("Testing disk information retrieval");

    // First get a list of available disks
    uint32_t count = hj_disk_count();

    if(count == 0)
    {
        GTEST_SKIP() << "No disks available";
    }

    std::vector<hj_disk_info_t> disks(count);
    uint32_t                    actual_count = 0;
    auto result = hj_disk_enumerate(disks.data(), count, &actual_count);
    ASSERT_EQ(result, HJ_DISK_OK) << "Should enumerate disks successfully";

    if(actual_count > 0)
    {
        // Test getting info for the first disk
        hj_disk_info_t info;
        result = hj_disk_info(disks[0].device_name, &info);

        if(result == HJ_DISK_OK)
        {
            // Validate the information
            EXPECT_STREQ(info.device_name, disks[0].device_name)
                << "Device name should match";
            EXPECT_GE(info.type, HJ_DISK_TYPE_UNKNOWN)
                << "Disk type should be valid";
            EXPECT_LE(info.type, HJ_DISK_TYPE_RAM)
                << "Disk type should be valid";

            if(info.sector_size > 0)
            {
                EXPECT_GE(info.sector_size, 512)
                    << "Sector size should be at least 512 bytes";
                EXPECT_LE(info.sector_size, 4096)
                    << "Sector size should be reasonable";
            }
        } else
        {
            std::cout << "Could not retrieve detailed info for disk (may "
                         "require elevated privileges)"
                      << std::endl;
        }
    }

    // Test invalid parameters
    hj_disk_info_t dummy_info;
    EXPECT_EQ(hj_disk_info(nullptr, &dummy_info),
              HJ_DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(hj_disk_info("valid_name", nullptr),
              HJ_DISK_ERROR_INVALID_PARAMETER);

    // Test non-existent device
    result = hj_disk_info("non_existent_device", &dummy_info);
}

// Test disk readiness check
TEST_F(disk, disk_readiness_check)
{
    SCOPED_TRACE("Testing disk readiness check");

    // Get available disks
    uint32_t count = hj_disk_count();
    if(count == 0)
    {
        GTEST_SKIP() << "No disks available for readiness testing";
    }

    std::vector<hj_disk_info_t> disks(count);
    uint32_t                    actual_count = 0;
    auto result = hj_disk_enumerate(disks.data(), count, &actual_count);
    ASSERT_EQ(result, HJ_DISK_OK) << "Should enumerate disks successfully";

    if(actual_count > 0)
    {
        bool ready = hj_disk_is_ready(disks[0].device_name);

        // Most system disks should be ready
        if(!disks[0].removable)
        {
            EXPECT_TRUE(ready)
                << "Non-removable disk should typically be ready";
        }
    }
}

// Test partition information retrieval
TEST_F(disk, partition_info_retrieval)
{
    SCOPED_TRACE("Testing partition information retrieval");

    hj_partition_info_t info;
    hj_disk_err_t       result;

    // Test common mount points based on platform
#if defined(HJ_DISK_PLATFORM_WINDOWS)
    result = hj_disk_get_partition_by_mount("C:\\", &info);
#elif defined(HJ_DISK_PLATFORM_LINUX) || defined(HJ_DISK_PLATFORM_MACOS)
    result = hj_disk_get_partition_by_mount("/", &info);
#else
    result = HJ_DISK_ERROR_NOT_SUPPORTED;
#endif

    if(result == HJ_DISK_OK)
    {
        if(info.total_size > 0)
        {
            char total_str[64], used_str[64], avail_str[64];

            if(hj_disk_format_size(info.total_size,
                                   total_str,
                                   sizeof(total_str))
                   == HJ_DISK_OK
               && hj_disk_format_size(info.used_size,
                                      used_str,
                                      sizeof(used_str))
                      == HJ_DISK_OK
               && hj_disk_format_size(info.available_size,
                                      avail_str,
                                      sizeof(avail_str))
                      == HJ_DISK_OK)
            {
                // Basic sanity checks
                EXPECT_GT(info.total_size, 0)
                    << "Total size should be positive";
                EXPECT_LE(info.used_size, info.total_size)
                    << "Used size should not exceed total";
                EXPECT_LE(info.available_size, info.total_size)
                    << "Available size should not exceed total";
            }
        }

    } else if(result == HJ_DISK_ERROR_NOT_SUPPORTED)
    {
        GTEST_SKIP() << "Platform does not support partition information";
    } else
    {
        GTEST_SKIP() << "Could not retrieve partition information";
    }

    // Test invalid parameters
    hj_partition_info_t dummy_info;
    EXPECT_EQ(hj_disk_get_partition_by_mount(nullptr, &dummy_info),
              HJ_DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(hj_disk_get_partition_by_mount("/", nullptr),
              HJ_DISK_ERROR_INVALID_PARAMETER);
}

// Test disk performance testing
TEST_F(disk, disk_performance_testing)
{
    SCOPED_TRACE("Testing disk performance testing");

    // Get available disks
    uint32_t count = hj_disk_count();
    if(count == 0)
    {
        GTEST_SKIP() << "No disks available for performance testing";
    }

    std::vector<hj_disk_info_t> disks(count);
    uint32_t                    actual_count = 0;
    auto result = hj_disk_enumerate(disks.data(), count, &actual_count);
    ASSERT_EQ(result, HJ_DISK_OK) << "Should enumerate disks successfully";
    if(actual_count > 0)
    {
        double read_speed = 0.0;
        result            = hj_disk_read_speed_test(disks[0].device_name,
                                                    1,
                                                    &read_speed); // 1 MB test

        if(result == HJ_DISK_OK)
        {
            EXPECT_GT(read_speed, 0.0) << "Read speed should be positive";
            EXPECT_LT(read_speed, 10000.0) << "Read speed should be reasonable";
        } else
        {
            std::cout << "Read speed test not available (may require elevated "
                         "privileges)"
                      << std::endl;
        }
    }

    // Test invalid parameters
    double dummy_speed;
    EXPECT_EQ(hj_disk_read_speed_test(nullptr, 1, &dummy_speed),
              HJ_DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(hj_disk_read_speed_test("valid_name", 0, &dummy_speed),
              HJ_DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(hj_disk_read_speed_test("valid_name", 1, nullptr),
              HJ_DISK_ERROR_INVALID_PARAMETER);

    // Test invalid parameters
    EXPECT_EQ(hj_disk_read_speed_test(nullptr, 1, &dummy_speed),
              HJ_DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(hj_disk_read_speed_test("valid_name", 0, &dummy_speed),
              HJ_DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(hj_disk_read_speed_test("valid_name", 1, nullptr),
              HJ_DISK_ERROR_INVALID_PARAMETER);
}

// Test parameter validation across all functions
TEST_F(disk, parameter_validation)
{
    SCOPED_TRACE("Testing parameter validation");

    uint32_t            dummy_uint32;
    bool                dummy_bool;
    double              dummy_double;
    char                dummy_buffer[256];
    hj_disk_info_t      dummy_disk_info;
    hj_partition_info_t dummy_partition_info;

    // Test all functions with NULL pointers where applicable
    dummy_uint32 = hj_disk_count();
    EXPECT_EQ(hj_disk_enumerate(nullptr, 1, &dummy_uint32),
              HJ_DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(hj_disk_info(nullptr, &dummy_disk_info),
              HJ_DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(hj_disk_info("test", nullptr), HJ_DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(hj_disk_get_partition_by_mount(nullptr, &dummy_partition_info),
              HJ_DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(hj_disk_get_partition_by_mount("/", nullptr),
              HJ_DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(hj_disk_is_ready(nullptr), false);
    EXPECT_EQ(hj_disk_is_ready("test"), false);
    EXPECT_EQ(hj_disk_read_speed_test(nullptr, 1, &dummy_double),
              HJ_DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(hj_disk_read_speed_test("test", 0, &dummy_double),
              HJ_DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(hj_disk_read_speed_test("test", 1, nullptr),
              HJ_DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(hj_disk_format_size(1024, nullptr, 256),
              HJ_DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(hj_disk_format_size(1024, dummy_buffer, 0),
              HJ_DISK_ERROR_INVALID_PARAMETER);
}

// Integration test - comprehensive disk system test
TEST_F(disk, comprehensive_disk_system_test)
{
    SCOPED_TRACE("Comprehensive disk system test");

    // Step 1: Get disk count
    uint32_t count = hj_disk_count();

    if(count == HJ_DISK_ERROR_NOT_SUPPORTED)
    {
        GTEST_SKIP() << "Disk operations not supported on this platform";
    }

    if(count == 0)
    {
        std::cout << "No disks found, ending comprehensive test" << std::endl;
        return;
    }

    // Step 2: Enumerate all disks
    std::vector<hj_disk_info_t> disks(count);
    uint32_t                    actual_count = 0;
    auto result = hj_disk_enumerate(disks.data(), count, &actual_count);
    ASSERT_EQ(result, HJ_DISK_OK) << "Should enumerate disks";

    // Step 3: Test each disk
    for(uint32_t i = 0; i < actual_count; i++)
    {
        const hj_disk_info_t &disk = disks[i];

        // Test readiness
        bool ready = hj_disk_is_ready(disk.device_name);
        if(result == HJ_DISK_OK)
        {
        }

        // Test getting detailed info
        hj_disk_info_t detailed_info;
        result = hj_disk_info(disk.device_name, &detailed_info);
        if(result == HJ_DISK_OK)
        {
            EXPECT_STREQ(detailed_info.device_name, disk.device_name);
        }

        // Test performance (only for ready, non-removable disks)
        if(ready && !disk.removable)
        {
            double read_speed = 0.0;
            result = hj_disk_read_speed_test(disk.device_name, 1, &read_speed);
            if(result == HJ_DISK_OK)
            {
                EXPECT_GT(read_speed, 0.0);
            }
        }
    }
}
