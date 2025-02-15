#include <libcpp/os/cpu.hpp>

extern "C" {
#include "cpu.h"
}

LIBCPP_API unsigned int libcpp_cpu_cores()
{
    return libcpp::cpu::cores();
}

LIBCPP_API bool libcpp_cpu_bind(const unsigned int core)
{
    return libcpp::cpu::bind(core);
}