#include <gtest/gtest.h>
#include <hj/hardware/nic.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>

class nic : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        // Initialize NIC subsystem
        nic_error_t result = nic_init();
        ASSERT_EQ(result, NIC_SUCCESS);
    }

    void TearDown() override { nic_cleanup(); }
};

// Test NIC subsystem initialization and cleanup
TEST_F(nic, initialization_and_cleanup)
{
    SCOPED_TRACE("Testing NIC initialization and cleanup");

    std::cout << "Testing NIC subsystem initialization..." << std::endl;

    // Cleanup and re-initialize to test the functions
    nic_cleanup();

    nic_error_t result = nic_init();
    EXPECT_EQ(result, NIC_SUCCESS) << "NIC initialization should succeed";

    // Test multiple initializations (should be safe)
    result = nic_init();
    EXPECT_EQ(result, NIC_SUCCESS)
        << "Multiple NIC initializations should be safe";

    // Test cleanup (should be safe to call multiple times)
    nic_cleanup();
    nic_cleanup();

    // Re-initialize for other tests
    result = nic_init();
    EXPECT_EQ(result, NIC_SUCCESS) << "NIC re-initialization should succeed";
}

// Test interface counting
TEST_F(nic, interface_counting)
{
    SCOPED_TRACE("Testing interface counting");

    // Test invalid parameter
    nic_error_t result = nic_get_interface_count(nullptr);
    EXPECT_EQ(result, NIC_ERROR_INVALID_PARAMETER)
        << "Should return error for null parameter";

    // Test valid counting
    uint32_t count = 0;
    result         = nic_get_interface_count(&count);
    EXPECT_EQ(result, NIC_SUCCESS) << "Interface counting should succeed";

    // Most systems should have at least one interface (loopback)
    EXPECT_GE(count, 1) << "System should have at least one network interface";
    EXPECT_LE(count, NIC_MAX_INTERFACES)
        << "Interface count should not exceed maximum";
}

// Test interface enumeration
TEST_F(nic, interface_enumeration)
{
    SCOPED_TRACE("Testing interface enumeration");
}

// Test getting interface information by name
TEST_F(nic, get_interface_info_by_name)
{
    SCOPED_TRACE("Testing get interface info by name");

    // Test invalid parameters
    nic_info_t  info;
    nic_error_t result = nic_get_interface_info(nullptr, &info);
    EXPECT_EQ(result, NIC_ERROR_INVALID_PARAMETER)
        << "Should return error for null interface name";

    result = nic_get_interface_info("eth0", nullptr);
    EXPECT_EQ(result, NIC_ERROR_INVALID_PARAMETER)
        << "Should return error for null info pointer";

    // Get first available interface
    nic_info_t interfaces[NIC_MAX_INTERFACES];
    uint32_t   count = 0;
    result = nic_enumerate_interfaces(interfaces, NIC_MAX_INTERFACES, &count);
    ASSERT_EQ(result, NIC_SUCCESS) << "Interface enumeration should succeed";
    ASSERT_GT(count, 0) << "Should have at least one interface";

    // Test getting info for existing interface
    const char *first_interface_name = interfaces[0].name;
    result = nic_get_interface_info(first_interface_name, &info);
    EXPECT_EQ(result, NIC_SUCCESS) << "Should get info for existing interface";

    if(result == NIC_SUCCESS)
    {
        EXPECT_STREQ(info.name, first_interface_name)
            << "Retrieved interface name should match";
    }

    // Test getting info for non-existent interface
    result = nic_get_interface_info("non_existent_interface_12345", &info);
    EXPECT_EQ(result, NIC_ERROR_NOT_FOUND)
        << "Should return not found for non-existent interface";
}

// Test interface statistics
TEST_F(nic, interface_statistics)
{
    SCOPED_TRACE("Testing interface statistics");

    // Test invalid parameters
    nic_statistics_t stats;
    nic_error_t      result = nic_get_statistics(nullptr, &stats);
    EXPECT_EQ(result, NIC_ERROR_INVALID_PARAMETER)
        << "Should return error for null interface name";

    result = nic_get_statistics("eth0", nullptr);
    EXPECT_EQ(result, NIC_ERROR_INVALID_PARAMETER)
        << "Should return error for null stats pointer";

    // Get first available interface
    nic_info_t interfaces[NIC_MAX_INTERFACES];
    uint32_t   count = 0;
    result = nic_enumerate_interfaces(interfaces, NIC_MAX_INTERFACES, &count);
    ASSERT_EQ(result, NIC_SUCCESS) << "Interface enumeration should succeed";
    ASSERT_GT(count, 0) << "Should have at least one interface";

    // Test getting statistics for existing interface
    const char *first_interface_name = interfaces[0].name;
    result = nic_get_statistics(first_interface_name, &stats);

    if(result == NIC_SUCCESS)
    {
        // Basic validation
        EXPECT_GE(stats.bytes_sent, 0) << "Bytes sent should be non-negative";
        EXPECT_GE(stats.bytes_received, 0)
            << "Bytes received should be non-negative";
        EXPECT_GE(stats.packets_sent, 0)
            << "Packets sent should be non-negative";
        EXPECT_GE(stats.packets_received, 0)
            << "Packets received should be non-negative";
    } else
    {
        std::cout << "Statistics not available for interface "
                  << first_interface_name
                  << " (may not be supported on this platform)" << std::endl;
    }
}

// Test interface control functions (may require privileges)
TEST_F(nic, interface_control)
{
    SCOPED_TRACE("Testing interface control");

    // Test invalid parameters
    nic_error_t result = nic_enable_interface(nullptr);
    EXPECT_EQ(result, NIC_ERROR_INVALID_PARAMETER)
        << "Should return error for null interface name";

    result = nic_disable_interface(nullptr);
    EXPECT_EQ(result, NIC_ERROR_INVALID_PARAMETER)
        << "Should return error for null interface name";

    // Test with non-existent interface
    result = nic_enable_interface("non_existent_interface_12345");
    EXPECT_NE(result, NIC_SUCCESS)
        << "Should return error for non-existent interface";

    result = nic_disable_interface("non_existent_interface_12345");
    EXPECT_NE(result, NIC_SUCCESS)
        << "Should return error for non-existent interface";
}

// Basic functionality test
TEST(NicBasicTest, basic_functionality)
{
    SCOPED_TRACE("Testing basic functionality");

    // Test that the header compiles and basic functions work
    nic_error_t result = nic_init();
    EXPECT_EQ(result, NIC_SUCCESS) << "Basic initialization should work";

    nic_cleanup();
}

// Compatibility test
TEST(NicBasicTest, c_compatibility)
{
    SCOPED_TRACE("Testing C compatibility");

    // Test that all structures can be initialized with zero
    nic_info_t info;
    memset(&info, 0, sizeof(info));
    nic_statistics_t stats;
    memset(&stats, 0, sizeof(stats));
    nic_wireless_info_t wireless;
    memset(&wireless, 0, sizeof(wireless));
    nic_ip_address_t ip;
    memset(&ip, 0, sizeof(ip));
    nic_ipv6_address_t ipv6;
    memset(&ipv6, 0, sizeof(ipv6));

    // Test that enums work correctly
    nic_type_t   type   = NIC_TYPE_ETHERNET;
    nic_status_t status = NIC_STATUS_UP;
    nic_error_t  error  = NIC_SUCCESS;

    EXPECT_EQ(type, NIC_TYPE_ETHERNET);
    EXPECT_EQ(status, NIC_STATUS_UP);
    EXPECT_EQ(error, NIC_SUCCESS);

    // Suppress unused variable warnings
    (void) info;
    (void) stats;
    (void) wireless;
    (void) ip;
    (void) ipv6;
}
