#ifndef ERR_H
#define ERR_H

#include <hj/testing/error.hpp>
#include <hj/util/init.hpp>

#include "examples/lic_core/src/lic_core.h"

#define OK                                0
#define ERR_FAIL                          1
#define ERR_INVALID_SUBCMD                2
#define ERR_ARGC_TOO_LESS                 3
#define ERR_LIC_CORE_LOAD_FAIL            4
#define ERR_LIC_CORE_VERSION_MISMATCH     5
#define ERR_LIC_CORE_CLAIM_NUM_EXCEED     6

using err_t = std::error_code;

INIT(
    hj::register_err("app", ERR_INVALID_SUBCMD, "invalid subcmd");
    hj::register_err("app", ERR_ARGC_TOO_LESS, "argc too less");
    hj::register_err("app", ERR_LIC_CORE_LOAD_FAIL, "lic core load fail");
    hj::register_err("app", ERR_LIC_CORE_VERSION_MISMATCH, "lic core version mismatch");
    hj::register_err("app", ERR_LIC_CORE_CLAIM_NUM_EXCEED, "lic core claim num exceed");
)

INIT(
    hj::register_err("lic", LIC_ERR_FAIL, "lic failed");
    hj::register_err("lic", LIC_ERR_INIT_FAIL, "lic init failed");
    hj::register_err("lic", LIC_ERR_INVALID_PARAM, "lic invalid param");
    hj::register_err("lic", LIC_ERR_ISSUER_EXISTED, "lic issuer existed");
    hj::register_err("lic", LIC_ERR_ISSUER_NOT_EXIST, "lic issuer not existed");
    hj::register_err("lic", LIC_ERR_INVALID_TIMES, "lic invalid times");
    hj::register_err("lic", LIC_ERR_CLAIM_MISMATCH, "lic claim mismatch");
    hj::register_err("lic", LIC_ERR_KEYS_NOT_CHANGED, "lic keys not changed");
    hj::register_err("lic", LIC_ERR_KEYS_NOT_ENOUGH, "lic keys not enough");
    hj::register_err("lic", LIC_ERR_FILE_NOT_EXIST, "lic file not exist");
    hj::register_err("lic", LIC_ERR_INPUT_STREAM_INVALID, "lic input stream invalid");
    hj::register_err("lic", LIC_ERR_OUTPUT_STREAM_INVALID, "lic output stream invalid");
)

#endif