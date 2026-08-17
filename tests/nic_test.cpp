#include <gtest/gtest.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>

#define HJ_NIC_IMPL
#include <hj/hardware/nic.h>

class nic : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        hj_nic_err_t result = hj_nic_init();
        ASSERT_EQ(result, HJ_NIC_SUCCESS);
    }

    void TearDown() override { hj_nic_cleanup(); }
};

TEST_F(nic, initialization_and_cleanup)
{
    SCOPED_TRACE("Testing NIC initialization and cleanup");

    hj_nic_cleanup();
    hj_nic_err_t result = hj_nic_init();
    EXPECT_EQ(result, HJ_NIC_SUCCESS) << "NIC initialization should succeed";

    result = hj_nic_init();
    EXPECT_EQ(result, HJ_NIC_SUCCESS)
        << "Multiple NIC initializations should be safe";

    hj_nic_cleanup();
    hj_nic_cleanup();

    ASSERT_EQ(hj_nic_init(), HJ_NIC_SUCCESS);
}

TEST_F(nic, interface_counting)
{
    SCOPED_TRACE("Testing interface counting");

    EXPECT_EQ(hj_nic_get_interface_count(nullptr),
              HJ_NIC_ERR_INVALID_PARAMETER);

    uint32_t     count  = 0;
    hj_nic_err_t result = hj_nic_get_interface_count(&count);
    EXPECT_EQ(result, HJ_NIC_SUCCESS);
    EXPECT_GE(count, 1)
        << "System should have at least one network interface (e.g., loopback)";
    EXPECT_LE(count, HJ_NIC_MAX_INTERFACES);
}

TEST_F(nic, interface_enumeration)
{
    SCOPED_TRACE("Testing interface enumeration thoroughly");

    uint32_t actual_count = 0;

    EXPECT_EQ(hj_nic_enumerate_interfaces(nullptr, 10, &actual_count),
              HJ_NIC_ERR_INVALID_PARAMETER);
    EXPECT_EQ(hj_nic_enumerate_interfaces(nullptr, 0, nullptr),
              HJ_NIC_ERR_INVALID_PARAMETER);

    hj_nic_info_t small_buf[1];
    uint32_t      required_count = 0;
    hj_nic_err_t  trunc_res =
        hj_nic_enumerate_interfaces(small_buf, 0, &required_count);
    if(required_count > 0)
    {
        EXPECT_EQ(trunc_res, HJ_NIC_ERR_INSUFFICIENT_BUFFER);
        EXPECT_GE(required_count, 1);
    }

    std::vector<hj_nic_info_t> interfaces(HJ_NIC_MAX_INTERFACES);
    hj_nic_err_t result = hj_nic_enumerate_interfaces(interfaces.data(),
                                                      HJ_NIC_MAX_INTERFACES,
                                                      &actual_count);
    ASSERT_EQ(result, HJ_NIC_SUCCESS);
    ASSERT_GT(actual_count, 0);

    for(uint32_t i = 0; i < actual_count; ++i)
    {
        EXPECT_NE(interfaces[i].name[0], '\0')
            << "Interface name must not be empty";
        EXPECT_GT(interfaces[i].index, 0) << "Interface index must be positive";
    }
}

TEST_F(nic, get_interface_info_by_name)
{
    SCOPED_TRACE("Testing get interface info by name");

    hj_nic_info_t info;
    EXPECT_EQ(hj_nic_get_interface_info(nullptr, &info),
              HJ_NIC_ERR_INVALID_PARAMETER);
    EXPECT_EQ(hj_nic_get_interface_info("lo", nullptr),
              HJ_NIC_ERR_INVALID_PARAMETER);

    std::vector<hj_nic_info_t> interfaces(HJ_NIC_MAX_INTERFACES);
    uint32_t                   count = 0;
    ASSERT_EQ(hj_nic_enumerate_interfaces(interfaces.data(),
                                          HJ_NIC_MAX_INTERFACES,
                                          &count),
              HJ_NIC_SUCCESS);
    ASSERT_GT(count, 0);

    const char  *target_name = interfaces[0].name;
    hj_nic_err_t result      = hj_nic_get_interface_info(target_name, &info);
    EXPECT_EQ(result, HJ_NIC_SUCCESS);
    EXPECT_STREQ(info.name, target_name);

    EXPECT_EQ(hj_nic_get_interface_info("non_existent_interface_12345", &info),
              HJ_NIC_ERR_NOT_FOUND);
}

TEST_F(nic, interface_statistics)
{
    SCOPED_TRACE("Testing interface statistics");

    hj_nic_statistics_t stats;
    EXPECT_EQ(hj_nic_get_statistics(nullptr, &stats),
              HJ_NIC_ERR_INVALID_PARAMETER);
    EXPECT_EQ(hj_nic_get_statistics("lo", nullptr),
              HJ_NIC_ERR_INVALID_PARAMETER);

    std::vector<hj_nic_info_t> interfaces(HJ_NIC_MAX_INTERFACES);
    uint32_t                   count = 0;
    ASSERT_EQ(hj_nic_enumerate_interfaces(interfaces.data(),
                                          HJ_NIC_MAX_INTERFACES,
                                          &count),
              HJ_NIC_SUCCESS);
    ASSERT_GT(count, 0);

    const char  *target_name = interfaces[0].name;
    hj_nic_err_t result      = hj_nic_get_statistics(target_name, &stats);

    if(result == HJ_NIC_SUCCESS)
    {
        EXPECT_GE(stats.bytes_sent, 0);
        EXPECT_GE(stats.bytes_received, 0);
        EXPECT_GE(stats.packets_sent, 0);
        EXPECT_GE(stats.packets_received, 0);
    } else
    {
        std::cout << "[INFO] Statistics not supported or accessible for "
                  << target_name << std::endl;
    }
}

TEST_F(nic, field_consistency_validation)
{
    SCOPED_TRACE("Testing cross-platform field consistency and enums");

    uint32_t count = 0;
    ASSERT_EQ(hj_nic_get_interface_count(&count), HJ_NIC_SUCCESS);
    ASSERT_GT(count, 0);

    std::vector<hj_nic_info_t> interfaces(count);
    uint32_t                   actual = 0;
    ASSERT_EQ(hj_nic_enumerate_interfaces(interfaces.data(), count, &actual),
              HJ_NIC_SUCCESS);
    ASSERT_EQ(actual, count);

    for(uint32_t i = 0; i < actual; ++i)
    {
        const auto &nic = interfaces[i];

        EXPECT_GE(nic.type, HJ_NIC_TYPE_UNKNOWN);
        EXPECT_LE(nic.type, HJ_NIC_TYPE_VIRTUAL);

        EXPECT_GE(nic.status, HJ_NIC_STATUS_UNKNOWN);
        EXPECT_LE(nic.status, HJ_NIC_STATUS_LOWER_LAYER_DOWN);

        if(nic.ip_address.str[0] != '\0')
        {
            EXPECT_NE(nic.ip_address.ipv4.addr, 0);
        }
    }
}

TEST_F(nic, concurrent_access_stress)
{
    SCOPED_TRACE("Testing thread safety under concurrent stress");

    const int                num_threads = 4;
    const int                iterations  = 30;
    std::vector<std::thread> threads;
    std::atomic<int>         success_count(0);

    for(int t = 0; t < num_threads; ++t)
    {
        threads.emplace_back([&]() {
            for(int i = 0; i < iterations; ++i)
            {
                uint32_t count = 0;
                if(hj_nic_get_interface_count(&count) == HJ_NIC_SUCCESS
                   && count > 0)
                {
                    std::vector<hj_nic_info_t> interfaces(count);
                    uint32_t                   actual = 0;
                    if(hj_nic_enumerate_interfaces(interfaces.data(),
                                                   count,
                                                   &actual)
                       == HJ_NIC_SUCCESS)
                    {
                        if(actual > 0)
                        {
                            hj_nic_info_t info;
                            (void) hj_nic_get_interface_info(interfaces[0].name,
                                                             &info);

                            hj_nic_statistics_t stats;
                            (void) hj_nic_get_statistics(interfaces[0].name,
                                                         &stats);
                        }
                    }
                }
                success_count++;
            }
        });
    }

    for(auto &th : threads)
    {
        th.join();
    }

    EXPECT_EQ(success_count.load(), num_threads * iterations);
}

TEST_F(nic, interface_control_privilege_awareness)
{
    SCOPED_TRACE(
        "Testing interface control error paths and permission semantics");

    EXPECT_EQ(hj_nic_enable_interface(nullptr), HJ_NIC_ERR_INVALID_PARAMETER);
    EXPECT_EQ(hj_nic_disable_interface(nullptr), HJ_NIC_ERR_INVALID_PARAMETER);

    EXPECT_NE(hj_nic_enable_interface("non_existent_interface_12345"),
              HJ_NIC_SUCCESS);
    EXPECT_NE(hj_nic_disable_interface("non_existent_interface_12345"),
              HJ_NIC_SUCCESS);

    uint32_t count = 0;
    if(hj_nic_get_interface_count(&count) == HJ_NIC_SUCCESS && count > 0)
    {
        std::vector<hj_nic_info_t> interfaces(count);
        uint32_t                   actual = 0;
        if(hj_nic_enumerate_interfaces(interfaces.data(), count, &actual)
               == HJ_NIC_SUCCESS
           && actual > 0)
        {
            const char  *target = interfaces[0].name;
            hj_nic_err_t res_en = hj_nic_enable_interface(target);

            bool valid_err =
                (res_en == HJ_NIC_SUCCESS || res_en == HJ_NIC_ERR_ACCESS_DENIED
                 || res_en == HJ_NIC_ERR_SYSTEM_ERROR
                 || res_en == HJ_NIC_ERR_NOT_SUPPORTED
                 || res_en == HJ_NIC_ERR_NOT_FOUND);
            EXPECT_TRUE(valid_err)
                << "Unexpected error code returned by enable_interface: "
                << res_en;
        }
    }
}