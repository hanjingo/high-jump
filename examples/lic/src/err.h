#ifndef ERR_H
#define ERR_H

#include <hj/testing/error.hpp>
#include <hj/util/init.hpp>

#define OK                                0
#define ERR_FAIL                          1
#define ERR_INVALID_SUBCMD                2
#define ERR_ARGC_TOO_LESS                 3
#define ERR_LIC_CORE_LOAD_FAIL            4
#define ERR_LIC_CORE_VERSION_MISMATCH     5

#define LIC_ERR_START 1000
#define LIC_ERR_END   1999
#include "examples/lic_core/src/lic_core.h"

using err_t = std::error_code;

INIT(
    hj::register_err("lic", ERR_INVALID_SUBCMD, "invalid subcmd");
    hj::register_err("lic", ERR_ARGC_TOO_LESS, "argc too less");
)

#endif