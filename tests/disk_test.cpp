#include <gtest/gtest.h>
#include <iostream>
#include <hj/hardware/disk.h>
#include <string>
#include <vector>

class DiskTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        disk_err_t result = disk_init();
        ASSERT_EQ(result, DISK_SUCCESS);
    }

    void TearDown() override
    {
    }
};

// Test disk subsystem initialization and cleanup
TEST_F(DiskTest, initialization_and_cleanup)
{
    SCOPED_TRACE("Testing disk subsystem initialization and cleanup");

    // std::cout << "Testing disk subsystem management..." << std::endl;

    // Test multiple initializations (should be safe)
    EXPECT_EQ(disk_init(), DISK_SUCCESS)
        << "Multiple initialization should be safe";

    // Re-initialize for other tests
    EXPECT_EQ(disk_init(), DISK_SUCCESS) << "Re-initialization should work";

    // std::cout << "Disk subsystem management test completed" << std::endl;
}

// Test size formatting function
TEST_F(DiskTest, size_formatting)
{
    SCOPED_TRACE("Testing size formatting");

    // std::cout << "Testing size formatting..." << std::endl;

    char buffer[256];

    // Test various sizes
    const uint64_t test_sizes[] = {0,
                                   512,
                                   1024,
                                   1024 * 1024,
                                   1024ULL * 1024 * 1024,
                                   1024ULL * 1024 * 1024 * 1024,
                                   1024ULL * 1024 * 1024 * 1024 * 1024};

    for (size_t i = 0; i < sizeof(test_sizes) / sizeof(test_sizes[0]); i++)
    {
        disk_err_t result =
            disk_format_size(test_sizes[i], buffer, sizeof(buffer));
        EXPECT_EQ(result, DISK_SUCCESS)
            << "Size formatting should succeed for " << test_sizes[i];
        EXPECT_GT(strlen(buffer), 0) << "Formatted size should not be empty";

        // std::cout << "Size " << test_sizes[i] << " bytes: " << buffer
        //           << std::endl;
    }

    // Test error conditions
    EXPECT_EQ(disk_format_size(1024, nullptr, 0),
    DISK_ERROR_INVALID_PARAMETER);

    // Test insufficient buffer
    char small_buffer[5];
    disk_err_t result = disk_format_size(1024ULL * 1024 * 1024,
    small_buffer,
                                           sizeof(small_buffer));
    EXPECT_EQ(result, DISK_ERROR_INSUFFICIENT_BUFFER)
        << "Should detect insufficient buffer";

    // std::cout << "Size formatting test completed" << std::endl;
}

// Test disk count retrieval
TEST_F(DiskTest, disk_count_retrieval)
{
    SCOPED_TRACE("Testing disk count retrieval");

    // std::cout << "Testing disk count retrieval..." << std::endl;

    uint32_t count = disk_count();

    // There should be at least one disk on any system
    EXPECT_GT(count, 0) << "Should find at least one disk";
    EXPECT_LE(count, DISK_MAX_DISKS) << "Count should not exceed maximum";
}

// Test disk enumeration
TEST_F(DiskTest, disk_enumeration)
{
    SCOPED_TRACE("Testing disk enumeration");

    // std::cout << "Testing disk enumeration..." << std::endl;

    uint32_t count = disk_count();
    ASSERT_GT(count, 0) << "Should get disk count successfully";
    if (count == 0)
    {
        // std::cout << "No disks found, skipping enumeration test" << std::endl; 
        return;
    }

    // Allocate array for disk information
    std::vector<disk_info_t> disks(count);
    uint32_t actual_count = 0;

    auto result = disk_enumerate(disks.data(), count, &actual_count);
    EXPECT_EQ(result, DISK_SUCCESS) << "Disk enumeration should succeed";
    EXPECT_LE(actual_count, count)
        << "Actual count should not exceed requested count";

    // std::cout << "Successfully enumerated " << actual_count
    //           << " disks:" << std::endl;

    for (uint32_t i = 0; i < actual_count; i++)
    {
        const disk_info_t& disk = disks[i];

        // Validate disk information
        EXPECT_GT(strlen(disk.device_name), 0)
            << "Device name should not be empty for disk " << i;
        EXPECT_GE(disk.type, DISK_TYPE_UNKNOWN)
            << "Disk type should be valid for disk " << i;
        EXPECT_LE(disk.type, DISK_TYPE_RAM)
            << "Disk type should be valid for disk " << i;

        // std::cout << "  Disk " << i << ":" << std::endl;
        // std::cout << "    Device: " << disk.device_name << std::endl;
        // std::cout << "    Model: " << disk.model << std::endl;

        // if (disk.total_size > 0)
        // {
        //     char size_str[64];
        //     if (disk_format_size(disk.total_size, size_str, sizeof(size_str)) == DISK_SUCCESS)
        //     {
        //         std::cout << "    Size: " << size_str << std::endl;
        //     }
        // }

        // std::cout << "    Sector size: " << disk.sector_size << " bytes"
        //           << std::endl;
        // std::cout << "    Removable: " << (disk.removable ? "Yes" : "No")
        //           << std::endl;
        // std::cout << "    Read-only: " << (disk.read_only ? "Yes" : "No")
        //           << std::endl;

        // if (disk.rpm > 0)
        // {
        //     std::cout << "    RPM: " << disk.rpm << std::endl;
        // }

        // if (disk.temperature >= 0)
        // {
        //     std::cout << "    Temperature: " << disk.temperature << "°C"
        //               << std::endl;
        // }
    }

    // Test invalid parameters
    EXPECT_EQ(disk_enumerate(nullptr, count, &actual_count),
              DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(disk_enumerate(disks.data(), count, nullptr),
              DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(disk_enumerate(disks.data(), 0, &actual_count),
              DISK_ERROR_INVALID_PARAMETER);

    // std::cout << "Disk enumeration test completed" << std::endl;
}

// Test disk information retrieval
TEST_F(DiskTest, disk_info_retrieval)
{
    SCOPED_TRACE("Testing disk information retrieval");

    // std::cout << "Testing disk information retrieval..." << std::endl;

    // First get a list of available disks
    uint32_t count = disk_count();

    if (count == 0)
    {
        // std::cout << "No disks available for testing" << std::endl;
        GTEST_SKIP() << "No disks available";
    }

    std::vector<disk_info_t> disks(count);
    uint32_t actual_count = 0;
    auto result = disk_enumerate(disks.data(), count, &actual_count);
    ASSERT_EQ(result, DISK_SUCCESS) << "Should enumerate disks successfully";

    if (actual_count > 0)
    {
        // Test getting info for the first disk
        disk_info_t info;
        result = disk_info(disks[0].device_name, &info);

        if (result == DISK_SUCCESS)
        {
            // std::cout << "Successfully retrieved info for disk: "
            //           << disks[0].device_name << std::endl;

            // Validate the information
            EXPECT_STREQ(info.device_name, disks[0].device_name)
                << "Device name should match";
            EXPECT_GE(info.type, DISK_TYPE_UNKNOWN)
                << "Disk type should be valid";
            EXPECT_LE(info.type, DISK_TYPE_RAM) << "Disk type should be valid";

            if (info.sector_size > 0)
            {
                EXPECT_GE(info.sector_size, 512)
                    << "Sector size should be at least 512 bytes";
                EXPECT_LE(info.sector_size, 4096)
                    << "Sector size should be reasonable";
            }
        }
        else
        {
            std::cout << "Could not retrieve detailed info for disk (may "
                         "require elevated privileges)"
                      << std::endl;
        }
    }

    // Test invalid parameters
    disk_info_t dummy_info;
    EXPECT_EQ(disk_info(nullptr, &dummy_info),
              DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(disk_info("valid_name", nullptr),
              DISK_ERROR_INVALID_PARAMETER);

    // Test non-existent device
    result = disk_info("non_existent_device", &dummy_info);
    // EXPECT_NE(result, DISK_SUCCESS) << "Should fail for non-existent device";

    // std::cout << "Disk information retrieval test completed" << std::endl;
}

// Test disk readiness check
TEST_F(DiskTest, disk_readiness_check)
{
    SCOPED_TRACE("Testing disk readiness check");

    // std::cout << "Testing disk readiness check..." << std::endl;

    // Get available disks
    uint32_t count = disk_count();
    if (count == 0)
    {
        GTEST_SKIP() << "No disks available for readiness testing";
    }

    std::vector<disk_info_t> disks(count);
    uint32_t actual_count = 0;
    auto result = disk_enumerate(disks.data(), count, &actual_count);
    ASSERT_EQ(result, DISK_SUCCESS) << "Should enumerate disks successfully";

    if (actual_count > 0)
    {
        bool ready = disk_is_ready(disks[0].device_name);
        // std::cout << "Disk " << disks[0].device_name
        //               << " ready status: " << (ready ? "Ready" : "Not ready")
        //               << std::endl;

        // Most system disks should be ready
        if (!disks[0].removable)
        {
            EXPECT_TRUE(ready)
                << "Non-removable disk should typically be ready";
        }
    }
}

// Test partition information retrieval
TEST_F(DiskTest, partition_info_retrieval)
{
    SCOPED_TRACE("Testing partition information retrieval");

    // std::cout << "Testing partition information retrieval..." << std::endl;

    partition_info_t info;
    disk_err_t result;

    // Test common mount points based on platform
#if defined(DISK_PLATFORM_WINDOWS)
    result = disk_get_partition_by_mount("C:\\", &info);
#elif defined(DISK_PLATFORM_LINUX) || defined(DISK_PLATFORM_MACOS)
    result = disk_get_partition_by_mount("/", &info);
#else
    result = DISK_ERROR_NOT_SUPPORTED;
#endif

    if (result == DISK_SUCCESS)
    {
        // std::cout << "Successfully retrieved partition information:"
        //           << std::endl;
        // std::cout << "  Mount point: " << info.mount_point << std::endl;
        // std::cout << "  Device: " << info.device_name << std::endl;
        // std::cout << "  Volume label: " << info.volume_label << std::endl;

        if (info.total_size > 0)
        {
            char total_str[64], used_str[64], avail_str[64];

            if (disk_format_size(info.total_size, total_str,
                                 sizeof(total_str)) == DISK_SUCCESS &&
                disk_format_size(info.used_size, used_str, sizeof(used_str))
                ==
                    DISK_SUCCESS &&
                disk_format_size(info.available_size, avail_str,
                                 sizeof(avail_str)) == DISK_SUCCESS)
            {

                // std::cout << "  Total size: " << total_str << std::endl;
                // std::cout << "  Used size: " << used_str << std::endl;
                // std::cout << "  Available size: " << avail_str << std::endl;

                // Basic sanity checks
                EXPECT_GT(info.total_size, 0)
                    << "Total size should be positive";
                EXPECT_LE(info.used_size, info.total_size)
                    << "Used size should not exceed total";
                EXPECT_LE(info.available_size, info.total_size)
                    << "Available size should not exceed total";
            }
        }

        // std::cout << "  Read-only: " << (info.read_only ? "Yes" : "No")
        //           << std::endl;
        // std::cout << "  Bootable: " << (info.bootable ? "Yes" : "No")
        //           << std::endl;
    }
    else if (result == DISK_ERROR_NOT_SUPPORTED)
    {
        // std::cout << "Partition information not supported on this platform"
        //           << std::endl;
        GTEST_SKIP() << "Platform does not support partition information";
    }
    else
    {
        GTEST_SKIP() << "Could not retrieve partition information";
    }

    // Test invalid parameters
    partition_info_t dummy_info;
    EXPECT_EQ(disk_get_partition_by_mount(nullptr, &dummy_info),
              DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(disk_get_partition_by_mount("/", nullptr),
              DISK_ERROR_INVALID_PARAMETER);

    // std::cout << "Partition information retrieval test completed" << std::endl;
}

// Test disk performance testing
TEST_F(DiskTest, disk_performance_testing)
{
    SCOPED_TRACE("Testing disk performance testing");

    // std::cout << "Testing disk performance testing..." << std::endl;

    // Get available disks
    uint32_t count = disk_count();
    if (count == 0)
    {
        GTEST_SKIP() << "No disks available for performance testing";
    }

    std::vector<disk_info_t> disks(count);
    uint32_t actual_count = 0;
    auto result = disk_enumerate(disks.data(), count, &actual_count);
    ASSERT_EQ(result, DISK_SUCCESS) << "Should enumerate disks successfully";
    if (actual_count > 0)
    {
        double read_speed = 0.0;
        result = disk_read_speed_test(disks[0].device_name, 1,
                                      &read_speed); // 1 MB test

        if (result == DISK_SUCCESS)
        {
            // std::cout << "Read speed test for " << disks[0].device_name 
            //           << ": " << read_speed << " MB/s" << std::endl;

            EXPECT_GT(read_speed, 0.0) << "Read speed should be positive";
            EXPECT_LT(read_speed, 10000.0) << "Read speed should be reasonable";
        }
        else
        {
            std::cout << "Read speed test not available (may require elevated privileges)"
                      << std::endl;
        }
    }

    // Test invalid parameters
    double dummy_speed;
    EXPECT_EQ(disk_read_speed_test(nullptr, 1, &dummy_speed),
              DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(disk_read_speed_test("valid_name", 0, &dummy_speed),
              DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(disk_read_speed_test("valid_name", 1, nullptr),
              DISK_ERROR_INVALID_PARAMETER);

    // Test invalid parameters
    EXPECT_EQ(disk_read_speed_test(nullptr, 1, &dummy_speed),
              DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(disk_read_speed_test("valid_name", 0, &dummy_speed),
              DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(disk_read_speed_test("valid_name", 1, nullptr),
              DISK_ERROR_INVALID_PARAMETER);

    // std::cout << "Disk performance testing test completed" << std::endl;
}

// Test parameter validation across all functions
TEST_F(DiskTest, parameter_validation)
{
    SCOPED_TRACE("Testing parameter validation");

    uint32_t dummy_uint32;
    bool dummy_bool;
    double dummy_double;
    char dummy_buffer[256];
    disk_info_t dummy_disk_info;
    partition_info_t dummy_partition_info;

    // Test all functions with NULL pointers where applicable
    dummy_uint32 = disk_count();
    EXPECT_EQ(disk_enumerate(nullptr, 1, &dummy_uint32),
              DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(disk_info(nullptr, &dummy_disk_info),
              DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(disk_info("test", nullptr), DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(disk_get_partition_by_mount(nullptr, &dummy_partition_info),
              DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(disk_get_partition_by_mount("/", nullptr),
              DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(disk_is_ready(nullptr), false);
    EXPECT_EQ(disk_is_ready("test"), false);
    EXPECT_EQ(disk_read_speed_test(nullptr, 1, &dummy_double),
              DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(disk_read_speed_test("test", 0, &dummy_double),
              DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(disk_read_speed_test("test", 1, nullptr),
              DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(disk_format_size(1024, nullptr, 256),
              DISK_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(disk_format_size(1024, dummy_buffer, 0),
              DISK_ERROR_INVALID_PARAMETER);

    // std::cout << "Parameter validation test completed" << std::endl;
}

// Integration test - comprehensive disk system test
TEST_F(DiskTest, comprehensive_disk_system_test)
{
    SCOPED_TRACE("Comprehensive disk system test");

    // std::cout << "Running comprehensive disk system test..." << std::endl;

    // Step 1: Get disk count
    uint32_t count = disk_count();

    if (count == DISK_ERROR_NOT_SUPPORTED)
    {
        GTEST_SKIP() << "Disk operations not supported on this platform";
    }

    if (count == 0)
    {
        std::cout << "No disks found, ending comprehensive test" << std::endl; 
        return;
    }

    // Step 2: Enumerate all disks
    std::vector<disk_info_t> disks(count);
    uint32_t actual_count = 0;
    auto result = disk_enumerate(disks.data(), count, &actual_count);
    ASSERT_EQ(result, DISK_SUCCESS) << "Should enumerate disks";
    // std::cout << "Successfully enumerated " << actual_count << " disks"
    //           << std::endl;

    // Step 3: Test each disk
    for (uint32_t i = 0; i < actual_count; i++)
    {
        const disk_info_t& disk = disks[i];
        // std::cout << "\nTesting disk " << i << ": " << disk.device_name
        //           << std::endl;

        // Test readiness
        bool ready = disk_is_ready(disk.device_name);
        if (result == DISK_SUCCESS)
        {
            // std::cout << "  Readiness: " << (ready ? "Ready" : "Not ready")
            //           << std::endl;
        }

        // Test getting detailed info
        disk_info_t detailed_info;
        result = disk_info(disk.device_name, &detailed_info);
        if (result == DISK_SUCCESS)
        {
            // std::cout << "  Detailed info retrieved successfully" << std::endl; 
            EXPECT_STREQ(detailed_info.device_name, disk.device_name);
        }

        // Test performance (only for ready, non-removable disks)
        if (ready && !disk.removable)
        {
            double read_speed = 0.0;
            result = disk_read_speed_test(disk.device_name, 1, &read_speed);
            if (result == DISK_SUCCESS)
            {
                // std::cout << "  Read speed: " << read_speed << " MB/s"
                //           << std::endl;
                EXPECT_GT(read_speed, 0.0);
            }
        }
    }

    // std::cout << "\nComprehensive disk system test completed successfully"
    //           << std::endl;
}
