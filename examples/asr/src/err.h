#ifndef ERR_H
#define ERR_H

#include <hj/testing/error.hpp>
#include <hj/testing/error_handler.hpp>
#include <hj/util/init.hpp>

#define OK 0
#define ERR_INVALID_SUBCMD 1
#define ERR_ARGC_TOO_LESS 2

#define ASR_ERR_FAIL 100

using err_t = std::error_code;

INIT(hj::register_err("asr", ASR_ERR_FAIL, "asr failed");)

INIT(hj::register_err("crypto", ERR_INVALID_SUBCMD, "invalid subcmd");
     hj::register_err("crypto", ERR_ARGC_TOO_LESS, "argc too less");)

static inline err_t error(const int e)
{
    return hj::make_err(e, "crypto");
}

#endif