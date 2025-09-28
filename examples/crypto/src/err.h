#ifndef ERR_H
#define ERR_H

#include <hj/testing/error.hpp>
#include <hj/util/init.hpp>

#include "examples/crypto_core/src/crypto_core.h"
#include "examples/db_core/src/db_core.h"

#define OK 0
#define ERR_INVALID_SUBCMD 1
#define ERR_ARGC_TOO_LESS 2

using err_t = std::error_code;

INIT(hj::register_err("crypto", CRYPTO_ERR_FAIL, "crypto failed");
     hj::register_err("crypto",
                      CRYPTO_ERR_CORE_NOT_LOAD,
                      "crypto core not load");
     hj::register_err("crypto",
                      CRYPTO_ERR_CORE_LOAD_FAIL,
                      "crypto core load failed");
     hj::register_err("crypto",
                      CRYPTO_ERR_VERSION_MISMATCH,
                      "crypto version mismatch");
     hj::register_err("crypto", CRYPTO_ERR_INIT_FAIL, "crypto init failed");
     hj::register_err("crypto", CRYPTO_ERR_QUIT_FAIL, "crypto quit failed");

     hj::register_err("crypto", CRYPTO_ERR_INVALID_FMT, "invalid format");
     hj::register_err("crypto", CRYPTO_ERR_INVALID_INPUT, "invalid input path");
     hj::register_err("crypto",
                      CRYPTO_ERR_INVALID_OUTPUT,
                      "invalid output path");
     hj::register_err("crypto", CRYPTO_ERR_INVALID_ALGO, "invalid algorithm");
     hj::register_err("crypto", CRYPTO_ERR_INVALID_KEY, "invalid key");
     hj::register_err("crypto", CRYPTO_ERR_INVALID_PADDING, "invalid padding");
     hj::register_err("crypto", CRYPTO_ERR_INVALID_IV, "invalid IV");
     hj::register_err("crypto", CRYPTO_ERR_INVALID_MODE, "invalid mode");

     hj::register_err("crypto",
                      CRYPTO_ERR_ENCRYPT_AES_FAILED,
                      "AES encrypt failed");
     hj::register_err("crypto",
                      CRYPTO_ERR_ENCRYPT_BASE64_FAILED,
                      "base64 encrypt failed");
     hj::register_err("crypto",
                      CRYPTO_ERR_ENCRYPT_DES_FAILED,
                      "DES encrypt failed");
     hj::register_err("crypto",
                      CRYPTO_ERR_ENCRYPT_MD5_FAILED,
                      "MD5 encrypt failed");
     hj::register_err("crypto",
                      CRYPTO_ERR_ENCRYPT_SHA256_FAILED,
                      "SHA256 encrypt failed");
     hj::register_err("crypto",
                      CRYPTO_ERR_ENCRYPT_RSA_FAILED,
                      "RSA encrypt failed");

     hj::register_err("crypto",
                      CRYPTO_ERR_DECRYPT_AES_FAILED,
                      "AES decrypt failed");
     hj::register_err("crypto",
                      CRYPTO_ERR_DECRYPT_BASE64_FAILED,
                      "base64 decrypt failed");
     hj::register_err("crypto",
                      CRYPTO_ERR_DECRYPT_DES_FAILED,
                      "DES decrypt failed");
     hj::register_err("crypto",
                      CRYPTO_ERR_DECRYPT_MD5_FAILED,
                      "MD5 decrypt failed");
     hj::register_err("crypto",
                      CRYPTO_ERR_DECRYPT_SHA256_FAILED,
                      "SHA256 decrypt failed");
     hj::register_err("crypto",
                      CRYPTO_ERR_DECRYPT_RSA_FAILED,
                      "RSA decrypt failed");

     hj::register_err("crypto", CRYPTO_ERR_KEYGEN_FAIL, "keygen failed");

     hj::register_err("crypto", CRYPTO_ERR_FORMAT_FAIL, "format failed");
     hj::register_err("crypto",
                      CRYPTO_ERR_FORMAT_STYLE_NOT_FOUND,
                      "format style not found");
     hj::register_err("crypto",
                      CRYPTO_ERR_FORMAT_INVALID_TARGET,
                      "format invalid target");
     hj::register_err("crypto",
                      CRYPTO_ERR_FORMAT_HEX_FAILED,
                      "format hex failed");
     hj::register_err("crypto",
                      CRYPTO_ERR_FORMAT_BASE64_FAILED,
                      "format base64 failed");

     hj::register_err("crypto", CRYPTO_ERR_UNFORMAT_FAIL, "unformat failed");
     hj::register_err("crypto",
                      CRYPTO_ERR_UNFORMAT_STYLE_NOT_FOUND,
                      "unformat style not found");
     hj::register_err("crypto",
                      CRYPTO_ERR_UNFORMAT_INVALID_TARGET,
                      "unformat invalid target");
     hj::register_err("crypto",
                      CRYPTO_ERR_UNFORMAT_HEX_FAILED,
                      "unformat hex failed");
     hj::register_err("crypto",
                      CRYPTO_ERR_UNFORMAT_BASE64_FAILED,
                      "unformat base64 failed");)

INIT(hj::register_err("crypto", ERR_INVALID_SUBCMD, "invalid subcmd");
     hj::register_err("crypto", ERR_ARGC_TOO_LESS, "argc too less");)

#endif