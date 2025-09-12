#ifndef ERROR_H
#define ERROR_H

#include <libcpp/testing/error.hpp>
#include <libcpp/util/init.hpp>

using err_t = std::error_code;

enum err
{
    err_invalid_subcmd,
    err_invalid_fmt,

    err_invalid_input,
    err_invalid_output,
    err_invalid_algo,
    err_invalid_key,
    err_invalid_padding,
    err_invalid_iv,
    err_encrypt_aes_failed,
    err_encrypt_base64_failed,
    err_encrypt_des_failed,
    err_encrypt_md5_failed,
    err_encrypt_sha256_failed,
    err_encrypt_rsa_failed,

    err_invalid_decrypt_subcmd,
    err_invalid_decrypt_input,
    err_invalid_decrypt_output,
    err_invalid_decrypt_algo,
    err_invalid_decrypt_key,
    err_invalid_decrypt_padding,
    err_invalid_decrypt_iv,
    err_decrypt_aes_failed,
    err_decrypt_base64_failed,
    err_decrypt_des_failed,
    err_decrypt_md5_failed,
    err_decrypt_sha256_failed,
    err_decrypt_rsa_failed,
};

INIT(
    libcpp::register_err("crypto", err_invalid_subcmd, "invalid subcmd");
    libcpp::register_err("crypto", err_invalid_fmt, "invalid format");

    libcpp::register_err("crypto", err_invalid_input, "invalid input path");
    libcpp::register_err("crypto", err_invalid_output, "invalid output path");
    libcpp::register_err("crypto", err_invalid_algo, "invalid algorithm");
    libcpp::register_err("crypto", err_invalid_key, "invalid key");
    libcpp::register_err("crypto", err_invalid_padding, "invalid padding");
    libcpp::register_err("crypto", err_invalid_iv, "invalid IV");
    libcpp::register_err("crypto", err_encrypt_aes_failed, "AES encrypt failed");
    libcpp::register_err("crypto", err_encrypt_base64_failed, "base64 encrypt failed");
    libcpp::register_err("crypto", err_encrypt_des_failed, "DES encrypt failed");
    libcpp::register_err("crypto", err_encrypt_md5_failed, "MD5 encrypt failed");
    libcpp::register_err("crypto", err_encrypt_sha256_failed, "SHA256 encrypt failed");
    libcpp::register_err("crypto", err_encrypt_rsa_failed, "RSA encrypt failed");

    libcpp::register_err("crypto", err_invalid_decrypt_subcmd, "invalid decrypt subcmd");
    libcpp::register_err("crypto", err_invalid_decrypt_input, "invalid decrypt input path");
    libcpp::register_err("crypto", err_invalid_decrypt_output, "invalid decrypt output path");
    libcpp::register_err("crypto", err_invalid_decrypt_algo, "invalid decrypt algorithm");
    libcpp::register_err("crypto", err_invalid_decrypt_key, "invalid decrypt key");
    libcpp::register_err("crypto", err_invalid_decrypt_padding, "invalid decrypt padding");
    libcpp::register_err("crypto", err_invalid_decrypt_iv, "invalid decrypt IV");
    libcpp::register_err("crypto", err_decrypt_aes_failed, "AES decrypt failed");
    libcpp::register_err("crypto", err_decrypt_base64_failed, "base64 decrypt failed");
    libcpp::register_err("crypto", err_decrypt_des_failed, "DES decrypt failed");
    libcpp::register_err("crypto", err_decrypt_md5_failed, "MD5 decrypt failed");
    libcpp::register_err("crypto", err_decrypt_sha256_failed, "SHA256 decrypt failed");
    libcpp::register_err("crypto", err_decrypt_rsa_failed, "RSA decrypt failed");
);

static inline std::error_code error(const err e) { return libcpp::make_err(e, "crypto"); }

#endif