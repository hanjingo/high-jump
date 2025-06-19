#include <gtest/gtest.h>
#include <libcpp/hardware/cpu.h>
#include <iostream>

TEST(cpu, cpu_core_n)
{
    ASSERT_EQ(cpu_core_n() > 0, true);
}

TEST(cpu, cpu_bind)
{
// macos not support cpu bind
#if defined(_WIN32) || defined(__linux__)
    unsigned int buf[1024];
    unsigned int sz = 1024;
    cpu_core_list(buf, sz);
    ASSERT_EQ(sz != 0, true);
    for (int i = 0; i < sz; ++i)
    {
        ASSERT_EQ(cpu_bind(buf[i]), true);
    }
#endif
}

TEST(cpu, cpu_core_list)
{
    unsigned int buf[1024];
    unsigned int sz = 1024;
    cpu_core_list(buf, sz);
    ASSERT_EQ(sz != 0, true);
    for (int i = 0; i < sz; ++i)
    {
        std::cout << "cpu core:" << buf[i] << std::endl;
    }
}