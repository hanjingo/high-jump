#include <gtest/gtest.h>
#include <libcpp/hardware/cpu.h>

TEST(cpu, cores)
{
    ASSERT_EQ(cpu_cores > 0, true);
}

TEST(cpu, bind)
{
    cpu_bind(0);
}