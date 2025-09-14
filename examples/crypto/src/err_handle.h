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
            LOG_ERROR(tr("log.err_invalid_subcmd").c_str(), std::forward<Args>(args)..., print_str_vector(subcmds));
            break;
        }
        case err_invalid_fmt: 
        {
            LOG_ERROR(tr("log.err_invalid_fmt").c_str(), std::forward<Args>(args)..., print_str_vector(fmts));
            break;
        }

        // encrypt errors
        case err_invalid_input: 
        {
            LOG_ERROR(tr("log.err_invalid_input").c_str(), std::forward<Args>(args)...);
            break;
        }
        case err_invalid_output: 
        {
            LOG_ERROR(tr("log.err_invalid_output").c_str(), std::forward<Args>(args)...);
            break;
        }
        case err_invalid_algo: 
        {
            LOG_ERROR(tr("log.err_invalid_algo").c_str(), std::forward<Args>(args)..., print_str_vector(algos));
            break;
        }
        case err_invalid_key: 
        {
            LOG_ERROR(tr("log.err_invalid_key").c_str(), std::forward<Args>(args)...);
            break;
        }
        case err_invalid_padding: 
        {
            LOG_ERROR(tr("log.err_invalid_padding").c_str(), std::forward<Args>(args)...);
            break;
        }
        case err_invalid_iv: 
        {
            LOG_ERROR(tr("log.err_invalid_iv").c_str(), std::forward<Args>(args)...);
            break;
        }
        case err_encrypt_aes_failed:
        {
            LOG_ERROR(tr("log.err_encrypt_aes_failed").c_str(), std::forward<Args>(args)...);
            break;
        }
        case err_encrypt_base64_failed:
        {
            LOG_ERROR(tr("log.err_encrypt_base64_failed").c_str(), std::forward<Args>(args)...);
            break;
        }
        case err_encrypt_des_failed:
        {
            LOG_ERROR(tr("log.err_encrypt_des_failed").c_str(), std::forward<Args>(args)...);
            break;
        }
        case err_encrypt_md5_failed:
        {
            LOG_ERROR(tr("log.err_encrypt_md5_failed").c_str(), std::forward<Args>(args)...);
            break;
        }
        case err_encrypt_sha256_failed:
        {
            LOG_ERROR(tr("log.err_encrypt_sha256_failed").c_str(), std::forward<Args>(args)...);
            break;
        }
        case err_encrypt_rsa_failed:
        {
            LOG_ERROR(tr("log.err_encrypt_rsa_failed").c_str(), 
                std::forward<Args>(args)...);
            break;
        }

        // decrypt errors
        case err_invalid_decrypt_input: 
        {
            LOG_ERROR(tr("log.err_invalid_decrypt_input").c_str(), std::forward<Args>(args)...);
            break;
        }
        case err_invalid_decrypt_output: 
        {
            LOG_ERROR(tr("log.err_invalid_decrypt_output").c_str(), std::forward<Args>(args)...);
            break;
        }
        case err_invalid_decrypt_algo:
        {
            LOG_ERROR(tr("log.err_invalid_decrypt_algo").c_str(), std::forward<Args>(args)..., print_str_vector(decrypt_algos));
            break;
        }
        case err_invalid_decrypt_key: 
        {
            LOG_ERROR(tr("log.err_invalid_decrypt_key").c_str(), std::forward<Args>(args)...);
            break;
        }
        case err_invalid_decrypt_padding: 
        {
            LOG_ERROR(tr("log.err_invalid_decrypt_padding").c_str(), std::forward<Args>(args)...);
            break;
        }
        case err_invalid_decrypt_iv: 
        {
            LOG_ERROR(tr("log.err_invalid_decrypt_iv").c_str(), std::forward<Args>(args)...);
            break;
        }
        case err_decrypt_aes_failed:
        {
            LOG_ERROR(tr("log.err_decrypt_aes_failed").c_str(), std::forward<Args>(args)...);
            break;
        }
        case err_decrypt_base64_failed:
        {
            LOG_ERROR(tr("log.err_decrypt_base64_failed").c_str(), std::forward<Args>(args)...);
            break;
        }
        case err_decrypt_des_failed:
        {
            LOG_ERROR(tr("log.err_decrypt_des_failed").c_str(), std::forward<Args>(args)...);
            break;
        }
        case err_decrypt_md5_failed:
        {
            LOG_ERROR(tr("log.err_decrypt_md5_failed").c_str(), std::forward<Args>(args)...);
            break;
        }
        case err_decrypt_sha256_failed:
        {
            LOG_ERROR(tr("log.err_decrypt_sha256_failed").c_str(), std::forward<Args>(args)...);
            break;
        }
        case err_decrypt_rsa_failed:
        {
            LOG_ERROR(tr("log.err_decrypt_rsa_failed").c_str(), std::forward<Args>(args)...);
            break;
        }
        case err_keygen_fail:
        {
            LOG_ERROR(tr("log.err_keygen_fail").c_str(), std::forward<Args>(args)...);
            break;
        }
        default: {
            break;
        }
    }
}

#endif