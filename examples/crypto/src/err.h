#ifndef ERR_H
#define ERR_H

#include <libcpp/testing/error.hpp>
#include <libcpp/util/init.hpp>

#include "examples/crypto_core/src/api.h"

#define OK                 10000
#define ERR_INVALID_SUBCMD 10001

using err_t = std::error_code;

INIT(
    libcpp::register_err("crypto", CRYPTO_ERR_FAIL, "crypto failed");
    libcpp::register_err("crypto", CRYPTO_ERR_CORE_NOT_LOAD, "crypto core not load");
    libcpp::register_err("crypto", CRYPTO_ERR_CORE_LOAD_FAIL, "crypto core load failed");
    libcpp::register_err("crypto", CRYPTO_ERR_VERSION_MISMATCH, "crypto version mismatch");
    libcpp::register_err("crypto", CRYPTO_ERR_INIT_FAIL, "crypto init failed");
    libcpp::register_err("crypto", CRYPTO_ERR_QUIT_FAIL, "crypto quit failed");

    libcpp::register_err("crypto", CRYPTO_ERR_INVALID_FMT, "invalid format");
    libcpp::register_err("crypto", CRYPTO_ERR_INVALID_INPUT, "invalid input path");
    libcpp::register_err("crypto", CRYPTO_ERR_INVALID_OUTPUT, "invalid output path");
    libcpp::register_err("crypto", CRYPTO_ERR_INVALID_ALGO, "invalid algorithm");
    libcpp::register_err("crypto", CRYPTO_ERR_INVALID_KEY, "invalid key");
    libcpp::register_err("crypto", CRYPTO_ERR_INVALID_PADDING, "invalid padding");
    libcpp::register_err("crypto", CRYPTO_ERR_INVALID_IV, "invalid IV");
    libcpp::register_err("crypto", CRYPTO_ERR_INVALID_MODE, "invalid mode");

    libcpp::register_err("crypto", CRYPTO_ERR_ENCRYPT_AES_FAILED, "AES encrypt failed");
    libcpp::register_err("crypto", CRYPTO_ERR_ENCRYPT_BASE64_FAILED, "base64 encrypt failed");
    libcpp::register_err("crypto", CRYPTO_ERR_ENCRYPT_DES_FAILED, "DES encrypt failed");
    libcpp::register_err("crypto", CRYPTO_ERR_ENCRYPT_MD5_FAILED, "MD5 encrypt failed");
    libcpp::register_err("crypto", CRYPTO_ERR_ENCRYPT_SHA256_FAILED, "SHA256 encrypt failed");
    libcpp::register_err("crypto", CRYPTO_ERR_ENCRYPT_RSA_FAILED, "RSA encrypt failed");

    libcpp::register_err("crypto", CRYPTO_ERR_DECRYPT_AES_FAILED, "AES decrypt failed");
    libcpp::register_err("crypto", CRYPTO_ERR_DECRYPT_BASE64_FAILED, "base64 decrypt failed");
    libcpp::register_err("crypto", CRYPTO_ERR_DECRYPT_DES_FAILED, "DES decrypt failed");
    libcpp::register_err("crypto", CRYPTO_ERR_DECRYPT_MD5_FAILED, "MD5 decrypt failed");
    libcpp::register_err("crypto", CRYPTO_ERR_DECRYPT_SHA256_FAILED, "SHA256 decrypt failed");
    libcpp::register_err("crypto", CRYPTO_ERR_DECRYPT_RSA_FAILED, "RSA decrypt failed");

    libcpp::register_err("crypto", CRYPTO_ERR_KEYGEN_FAIL, "keygen failed");

    libcpp::register_err("crypto", CRYPTO_ERR_FORMAT_FAIL, "format failed");
    libcpp::register_err("crypto", CRYPTO_ERR_FORMAT_STYLE_NOT_FOUND, "format style not found");
    libcpp::register_err("crypto", CRYPTO_ERR_FORMAT_INVALID_TARGET, "format invalid target");
    libcpp::register_err("crypto", CRYPTO_ERR_FORMAT_HEX_FAILED, "format hex failed");
    libcpp::register_err("crypto", CRYPTO_ERR_FORMAT_BASE64_FAILED, "format base64 failed");

    libcpp::register_err("crypto", CRYPTO_ERR_UNFORMAT_FAIL, "unformat failed");
    libcpp::register_err("crypto", CRYPTO_ERR_UNFORMAT_STYLE_NOT_FOUND, "unformat style not found");
    libcpp::register_err("crypto", CRYPTO_ERR_UNFORMAT_INVALID_TARGET, "unformat invalid target");
    libcpp::register_err("crypto", CRYPTO_ERR_UNFORMAT_HEX_FAILED, "unformat hex failed");
    libcpp::register_err("crypto", CRYPTO_ERR_UNFORMAT_BASE64_FAILED, "unformat base64 failed");
)

INIT(
    libcpp::register_err("crypto", ERR_INVALID_SUBCMD, "invalid subcmd");
)

#endif