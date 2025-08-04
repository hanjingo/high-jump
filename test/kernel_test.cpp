#include <string>

#include <gtest/gtest.h>
#include <libcpp/os/kernel.h>

using libcpp::kernel;

TEST(KernelTest, Name)
{
    std::string n = kernel::name();
    EXPECT_FALSE(n.empty());
    EXPECT_TRUE(n == "Linux" || n == "Darwin" || n == "Windows" ||
                n == "Unknown");
}

TEST(KernelTest, Version)
{
    std::string v = kernel::version();
    EXPECT_FALSE(v.empty());
    EXPECT_NE(v, "Unknown");
}

TEST(KernelTest, Arch)
{
    std::string a = kernel::arch();
    EXPECT_FALSE(a.empty());
    EXPECT_NE(a, "Unknown");
}

TEST(KernelTest, CpuCount)
{
    int count = kernel::cpu_count();
    EXPECT_GT(count, 0);
    EXPECT_LT(count, 1024);
}

TEST(KernelTest, Uptime)
{
    uint64_t up = kernel::uptime();
    // Uptime may be zero on some platforms, but should not be negative
    EXPECT_GE(up, 0);
}
