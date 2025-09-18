#ifndef HANDLE_H
#define HANDLE_H

#include <system_error>
#include "comm.h"

template<typename... Args>
static void handle_err(std::error_code err, Args&& ... args)
{
    switch (err.value())
    {
        case OK:
        {
            break;
        }
        case ERR_INVALID_SUBCMD:
        {
            LOG_ERROR(tr("app.err_invalid_subcmd").c_str(), 
                std::forward<Args>(args)..., 
                fmt_strs(std::vector<std::string>{"encrypt", "decrypt", "keygen", "guess", "add", "list", "help"}));
            break;
        }

        // crypto
        case CRYPTO_ERR_FAIL:
        {
            LOG_ERROR(tr("log.crypto_err_fail").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_CORE_NOT_LOAD:
        {
            LOG_ERROR(tr("log.crypto_err_core_not_load").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_CORE_LOAD_FAIL:
        {
            LOG_ERROR(tr("log.crypto_err_core_load_fail").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_VERSION_MISMATCH:
        {
            LOG_ERROR(tr("log.crypto_err_version_mismatch").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_INIT_FAIL:
        {
            LOG_ERROR(tr("log.crypto_err_init_fail").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_QUIT_FAIL:
        {
            LOG_ERROR(tr("log.crypto_err_quit_fail").c_str(), std::forward<Args>(args)...);
            break;
        }

        case CRYPTO_ERR_INVALID_FMT: 
        {
            LOG_ERROR(tr("log.crypto_err_invalid_fmt").c_str(), 
                std::forward<Args>(args)..., 
                fmt_strs(std::vector<std::string>{"hex", "base64", "none"}));
            break;
        }
        case CRYPTO_ERR_INVALID_INPUT: 
        {
            LOG_ERROR(tr("log.crypto_err_invalid_input").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_INVALID_OUTPUT: 
        {
            LOG_ERROR(tr("log.crypto_err_invalid_output").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_INVALID_ALGO: 
        {
            LOG_ERROR(tr("log.crypto_err_invalid_algo").c_str(), 
                std::forward<Args>(args)..., 
                fmt_strs(std::vector<std::string>{"aes", "des", "rsa", "sha256", "md5", "base64"}));
            break;
        }
        case CRYPTO_ERR_INVALID_KEY: 
        {
            LOG_ERROR(tr("log.crypto_err_invalid_key").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_INVALID_PADDING: 
        {
            LOG_ERROR(tr("log.crypto_err_invalid_padding").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_INVALID_IV: 
        {
            LOG_ERROR(tr("log.crypto_err_invalid_iv").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_INVALID_MODE: 
        {
            LOG_ERROR(tr("log.crypto_err_invalid_mode").c_str(), 
                std::forward<Args>(args)..., 
                fmt_strs(std::vector<std::string>{"ecb", "cbc", "cfb", "ofb", "ctr"}));
            break;
        }
        
        case CRYPTO_ERR_ENCRYPT_AES_FAILED:
        {
            LOG_ERROR(tr("log.crypto_err_encrypt_aes_failed").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_ENCRYPT_BASE64_FAILED:
        {
            LOG_ERROR(tr("log.crypto_err_encrypt_base64_failed").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_ENCRYPT_DES_FAILED:
        {
            LOG_ERROR(tr("log.crypto_err_encrypt_des_failed").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_ENCRYPT_MD5_FAILED:
        {
            LOG_ERROR(tr("log.crypto_err_encrypt_md5_failed").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_ENCRYPT_SHA256_FAILED:
        {
            LOG_ERROR(tr("log.crypto_err_encrypt_sha256_failed").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_ENCRYPT_RSA_FAILED:
        {
            LOG_ERROR(tr("log.crypto_err_encrypt_rsa_failed").c_str(), 
                std::forward<Args>(args)...);
            break;
        }

        case CRYPTO_ERR_DECRYPT_AES_FAILED:
        {
            LOG_ERROR(tr("log.crypto_err_decrypt_aes_failed").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_DECRYPT_BASE64_FAILED:
        {
            LOG_ERROR(tr("log.crypto_err_decrypt_base64_failed").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_DECRYPT_DES_FAILED:
        {
            LOG_ERROR(tr("log.crypto_err_decrypt_des_failed").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_DECRYPT_MD5_FAILED:
        {
            LOG_ERROR(tr("log.crypto_err_decrypt_md5_failed").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_DECRYPT_SHA256_FAILED:
        {
            LOG_ERROR(tr("log.crypto_err_decrypt_sha256_failed").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_DECRYPT_RSA_FAILED:
        {
            LOG_ERROR(tr("log.crypto_err_decrypt_rsa_failed").c_str(), std::forward<Args>(args)...);
            break;
        }

        case CRYPTO_ERR_KEYGEN_FAIL:
        {
            LOG_ERROR(tr("log.crypto_err_keygen_fail").c_str(), std::forward<Args>(args)...);
            break;
        }

        case CRYPTO_ERR_FORMAT_FAIL:
        {
            LOG_ERROR(tr("log.crypto_err_format_fail").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_FORMAT_STYLE_NOT_FOUND:
        {
            LOG_ERROR(tr("log.crypto_err_format_style_not_found").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_FORMAT_INVALID_TARGET:
        {
            LOG_ERROR(tr("log.crypto_err_format_invalid_target").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_FORMAT_HEX_FAILED:
        {
            LOG_ERROR(tr("log.crypto_err_format_hex_failed").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_FORMAT_BASE64_FAILED:
        {
            LOG_ERROR(tr("log.crypto_err_format_base64_failed").c_str(), std::forward<Args>(args)...);
            break;
        }

        case CRYPTO_ERR_UNFORMAT_FAIL:
        {
            LOG_ERROR(tr("log.crypto_err_unformat_fail").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_UNFORMAT_STYLE_NOT_FOUND:
        {
            LOG_ERROR(tr("log.crypto_err_unformat_style_not_found").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_UNFORMAT_INVALID_TARGET:
        {
            LOG_ERROR(tr("log.crypto_err_unformat_invalid_target").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_UNFORMAT_HEX_FAILED:
        {
            LOG_ERROR(tr("log.crypto_err_unformat_hex_failed").c_str(), std::forward<Args>(args)...);
            break;
        }
        case CRYPTO_ERR_UNFORMAT_BASE64_FAILED:
        {
            LOG_ERROR(tr("log.crypto_err_unformat_base64_failed").c_str(), std::forward<Args>(args)...);
            break;
        }

        // db
        case DB_ERR_FAIL:
        {
            LOG_ERROR(tr("log.db_err_fail").c_str(), std::forward<Args>(args)...);
            break;
        }
        case DB_ERR_CORE_NOT_LOAD:
        {
            LOG_ERROR(tr("log.db_err_core_not_load").c_str(), std::forward<Args>(args)...);
            break;
        }
        case DB_ERR_CORE_LOAD_FAIL:
        {
            LOG_ERROR(tr("log.db_err_core_load_fail").c_str(), std::forward<Args>(args)...);
            break;
        }
        case DB_ERR_VERSION_MISMATCH:
        {
            LOG_ERROR(tr("log.db_err_version_mismatch").c_str(), std::forward<Args>(args)...);
            break;
        }
        case DB_ERR_INIT_FAIL:
        {
            LOG_ERROR(tr("log.db_err_init_fail").c_str(), std::forward<Args>(args)...);
            break;
        }
        case DB_ERR_QUIT_FAIL:
        {
            LOG_ERROR(tr("log.db_err_quit_fail").c_str(), std::forward<Args>(args)...);
            break;
        }

        default: 
        {
            LOG_ERROR(tr("log.unknow_err").c_str(), err.value());
            break;
        }
    }
}

#endif