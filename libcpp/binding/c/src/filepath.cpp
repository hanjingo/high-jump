#include <libcpp/io/filepath.hpp>

extern "C" {
#include "filepath.h"
}

LIBCPP_API const char* libcpp_file_path_pwd()
{
    return libcpp::file_path::pwd().c_str();
}