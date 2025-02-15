#include <libcpp/types/any.hpp>

extern "C" {
#include "any.h"
}

LIBCPP_API int libcpp_any_cast(void* src)
{
    return libcpp::any_cast<int>(src);
}