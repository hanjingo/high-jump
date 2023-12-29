#include <gtest/gtest.h>
#include <libcpp/os/cpu.hpp>

TEST(cpu, cpu_cores)
{
    ASSERT_EQ(libcpp::cpu_cores() > 0, true);
}

TEST(cpu, bind_cpu_core)
{
    ASSERT_EQ(libcpp::bind_cpu_core(0), true);
}