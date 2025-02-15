#ifndef CPU_H
#define CPU_H

#include <libcpp/binding/c/api.h>

LIBCPP_API unsigned int libcpp_cpu_cores();

LIBCPP_API bool libcpp_cpu_bind(const unsigned int core);

#endif