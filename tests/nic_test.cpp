#include <gtest/gtest.h>
#include <hj/hardware/nic.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>

class NicTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        //         std::cout << "\n=== Network Interface Card Tests Setup ===" << std::endl;

        // Initialize NIC subsystem
        nic_error_t result = nic_init();
        ASSERT_EQ(result, NIC_SUCCESS);

        //         // Print platform information
        //         std::cout << "Platform: ";
        // #if defined(NIC_PLATFORM_WINDOWS)
        //         std::cout << "Windows" << std::endl;
        // #elif defined(NIC_PLATFORM_LINUX)
        //         std::cout << "Linux" << std::endl;
        // #elif defined(NIC_PLATFORM_MACOS)
        //         std::cout << "macOS" << std::endl;
        // #else
        //         std::cout << "Unknown" << std::endl;
        // #endif

        //         std::cout << "==========================================" << std::endl;
    }

    void TearDown() override
    {
        // std::cout << "=== Network Interface Card Tests Teardown ===" << std::endl;
        nic_cleanup();
    }
};

// Test NIC subsystem initialization and cleanup
TEST_F(NicTest, initialization_and_cleanup)
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

    // std::cout << "NIC initialization and cleanup tests completed" << std::endl;
}

// Test interface counting
TEST_F(NicTest, interface_counting)
{
    SCOPED_TRACE("Testing interface counting");

    // std::cout << "Testing interface counting..." << std::endl;

    // Test invalid parameter
    nic_error_t result = nic_get_interface_count(nullptr);
    EXPECT_EQ(result, NIC_ERROR_INVALID_PARAMETER)
        << "Should return error for null parameter";

    // Test valid counting
    uint32_t count = 0;
    result         = nic_get_interface_count(&count);
    EXPECT_EQ(result, NIC_SUCCESS) << "Interface counting should succeed";

    // std::cout << "Found " << count << " network interfaces" << std::endl;

    // Most systems should have at least one interface (loopback)
    EXPECT_GE(count, 1) << "System should have at least one network interface";
    EXPECT_LE(count, NIC_MAX_INTERFACES)
        << "Interface count should not exceed maximum";

    // std::cout << "Interface counting tests completed" << std::endl;
}

// Test interface enumeration
TEST_F(NicTest, interface_enumeration)
{
    SCOPED_TRACE("Testing interface enumeration");

    // std::cout << "Testing interface enumeration..." << std::endl;

    // // Test invalid parameters
    // uint32_t actual_count = 0;
    // nic_error_t result = nic_enumerate_interfaces(nullptr, 10, &actual_count);
    // EXPECT_EQ(result, NIC_ERROR_INVALID_PARAMETER) << "Should return error for null interfaces array";

    // nic_info_t interfaces[1];
    // result = nic_enumerate_interfaces(interfaces, 1, nullptr);
    // EXPECT_EQ(result, NIC_ERROR_INVALID_PARAMETER) << "Should return error for null actual_count";

    // result = nic_enumerate_interfaces(interfaces, 0, &actual_count);
    // EXPECT_EQ(result, NIC_ERROR_INVALID_PARAMETER) << "Should return error for zero max_interfaces";

    // // Test valid enumeration
    // nic_info_t all_interfaces[NIC_MAX_INTERFACES];
    // result = nic_enumerate_interfaces(all_interfaces, NIC_MAX_INTERFACES, &actual_count);
    // EXPECT_EQ(result, NIC_SUCCESS) << "Interface enumeration should succeed";

    // // std::cout << "Enumerated " << actual_count << " interfaces:" << std::endl;

    // for (uint32_t i = 0; i < actual_count; i++) {
    //     const nic_info_t& nic = all_interfaces[i];

    //     // std::cout << "  Interface " << i + 1 << ":" << std::endl;
    //     // std::cout << "    Name: " << nic.name << std::endl;
    //     // std::cout << "    Description: " << nic.description << std::endl;
    //     // std::cout << "    Index: " << nic.index << std::endl;
    //     // std::cout << "    MTU: " << nic.mtu << std::endl;
    //     // std::cout << "    Speed: " << nic.speed << " bps" << std::endl;
    //     // std::cout << "    IP Address: " << nic.ip_address.str << std::endl;
    //     // std::cout << "    Subnet Mask: " << nic.subnet_mask.str << std::endl;
    //     // std::cout << "    Gateway: " << nic.gateway.str << std::endl;
    //     // std::cout << "    IPv6 Address: " << nic.ipv6_address.str << std::endl;
    //     // std::cout << "    DHCP Enabled: " << (nic.dhcp_enabled ? "Yes" : "No") << std::endl;
    //     // std::cout << "    Is Virtual: " << (nic.is_virtual ? "Yes" : "No") << std::endl;
    //     // std::cout << "    Is Wireless: " << (nic.is_wireless ? "Yes" : "No") << std::endl;

    //     // Validate interface data
    //     EXPECT_GT(strlen(nic.name), 0) << "Interface name should not be empty";
    //     EXPECT_LT(strlen(nic.name), NIC_MAX_NAME_LENGTH) << "Interface name should not exceed maximum length";
    //     EXPECT_NE(nic.type, NIC_TYPE_UNKNOWN) << "Interface type should be known for interface " << nic.name;

    //     // std::cout << std::endl;
    // }

    // // std::cout << "Interface enumeration tests completed" << std::endl;
}

// Test getting interface information by name
TEST_F(NicTest, get_interface_info_by_name)
{
    SCOPED_TRACE("Testing get interface info by name");

    // std::cout << "Testing get interface info by name..." << std::endl;

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
        // std::cout << "Successfully retrieved info for interface: " << info.name << std::endl;
    }

    // Test getting info for non-existent interface
    result = nic_get_interface_info("non_existent_interface_12345", &info);
    EXPECT_EQ(result, NIC_ERROR_NOT_FOUND)
        << "Should return not found for non-existent interface";

    // std::cout << "Get interface info by name tests completed" << std::endl;
}

// Test interface statistics
TEST_F(NicTest, interface_statistics)
{
    SCOPED_TRACE("Testing interface statistics");

    // std::cout << "Testing interface statistics..." << std::endl;

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
        // std::cout << "Statistics for interface " << first_interface_name << ":" << std::endl;
        // std::cout << "  Bytes sent: " << stats.bytes_sent << std::endl;
        // std::cout << "  Bytes received: " << stats.bytes_received << std::endl;
        // std::cout << "  Packets sent: " << stats.packets_sent << std::endl;
        // std::cout << "  Packets received: " << stats.packets_received << std::endl;
        // std::cout << "  Send errors: " << stats.errors_sent << std::endl;
        // std::cout << "  Receive errors: " << stats.errors_received << std::endl;
        // std::cout << "  Send drops: " << stats.drops_sent << std::endl;
        // std::cout << "  Receive drops: " << stats.drops_received << std::endl;

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

    // // Test getting statistics for non-existent interface
    // result = nic_get_statistics("non_existent_interface_12345", &stats);
    // EXPECT_EQ(result, NIC_ERROR_NOT_FOUND) << "Should return not found for non-existent interface";

    // std::cout << "Interface statistics tests completed" << std::endl;
}

// Test interface control functions (may require privileges)
TEST_F(NicTest, interface_control)
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

    // Note: We don't test with real interfaces as it may disrupt network connectivity
    // and require administrative privileges

    // std::cout << "Interface control tests completed (limited due to privilege requirements)" << std::endl;
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
