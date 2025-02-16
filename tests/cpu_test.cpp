#include <gtest/gtest.h>
#include <libcpp/os/cpu.hpp>

TEST(cpu, cores)
{
    ASSERT_EQ(libcpp::cpu::cores() > 0, true);
}

TEST(cpu, bind)
{
    ASSERT_EQ(libcpp::cpu::bind(0), false);
}