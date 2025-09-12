#ifndef ERR_HANDLE_H
#define ERR_HANDLE_H

#include "err.h"
#include "comm.h"

template<typename... Args>
static void handle_err(std::error_code err, Args&& ... args)
{
    switch (err.value())
    {
        case err_invalid_subcmd: 
        {
            LOG_ERROR("unknown subcommand: \"{}\"; we expected one of these subcommands: [{}]", 
                std::forward<Args>(args)..., print_str_vector(subcmds));
            break;
        }
        case err_invalid_fmt: 
        {
            LOG_ERROR("not support this format:\"{}\"; we expected one of these formats: [{}]", 
                std::forward<Args>(args)..., print_str_vector(fmts));
            break;
        }

        // encrypt errors
        case err_invalid_input: 
        {
            LOG_ERROR("invalid input: in:\"{}\", ctx:\"{}\", algo:\"{}\", mode:\"{}\", key:\"{}\", padding:\"{}\"", std::forward<Args>(args)...);
            break;
        }
        case err_invalid_output: 
        {
            LOG_ERROR("invalid output: \"{}\"", std::forward<Args>(args)...);
            break;
        }
        case err_invalid_algo: 
        {
            LOG_ERROR("not support this algo:\"{}\"; we expected one of these algos: [{}]", std::forward<Args>(args)..., print_str_vector(algos));
            break;
        }
        case err_invalid_key: 
        {
            LOG_ERROR("invalid key: key:\"{}\", algo:\"{}\", mode:\"{}\"", std::forward<Args>(args)...);
            break;
        }
        case err_invalid_padding: 
        {
            LOG_ERROR("invalid padding: padding:\"{}\", algo:\"{}\"", std::forward<Args>(args)...);
            break;
        }
        case err_invalid_iv: 
        {
            LOG_ERROR("invalid IV: iv:\"{}\", algo:\"{}\", mode:\"{}\"", std::forward<Args>(args)...);
            break;
        }
        case err_encrypt_aes_failed:
        {
            LOG_ERROR("AES encrypt failed: out:\"{}\", in:\"{}\", algo:\"{}\", mode:\"{}\", key:\"{}\", padding:\"{}\", iv:\"{}\", ctx:\"{}\"", 
                std::forward<Args>(args)...);
            break;
        }
        case err_encrypt_base64_failed:
        {
            LOG_ERROR("base64 encrypt failed: out:\"{}\", in:\"{}\", ctx:\"{}\"", std::forward<Args>(args)...);
            break;
        }
        case err_encrypt_des_failed:
        {
            LOG_ERROR("DES encrypt failed: out:\"{}\", in:\"{}\", algo:\"{}\", mode:\"{}\", key:\"{}\", padding:\"{}\", iv:\"{}\", ctx:\"{}\"", 
                std::forward<Args>(args)...);
            break;
        }
        case err_encrypt_md5_failed:
        {
            LOG_ERROR("MD5 encrypt failed: out:\"{}\", in:\"{}\", ctx:\"{}\"", std::forward<Args>(args)...);
            break;
        }
        case err_encrypt_sha256_failed:
        {
            LOG_ERROR("SHA256 encrypt failed: out:\"{}\", in:\"{}\", ctx:\"{}\"", std::forward<Args>(args)...);
            break;
        }
        case err_encrypt_rsa_failed:
        {
            LOG_ERROR("RSA encrypt failed: out:\"{}\", in:\"{}\", algo:\"{}\", mode:\"{}\", key:\"{}\", padding:\"{}\", iv:\"{}\", ctx:\"{}\"", 
                std::forward<Args>(args)...);
            break;
        }

        // decrypt errors
        case err_invalid_decrypt_input: 
        {
            LOG_ERROR("invalid decrypt input: in:\"{}\", ctx:\"{}\", algo:\"{}\", key:\"{}\", padding:\"{}\"", std::forward<Args>(args)...);
            break;
        }
        case err_invalid_decrypt_output: 
        {
            LOG_ERROR("invalid decrypt output: \"{}\"", std::forward<Args>(args)...);
            break;
        }
        case err_invalid_decrypt_algo:
        {
            LOG_ERROR("not support this decrypt algo:\"{}\"; we expected one of these decrypt algos: [{}]", std::forward<Args>(args)..., print_str_vector(decrypt_algos));
            break;
        }
        case err_invalid_decrypt_key: 
        {
            LOG_ERROR("invalid decrypt key: key:\"{}\", algo:\"{}\", mode:\"{}\"", std::forward<Args>(args)...);
            break;
        }
        case err_invalid_decrypt_padding: 
        {
            LOG_ERROR("invalid decrypt padding: padding:\"{}\", algo:\"{}\"", std::forward<Args>(args)...);
            break;
        }
        case err_invalid_decrypt_iv: 
        {
            LOG_ERROR("invalid decrypt IV: iv:\"{}\", algo:\"{}\", mode:\"{}\"", std::forward<Args>(args)...);
            break;
        }
        case err_decrypt_aes_failed:
        {
            LOG_ERROR("AES decrypt failed: out:\"{}\", in:\"{}\", algo:\"{}\", mode:\"{}\", key:\"{}\", padding:\"{}\", iv:\"{}\", ctx:\"{}\"", 
                std::forward<Args>(args)...);
            break;
        }
        case err_decrypt_base64_failed:
        {
            LOG_ERROR("base64 decrypt failed: out:\"{}\", in:\"{}\", ctx:\"{}\"", std::forward<Args>(args)...);
            break;
        }
        case err_decrypt_des_failed:
        {
            LOG_ERROR("DES decrypt failed: out:\"{}\", in:\"{}\", algo:\"{}\", mode:\"{}\", key:\"{}\", padding:\"{}\", iv:\"{}\", ctx:\"{}\"", 
                std::forward<Args>(args)...);
            break;
        }
        case err_decrypt_md5_failed:
        {
            LOG_ERROR("MD5 decrypt failed: out:\"{}\", in:\"{}\", ctx:\"{}\"", std::forward<Args>(args)...);
            break;
        }
        case err_decrypt_sha256_failed:
        {
            LOG_ERROR("SHA256 decrypt failed: out:\"{}\", in:\"{}\", ctx:\"{}\"", std::forward<Args>(args)...);
            break;
        }
        case err_decrypt_rsa_failed:
        {
            LOG_ERROR("RSA decrypt failed: out:\"{}\", in:\"{}\", algo:\"{}\", mode:\"{}\", key:\"{}\", padding:\"{}\", iv:\"{}\", ctx:\"{}\"", 
                std::forward<Args>(args)...);
            break;
        }
        case err_keygen_fail:
        {
            LOG_ERROR("keygen failed: name:\"{}\", fmt:\"{}\", mode:\"{}\", bits:\"{}\"", std::forward<Args>(args)...);
            break;
        }
        default: {
            break;
        }
    }
}

#endif