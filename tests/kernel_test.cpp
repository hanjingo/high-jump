#include <gtest/gtest.h>

#define HJ_KERNEL_IMPL
#include <hj/os/kernel.h>
#include <string>

TEST(kernel, hj_kernel_name)
{
    const char *name = hj_kernel_name();
    EXPECT_TRUE(strcmp(name, "Linux") == 0 || strcmp(name, "Darwin") == 0
                || strcmp(name, "Windows") == 0
                || strcmp(name, "Unknown") == 0);
}

TEST(kernel, hj_kernel_version)
{
    char        version[128];
    const char *v = hj_kernel_version(version, sizeof(version));
    EXPECT_FALSE(v == nullptr);
    EXPECT_FALSE(std::string(v).empty());
}

TEST(kernel, hj_kernel_uptime)
{
    uint64_t up = hj_kernel_uptime();
    // Uptime may be zero on some platforms, but should not be negative
    EXPECT_GE(up, 0);

    char        buffer[64];
    const char *uptime_str = hj_kernel_uptime_str(
        buffer,
        sizeof(buffer),
        "%llu days, %llu hours, %llu minutes, %llu seconds");
    EXPECT_FALSE(uptime_str == nullptr);
}

TEST(kernel, hj_kernel_info)
{
    kernel_info_t info;
    EXPECT_TRUE(hj_kernel_info(&info)) << "kernel_info should succeed";
    EXPECT_GT(strlen(info.name), 0) << "Name should not be empty";
    EXPECT_GT(strlen(info.version), 0) << "Version should not be empty";
    EXPECT_GE(info.uptime_seconds, 0) << "Uptime should not be negative";
    EXPECT_EQ(info.name[sizeof(info.name) - 1], '\0')
        << "Name should be null-terminated";
    EXPECT_EQ(info.version[sizeof(info.version) - 1], '\0')
        << "Version should be null-terminated";

    EXPECT_FALSE(hj_kernel_info(nullptr))
        << "kernel_info should fail with null pointer";
}