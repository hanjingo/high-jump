/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RSA_HPP
#define RSA_HPP

#include <cstddef>
#include <climits>
#include <cstring>
#include <fstream>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>

#include <openssl/bio.h>
#include <openssl/decoder.h>
#include <openssl/encoder.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>

namespace hj
{

class rsa
{
  public:
    enum class error_code
    {
        ok = 0,
        invalid_input,
        invalid_output,
        invalid_pubkey_pem,
        invalid_prikey_pem,
        invalid_key_bits,
        buffer_too_small,
        key_generation_failed,
        key_loading_failed,
        encryption_failed,
        decryption_failed,
        signing_failed,
        verification_failed,
        not_supported_padding,
        no_padding_specified,
        write_failed,
        read_failed,
        mode_and_password_mismatch,
        unknown,
    };

    enum class key_format
    {
        x509, // X.509 (-----BEGIN PUBLIC KEY-----) (-----BEGIN PRIVATE KEY-----)
        pkcs1, // PKCS#1 (-----BEGIN RSA PUBLIC KEY-----) (-----BEGIN RSA PRIVATE KEY-----)
    };

    enum class pem_cipher
    {
        none,

        aes_128_ecb,
        aes_192_ecb,
        aes_256_ecb,
        aes_128_cbc,
        aes_192_cbc,
        aes_256_cbc,
        aes_128_cfb,
        aes_192_cfb,
        aes_256_cfb,
        aes_128_ofb,
        aes_192_ofb,
        aes_256_ofb,

#ifndef OPENSSL_NO_DES
        des_ede3_ecb,
        des_ede3_cbc,
        des_ede3_cfb,
        des_ede3_ofb,
        des_ede_cbc,
        des_ecb,
        des_cbc,
        des_cfb,
        des_ofb,
#endif

#ifndef OPENSSL_NO_BF
        bf_ecb,
        bf_cbc,
        bf_cfb,
        bf_ofb,
#endif

#ifndef OPENSSL_NO_CAST
        cast5_ecb,
        cast5_cbc,
        cast5_cfb,
        cast5_ofb,
#endif

#ifndef OPENSSL_NO_RC2
        rc2_ecb,
        rc2_cbc,
        rc2_cfb,
        rc2_ofb,
#endif
    };

    // RSA encryption/signature padding is intentionally separated by use.
    enum class padding
    {
        pkcs1,      // RSAES-PKCS1-v1_5 / RSASSA-PKCS1-v1_5
        no_padding, // RSAES raw operation; use only when protocol requires it
        pkcs1_oaep, // RSAES-OAEP

        // WARNING: RSA X9.31 padding is legacy and no longer supported
        x931,

        pkcs1_pss, // RSASSA-PSS; not valid for encryption
    };

    enum class digest
    {
        sha256,
        sha384,
        sha512,
    };

    static constexpr std::size_t rsa_max_key_bits = 4096;

    struct options
    {
        const unsigned char *pubkey_pem     = nullptr;
        std::size_t          pubkey_pem_len = 0;
        const unsigned char *prikey_pem     = nullptr;
        std::size_t          prikey_pem_len = 0;
        const unsigned char *password       = nullptr;
        std::size_t          password_len   = 0;
        padding              pad_style      = padding::pkcs1_oaep;

        // OAEP and signature digest. SHA-256 is the modern default.
        digest digest_type = digest::sha256;

        options() = default;

        options(const unsigned char *pubkey,
                std::size_t          pubkey_len,
                const unsigned char *prikey     = nullptr,
                std::size_t          prikey_len = 0,
                const unsigned char *pwd        = nullptr,
                std::size_t          pwd_len    = 0,
                padding              pad        = padding::pkcs1_oaep,
                digest               dgst       = digest::sha256)
            : pubkey_pem(pubkey)
            , pubkey_pem_len(pubkey_len)
            , prikey_pem(prikey)
            , prikey_pem_len(prikey_len)
            , password(pwd)
            , password_len(pwd_len)
            , pad_style(pad)
            , digest_type(dgst)
        {
        }

        void reset()
        {
            pubkey_pem     = nullptr;
            pubkey_pem_len = 0;
            prikey_pem     = nullptr;
            prikey_pem_len = 0;
            password       = nullptr;
            password_len   = 0;
            pad_style      = padding::pkcs1_oaep;
            digest_type    = digest::sha256;
        }
    };

    struct keygen_options
    {
        std::size_t          bits         = 2048;
        key_format           format       = key_format::x509;
        pem_cipher           cipher       = pem_cipher::none;
        const unsigned char *password     = nullptr;
        std::size_t          password_len = 0;

        keygen_options() = default;

        keygen_options(std::size_t          kbits,
                       key_format           fmt,
                       pem_cipher           m       = pem_cipher::none,
                       const unsigned char *pwd     = nullptr,
                       std::size_t          pwd_len = 0)
            : bits(kbits)
            , format(fmt)
            , cipher(m)
            , password(pwd)
            , password_len(pwd_len)
        {
        }

        void reset()
        {
            bits         = 2048;
            format       = key_format::x509;
            cipher       = pem_cipher::none;
            password     = nullptr;
            password_len = 0;
        }
    };

    // ---------------------------------------------------------------------
    // RSA encryption
    // ---------------------------------------------------------------------

    static error_code encrypt(unsigned char       *dst,
                              std::size_t         &dst_len,
                              const unsigned char *src,
                              std::size_t          src_len,
                              const options       &opt)
    {
        if(!dst)
            return error_code::invalid_output;
        if(!src && src_len != 0)
            return error_code::invalid_input;
        if(!opt.pubkey_pem || opt.pubkey_pem_len == 0)
            return error_code::invalid_pubkey_pem;
        if(!_is_encryption_padding(opt.pad_style))
            return error_code::not_supported_padding;

        EVP_PKEY *pkey = _load_public_key(opt.pubkey_pem, opt.pubkey_pem_len);
        if(!pkey)
            return error_code::key_loading_failed;

        EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pkey, nullptr);
        if(!ctx)
        {
            EVP_PKEY_free(pkey);
            return error_code::encryption_failed;
        }

        error_code ec = error_code::encryption_failed;
        do
        {
            if(EVP_PKEY_encrypt_init(ctx) <= 0)
                break;
            if(!_configure_encryption(ctx, opt.pad_style, opt.digest_type))
            {
                ec = error_code::not_supported_padding;
                break;
            }

            if(!_is_plain_valid(src_len, opt.pad_style, pkey, opt.digest_type))
            {
                ec = error_code::invalid_input;
                break;
            }

            std::size_t required = 0;
            if(EVP_PKEY_encrypt(ctx, nullptr, &required, src, src_len) <= 0)
                break;
            if(dst_len < required)
            {
                dst_len = required;
                ec      = error_code::buffer_too_small;
                break;
            }

            std::size_t out_len = dst_len;
            if(EVP_PKEY_encrypt(ctx, dst, &out_len, src, src_len) <= 0)
                break;

            dst_len = out_len;
            ec      = error_code::ok;
        } while(false);

        if(ec != error_code::ok && dst && dst_len > 0)
            std::memset(dst, 0, dst_len);

        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return ec;
    }

    static error_code
    encrypt(std::string &dst, const std::string &src, const options &opt)
    {
        const std::size_t reserve =
            encrypt_len_reserve(opt.pubkey_pem, opt.pubkey_pem_len);
        if(reserve == 0)
        {
            dst.clear();
            return error_code::key_loading_failed;
        }

        dst.resize(reserve);
        std::size_t          dst_len = dst.size();
        const unsigned char *src_ptr =
            reinterpret_cast<const unsigned char *>(src.data());

        error_code ec = encrypt(reinterpret_cast<unsigned char *>(dst.data()),
                                dst_len,
                                src_ptr,
                                src.size(),
                                opt);
        if(ec != error_code::ok)
        {
            dst.clear();
            return ec;
        }

        dst.resize(dst_len);
        return error_code::ok;
    }

    // RSA block encryption for protocol compatibility; not intended for bulk data encryption.
    static error_code
    encrypt(std::ostream &out, std::istream &in, const options &opt)
    {
        if(!in)
            return error_code::invalid_input;
        if(!out)
            return error_code::invalid_output;
        if(!opt.pubkey_pem || opt.pubkey_pem_len == 0)
            return error_code::invalid_pubkey_pem;
        if(!_is_encryption_padding(opt.pad_style))
            return error_code::not_supported_padding;

        EVP_PKEY *pkey = _load_public_key(opt.pubkey_pem, opt.pubkey_pem_len);
        if(!pkey)
            return error_code::key_loading_failed;

        std::size_t key_size = _key_size(pkey);
        if(key_size == 0)
        {
            EVP_PKEY_free(pkey);
            return error_code::key_loading_failed;
        }

        const std::size_t max_chunk =
            _max_encrypt_plaintext(key_size, opt.pad_style, opt.digest_type);
        if(max_chunk == 0)
        {
            EVP_PKEY_free(pkey);
            return error_code::not_supported_padding;
        }

        // No-padding is a block operation. Other paddings are chunked at the
        // maximum plaintext size accepted by RSAES.
        std::string inbuf(max_chunk, '\0');
        std::string outbuf(key_size, '\0');

        // Create a new context for each chunk to avoid state issues with padding and digest.
        EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pkey, nullptr);
        if(!ctx)
        {
            EVP_PKEY_free(pkey);
            return error_code::encryption_failed;
        }

        bool ok = EVP_PKEY_encrypt_init(ctx) > 0
                  && _configure_encryption(ctx, opt.pad_style, opt.digest_type);
        std::size_t out_len = outbuf.size();
        if(!ok)
        {
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            return error_code::encryption_failed;
        }

        while(in)
        {
            in.read(inbuf.data(), static_cast<std::streamsize>(inbuf.size()));
            const std::streamsize read_len = in.gcount();
            if(read_len <= 0)
                break;

            if(opt.pad_style == padding::no_padding
               && static_cast<std::size_t>(read_len) != key_size)
            {
                EVP_PKEY_CTX_free(ctx);
                EVP_PKEY_free(pkey);
                return error_code::no_padding_specified;
            }

            ok = EVP_PKEY_encrypt(
                     ctx,
                     reinterpret_cast<unsigned char *>(outbuf.data()),
                     &out_len,
                     reinterpret_cast<const unsigned char *>(inbuf.data()),
                     static_cast<std::size_t>(read_len))
                 > 0;
            if(!ok)
            {
                EVP_PKEY_CTX_free(ctx);
                EVP_PKEY_free(pkey);
                return error_code::encryption_failed;
            }

            out.write(outbuf.data(), static_cast<std::streamsize>(out_len));
            if(!out)
            {
                EVP_PKEY_CTX_free(ctx);
                EVP_PKEY_free(pkey);
                return error_code::write_failed;
            }
        }

        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        if(in.bad())
            return error_code::read_failed;

        return error_code::ok;
    }

    static error_code encrypt_file(const char    *dst_file_path,
                                   const char    *src_file_path,
                                   const options &opt)
    {
        if(!dst_file_path || !src_file_path)
            return error_code::invalid_input;

        std::ifstream in(src_file_path, std::ios::binary);
        if(!in)
            return error_code::read_failed;
        std::ofstream out(dst_file_path, std::ios::binary | std::ios::trunc);
        if(!out)
            return error_code::write_failed;
        return encrypt(out, in, opt);
    }

    static error_code encrypt_file(const std::string &dst_file_path,
                                   const std::string &src_file_path,
                                   const options     &opt)
    {
        return encrypt_file(dst_file_path.c_str(), src_file_path.c_str(), opt);
    }

    // ---------------------------------------------------------------------
    // RSA decryption
    // ---------------------------------------------------------------------

    static error_code decrypt(unsigned char       *dst,
                              std::size_t         &dst_len,
                              const unsigned char *src,
                              std::size_t          src_len,
                              const options       &opt)
    {
        if(!dst)
            return error_code::invalid_output;
        if(!src || src_len == 0)
            return error_code::invalid_input;
        if(!opt.prikey_pem || opt.prikey_pem_len == 0)
            return error_code::invalid_prikey_pem;
        if(!_is_encryption_padding(opt.pad_style))
            return error_code::not_supported_padding;

        EVP_PKEY *pkey = _load_private_key(opt.prikey_pem,
                                           opt.prikey_pem_len,
                                           opt.password,
                                           opt.password_len);
        if(!pkey)
            return error_code::key_loading_failed;

        const std::size_t key_size = _key_size(pkey);
        if(key_size == 0 || src_len != key_size)
        {
            EVP_PKEY_free(pkey);
            return error_code::invalid_input;
        }

        EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pkey, nullptr);
        if(!ctx)
        {
            EVP_PKEY_free(pkey);
            return error_code::decryption_failed;
        }

        error_code ec = error_code::decryption_failed;
        do
        {
            if(EVP_PKEY_decrypt_init(ctx) <= 0)
                break;

            if(!_configure_encryption(ctx, opt.pad_style, opt.digest_type))
            {
                ec = error_code::not_supported_padding;
                break;
            }

            std::size_t required = 0;
            if(EVP_PKEY_decrypt(ctx, nullptr, &required, src, src_len) <= 0)
                break;

            if(dst_len < required)
            {
                dst_len = required;
                ec      = error_code::buffer_too_small;
                break;
            }

            std::size_t out_len = dst_len;
            if(EVP_PKEY_decrypt(ctx, dst, &out_len, src, src_len) <= 0)
                break;

            dst_len = out_len;
            ec      = error_code::ok;
        } while(false);

        if(ec != error_code::ok && dst && dst_len > 0)
            std::memset(dst, 0, dst_len);

        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return ec;
    }

    static error_code
    decrypt(std::string &dst, const std::string &src, const options &opt)
    {
        const std::size_t reserve = decrypt_len_reserve(opt.prikey_pem,
                                                        opt.prikey_pem_len,
                                                        opt.password,
                                                        opt.password_len);
        if(reserve == 0)
        {
            dst.clear();
            return error_code::key_loading_failed;
        }

        dst.resize(reserve);
        std::size_t dst_len = dst.size();
        error_code  ec =
            decrypt(reinterpret_cast<unsigned char *>(dst.data()),
                    dst_len,
                    reinterpret_cast<const unsigned char *>(src.data()),
                    src.size(),
                    opt);
        if(ec != error_code::ok)
        {
            dst.clear();
            return ec;
        }

        dst.resize(dst_len);
        return error_code::ok;
    }

    static error_code
    decrypt(std::ostream &out, std::istream &in, const options &opt)
    {
        if(!in)
            return error_code::invalid_input;
        if(!out)
            return error_code::invalid_output;
        if(!opt.prikey_pem || opt.prikey_pem_len == 0)
            return error_code::invalid_prikey_pem;
        if(!_is_encryption_padding(opt.pad_style))
            return error_code::not_supported_padding;

        EVP_PKEY *pkey = _load_private_key(opt.prikey_pem,
                                           opt.prikey_pem_len,
                                           opt.password,
                                           opt.password_len);
        if(!pkey)
            return error_code::key_loading_failed;

        const std::size_t key_size = _key_size(pkey);
        if(key_size == 0)
        {
            EVP_PKEY_free(pkey);
            return error_code::key_loading_failed;
        }

        std::string inbuf(key_size, '\0');
        std::string outbuf(key_size, '\0');

        // Create a new context for each chunk to avoid state issues with padding and digest.
        EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pkey, nullptr);
        if(!ctx)
        {
            EVP_PKEY_free(pkey);
            return error_code::decryption_failed;
        }

        bool ok = EVP_PKEY_decrypt_init(ctx) > 0
                  && _configure_encryption(ctx, opt.pad_style, opt.digest_type);
        std::size_t out_len = outbuf.size();
        if(!ok)
        {
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            return error_code::decryption_failed;
        }

        while(in)
        {
            in.read(inbuf.data(), static_cast<std::streamsize>(key_size));
            const std::streamsize read_len = in.gcount();
            if(read_len <= 0)
                break;

            if(static_cast<std::size_t>(read_len) != key_size)
            {
                EVP_PKEY_CTX_free(ctx);
                EVP_PKEY_free(pkey);
                return error_code::invalid_input;
            }

            ok = EVP_PKEY_decrypt(
                     ctx,
                     reinterpret_cast<unsigned char *>(outbuf.data()),
                     &out_len,
                     reinterpret_cast<const unsigned char *>(inbuf.data()),
                     key_size)
                 > 0;
            if(!ok)
            {
                EVP_PKEY_CTX_free(ctx);
                EVP_PKEY_free(pkey);
                return error_code::decryption_failed;
            }

            out.write(outbuf.data(), static_cast<std::streamsize>(out_len));
            if(!out)
            {
                EVP_PKEY_CTX_free(ctx);
                EVP_PKEY_free(pkey);
                return error_code::write_failed;
            }
        }

        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        if(in.bad())
            return error_code::read_failed;

        return error_code::ok;
    }

    static error_code decrypt_file(const char    *dst_file_path,
                                   const char    *src_file_path,
                                   const options &opt)
    {
        if(!dst_file_path || !src_file_path)
            return error_code::invalid_input;

        std::ifstream in(src_file_path, std::ios::binary);
        if(!in)
            return error_code::read_failed;
        std::ofstream out(dst_file_path, std::ios::binary | std::ios::trunc);
        if(!out)
            return error_code::write_failed;
        return decrypt(out, in, opt);
    }

    static error_code decrypt_file(const std::string &dst_file_path,
                                   const std::string &src_file_path,
                                   const options     &opt)
    {
        return decrypt_file(dst_file_path.c_str(), src_file_path.c_str(), opt);
    }

    // ---------------------------------------------------------------------
    // Proper RSA signatures: RSASSA-PKCS1-v1_5 / RSASSA-PSS
    // ---------------------------------------------------------------------

    static error_code sign(unsigned char       *signature,
                           std::size_t         &signature_len,
                           const unsigned char *message,
                           std::size_t          message_len,
                           const options       &opt)
    {
        if(!signature)
            return error_code::invalid_output;
        if(!message && message_len != 0)
            return error_code::invalid_input;
        if(!opt.prikey_pem || opt.prikey_pem_len == 0)
            return error_code::invalid_prikey_pem;
        if(!_is_signature_padding(opt.pad_style))
            return error_code::not_supported_padding;

        EVP_PKEY *pkey = _load_private_key(opt.prikey_pem,
                                           opt.prikey_pem_len,
                                           opt.password,
                                           opt.password_len);
        if(!pkey)
            return error_code::key_loading_failed;

        EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
        if(!mdctx)
        {
            EVP_PKEY_free(pkey);
            return error_code::signing_failed;
        }

        EVP_PKEY_CTX *pctx = nullptr;
        error_code    ec   = error_code::signing_failed;
        do
        {
            if(EVP_DigestSignInit(mdctx,
                                  &pctx,
                                  _select_digest(opt.digest_type),
                                  nullptr,
                                  pkey)
               <= 0)
                break;

            if(!_configure_signature(pctx, opt.pad_style, opt.digest_type))
            {
                ec = error_code::not_supported_padding;
                break;
            }

            if(EVP_DigestSignUpdate(mdctx, message, message_len) <= 0)
                break;

            std::size_t required = 0;
            if(EVP_DigestSignFinal(mdctx, nullptr, &required) <= 0)
                break;
            if(signature_len < required)
            {
                signature_len = required;
                ec            = error_code::buffer_too_small;
                break;
            }

            if(EVP_DigestSignFinal(mdctx, signature, &signature_len) <= 0)
                break;

            ec = error_code::ok;
        } while(false);

        if(ec != error_code::ok && signature && signature_len > 0)
            std::memset(signature, 0, signature_len);

        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        return ec;
    }

    static error_code
    sign(std::string &signature, const std::string &message, const options &opt)
    {
        const std::size_t reserve = signature_len_reserve(opt.prikey_pem,
                                                          opt.prikey_pem_len,
                                                          opt.password,
                                                          opt.password_len);
        if(reserve == 0)
        {
            signature.clear();
            return error_code::key_loading_failed;
        }

        signature.resize(reserve);
        std::size_t signature_len = signature.size();
        error_code  ec =
            sign(reinterpret_cast<unsigned char *>(signature.data()),
                 signature_len,
                 reinterpret_cast<const unsigned char *>(message.data()),
                 message.size(),
                 opt);
        if(ec != error_code::ok)
        {
            signature.clear();
            return ec;
        }

        signature.resize(signature_len);
        return error_code::ok;
    }

    static error_code verify(const unsigned char *signature,
                             std::size_t          signature_len,
                             const unsigned char *message,
                             std::size_t          message_len,
                             const options       &opt)
    {
        if(!signature || signature_len == 0)
            return error_code::invalid_input;
        if(!message && message_len != 0)
            return error_code::invalid_input;
        if(!opt.pubkey_pem || opt.pubkey_pem_len == 0)
            return error_code::invalid_pubkey_pem;
        if(!_is_signature_padding(opt.pad_style))
            return error_code::not_supported_padding;

        EVP_PKEY *pkey = _load_public_key(opt.pubkey_pem, opt.pubkey_pem_len);
        if(!pkey)
            return error_code::key_loading_failed;

        EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
        if(!mdctx)
        {
            EVP_PKEY_free(pkey);
            return error_code::verification_failed;
        }

        EVP_PKEY_CTX *pctx = nullptr;
        error_code    ec   = error_code::verification_failed;
        do
        {
            if(EVP_DigestVerifyInit(mdctx,
                                    &pctx,
                                    _select_digest(opt.digest_type),
                                    nullptr,
                                    pkey)
               <= 0)
                break;

            if(!_configure_signature(pctx, opt.pad_style, opt.digest_type))
            {
                ec = error_code::not_supported_padding;
                break;
            }

            if(EVP_DigestVerifyUpdate(mdctx, message, message_len) <= 0)
                break;

            const int rc =
                EVP_DigestVerifyFinal(mdctx, signature, signature_len);
            if(rc == 1)
                ec = error_code::ok;
            else if(rc == 0)
                ec = error_code::verification_failed;
        } while(false);

        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        return ec;
    }

    static error_code verify(const std::string &signature,
                             const std::string &message,
                             const options     &opt)
    {
        return verify(reinterpret_cast<const unsigned char *>(signature.data()),
                      signature.size(),
                      reinterpret_cast<const unsigned char *>(message.data()),
                      message.size(),
                      opt);
    }

    // Kept as a migration-friendly name. Unlike the old implementation,
    // this is real RSASSA verification and does not perform raw RSA decrypt.
    static error_code verify_signature(const std::string &signature,
                                       const std::string &message,
                                       const options     &opt)
    {
        return verify(signature, message, opt);
    }

    // ---------------------------------------------------------------------
    // Key generation / serialization
    // ---------------------------------------------------------------------

    static error_code keygen(unsigned char        *pubkey_pem,
                             std::size_t          &pubkey_pem_len,
                             unsigned char        *prikey_pem,
                             std::size_t          &prikey_pem_len,
                             const keygen_options &opt)
    {
        if(!pubkey_pem || pubkey_pem_len == 0)
            return error_code::invalid_pubkey_pem;
        if(!prikey_pem || prikey_pem_len == 0)
            return error_code::invalid_prikey_pem;
        if(!is_key_pair_bits_valid(opt.bits))
            return error_code::invalid_key_bits;
        if(opt.cipher != pem_cipher::none
           && (!opt.password
               || !_validate_password_for_cipher(opt.cipher, opt.password_len)))
            return error_code::mode_and_password_mismatch;

        EVP_PKEY_CTX *keygen_ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
        if(!keygen_ctx)
            return error_code::key_generation_failed;

        EVP_PKEY  *pkey = nullptr;
        error_code ret  = error_code::key_generation_failed;
        do
        {
            if(EVP_PKEY_keygen_init(keygen_ctx) <= 0)
                break;
            if(EVP_PKEY_CTX_set_rsa_keygen_bits(keygen_ctx,
                                                static_cast<int>(opt.bits))
               <= 0)
                break;
            if(EVP_PKEY_keygen(keygen_ctx, &pkey) <= 0 || !pkey)
                break;

            BIO *pub_bio = BIO_new(BIO_s_mem());
            BIO *pri_bio = BIO_new(BIO_s_mem());
            if(!pub_bio || !pri_bio)
            {
                if(pub_bio)
                    BIO_free(pub_bio);
                if(pri_bio)
                    BIO_free(pri_bio);
                break;
            }

            bool pub_ok = _encode_public_key(pub_bio, pkey, opt.format);
            bool pri_ok = _encode_private_key(pri_bio,
                                              pkey,
                                              opt.format,
                                              opt.cipher,
                                              opt.password,
                                              opt.password_len);

            if(!pub_ok || !pri_ok)
            {
                BIO_free(pub_bio);
                BIO_free(pri_bio);
                break;
            }

            char      *pub_data = nullptr;
            char      *pri_data = nullptr;
            const long pub_len  = BIO_get_mem_data(pub_bio, &pub_data);
            const long pri_len  = BIO_get_mem_data(pri_bio, &pri_data);
            if(pub_len <= 0 || pri_len <= 0
               || static_cast<std::size_t>(pub_len) > pubkey_pem_len
               || static_cast<std::size_t>(pri_len) > prikey_pem_len)
            {
                BIO_free(pub_bio);
                BIO_free(pri_bio);
                ret = error_code::buffer_too_small;
                break;
            }

            std::memcpy(pubkey_pem,
                        pub_data,
                        static_cast<std::size_t>(pub_len));
            std::memcpy(prikey_pem,
                        pri_data,
                        static_cast<std::size_t>(pri_len));
            pubkey_pem_len = static_cast<std::size_t>(pub_len);
            prikey_pem_len = static_cast<std::size_t>(pri_len);
            ret            = error_code::ok;

            BIO_free(pub_bio);
            BIO_free(pri_bio);
        } while(false);

        EVP_PKEY_free(pkey);
        EVP_PKEY_CTX_free(keygen_ctx);
        return ret;
    }

    static error_code keygen(std::string          &pubkey_pem,
                             std::string          &prikey_pem,
                             const keygen_options &opt)
    {
        // PEM is larger than the RSA modulus and depends on the selected
        // encoding/cipher. Start generously and shrink after encoding.
        pubkey_pem.resize(rsa_max_key_bits);
        prikey_pem.resize(rsa_max_key_bits * 2);
        std::size_t pub_len = pubkey_pem.size();
        std::size_t pri_len = prikey_pem.size();

        error_code ec =
            keygen(reinterpret_cast<unsigned char *>(pubkey_pem.data()),
                   pub_len,
                   reinterpret_cast<unsigned char *>(prikey_pem.data()),
                   pri_len,
                   opt);
        if(ec != error_code::ok)
        {
            pubkey_pem.clear();
            prikey_pem.clear();
            return ec;
        }

        pubkey_pem.resize(pub_len);
        prikey_pem.resize(pri_len);
        return error_code::ok;
    }

    // ---------------------------------------------------------------------
    // Capacity / validation helpers
    // ---------------------------------------------------------------------

    static std::size_t encrypt_len_reserve(const unsigned char *pubkey_pem,
                                           std::size_t          pubkey_pem_len)
    {
        EVP_PKEY *pkey = _load_public_key(pubkey_pem, pubkey_pem_len);
        if(!pkey)
            return 0;
        const std::size_t size = _key_size(pkey);
        EVP_PKEY_free(pkey);
        return size;
    }

    static std::size_t
    decrypt_len_reserve(const unsigned char *prikey_pem,
                        std::size_t          prikey_pem_len,
                        const unsigned char *password     = nullptr,
                        std::size_t          password_len = 0)
    {
        EVP_PKEY *pkey = _load_private_key(prikey_pem,
                                           prikey_pem_len,
                                           password,
                                           password_len);
        if(!pkey)
            return 0;
        const std::size_t size = _key_size(pkey);
        EVP_PKEY_free(pkey);
        return size;
    }

    static std::size_t
    signature_len_reserve(const unsigned char *prikey_pem,
                          std::size_t          prikey_pem_len,
                          const unsigned char *password     = nullptr,
                          std::size_t          password_len = 0)
    {
        return decrypt_len_reserve(prikey_pem,
                                   prikey_pem_len,
                                   password,
                                   password_len);
    }

    static bool is_pubkey_valid(const unsigned char *pubkey_pem,
                                std::size_t          pubkey_pem_len,
                                padding pad_style = padding::pkcs1_oaep)
    {
        EVP_PKEY *pkey = _load_public_key(pubkey_pem, pubkey_pem_len);
        if(!pkey)
            return false;

        const bool valid = _is_rsa_key(pkey)
                           && (_is_encryption_padding(pad_style)
                               || _is_signature_padding(pad_style));
        EVP_PKEY_free(pkey);
        return valid;
    }

    static bool is_prikey_valid(const unsigned char *prikey_pem,
                                std::size_t          prikey_pem_len,
                                padding pad_style = padding::pkcs1_oaep,
                                const unsigned char *password     = nullptr,
                                std::size_t          password_len = 0)
    {
        EVP_PKEY *pkey = _load_private_key(prikey_pem,
                                           prikey_pem_len,
                                           password,
                                           password_len);
        if(!pkey)
            return false;

        const bool valid = _is_rsa_key(pkey)
                           && (_is_encryption_padding(pad_style)
                               || _is_signature_padding(pad_style));
        EVP_PKEY_free(pkey);
        return valid;
    }

    static bool is_key_pair_bits_valid(std::size_t bits)
    {
        // 512/1024-bit RSA are intentionally not generated by this modern
        // wrapper. Existing old keys may still be loaded for compatibility.
        return bits == 2048 || bits == 3072 || bits == 4096;
    }

    static bool is_plain_valid(std::size_t          plain_len,
                               padding              pad_style,
                               const unsigned char *pubkey_pem,
                               std::size_t          pubkey_pem_len,
                               digest digest_type = digest::sha256)
    {
        EVP_PKEY *pkey = _load_public_key(pubkey_pem, pubkey_pem_len);
        if(!pkey)
            return false;
        const bool valid =
            _is_plain_valid(plain_len, pad_style, pkey, digest_type);
        EVP_PKEY_free(pkey);
        return valid;
    }

    static bool is_plain_valid(std::istream        &in,
                               padding              pad_style,
                               const unsigned char *pubkey_pem,
                               std::size_t          pubkey_pem_len,
                               digest digest_type = digest::sha256)
    {
        EVP_PKEY *pkey = _load_public_key(pubkey_pem, pubkey_pem_len);
        if(!pkey)
            return false;
        const std::size_t key_size = _key_size(pkey);
        EVP_PKEY_free(pkey);
        return key_size != 0
               && _is_stream_valid(in, pad_style, key_size, digest_type, true);
    }

    static bool is_cipher_valid(std::istream        &in,
                                padding              pad_style,
                                const unsigned char *prikey_pem,
                                std::size_t          prikey_pem_len,
                                const unsigned char *password     = nullptr,
                                std::size_t          password_len = 0)
    {
        EVP_PKEY *pkey = _load_private_key(prikey_pem,
                                           prikey_pem_len,
                                           password,
                                           password_len);
        if(!pkey)
            return false;
        const std::size_t key_size = _key_size(pkey);
        EVP_PKEY_free(pkey);
        return key_size != 0
               && _is_stream_valid(in,
                                   pad_style,
                                   key_size,
                                   digest::sha256,
                                   false);
    }

    static bool is_cipher_valid(const std::string &ciphertext,
                                padding            pad_style,
                                const std::string &prikey_pem,
                                const std::string &password = std::string())
    {
        std::istringstream in(ciphertext);
        return is_cipher_valid(
            in,
            pad_style,
            reinterpret_cast<const unsigned char *>(prikey_pem.data()),
            prikey_pem.size(),
            password.empty()
                ? nullptr
                : reinterpret_cast<const unsigned char *>(password.data()),
            password.size());
    }

    static bool is_cipher_valid(const unsigned char *ciphertext,
                                std::size_t          ciphertext_len,
                                padding              pad_style,
                                const unsigned char *prikey_pem,
                                std::size_t          prikey_pem_len,
                                const unsigned char *password     = nullptr,
                                std::size_t          password_len = 0)
    {
        if(!ciphertext && ciphertext_len != 0)
            return false;
        std::istringstream in(
            std::string(reinterpret_cast<const char *>(ciphertext),
                        ciphertext_len));
        return is_cipher_valid(in,
                               pad_style,
                               prikey_pem,
                               prikey_pem_len,
                               password,
                               password_len);
    }

  private:
    static bool _is_rsa_key(const EVP_PKEY *pkey)
    {
        return pkey && EVP_PKEY_is_a(pkey, "RSA");
    }

    static std::size_t _key_size(const EVP_PKEY *pkey)
    {
        if(!_is_rsa_key(pkey))
            return 0;
        const int size = EVP_PKEY_get_size(pkey);
        return size > 0 ? static_cast<std::size_t>(size) : 0;
    }

    static EVP_PKEY *_load_public_key(const unsigned char *pem,
                                      std::size_t          pem_len)
    {
        if(!pem || pem_len == 0 || pem_len > static_cast<std::size_t>(INT_MAX))
            return nullptr;

        BIO *bio = BIO_new_mem_buf(pem, static_cast<int>(pem_len));
        if(!bio)
            return nullptr;

        EVP_PKEY         *pkey = nullptr;
        OSSL_DECODER_CTX *ctx =
            OSSL_DECODER_CTX_new_for_pkey(&pkey,
                                          "PEM",
                                          nullptr,
                                          "RSA",
                                          EVP_PKEY_PUBLIC_KEY,
                                          nullptr,
                                          nullptr);
        if(ctx)
            (void) OSSL_DECODER_from_bio(ctx, bio);
        OSSL_DECODER_CTX_free(ctx);
        BIO_free(bio);

        if(!_is_rsa_key(pkey))
        {
            EVP_PKEY_free(pkey);
            return nullptr;
        }
        return pkey;
    }

    static EVP_PKEY *_load_private_key(const unsigned char *pem,
                                       std::size_t          pem_len,
                                       const unsigned char *password = nullptr,
                                       std::size_t          password_len = 0)
    {
        if(!pem || pem_len == 0 || pem_len > static_cast<std::size_t>(INT_MAX))
            return nullptr;

        BIO *bio = BIO_new_mem_buf(pem, static_cast<int>(pem_len));
        if(!bio)
            return nullptr;

        EVP_PKEY         *pkey = nullptr;
        OSSL_DECODER_CTX *ctx  = OSSL_DECODER_CTX_new_for_pkey(&pkey,
                                                               "PEM",
                                                               nullptr,
                                                               "RSA",
                                                               EVP_PKEY_KEYPAIR,
                                                               nullptr,
                                                               nullptr);
        if(ctx)
        {
            if(password && password_len > 0)
                (void) OSSL_DECODER_CTX_set_passphrase(ctx,
                                                       password,
                                                       password_len);
            (void) OSSL_DECODER_from_bio(ctx, bio);
        }
        OSSL_DECODER_CTX_free(ctx);
        BIO_free(bio);

        if(!_is_rsa_key(pkey))
        {
            EVP_PKEY_free(pkey);
            return nullptr;
        }
        return pkey;
    }

    static const EVP_MD *_select_digest(digest type)
    {
        switch(type)
        {
            case digest::sha256:
                return EVP_sha256();
            case digest::sha384:
                return EVP_sha384();
            case digest::sha512:
                return EVP_sha512();
            default:
                return nullptr;
        }
    }

    static const char *_select_cipher_name(pem_cipher cipher)
    {
        switch(cipher)
        {
            case pem_cipher::none:
                return nullptr;
            case pem_cipher::aes_128_ecb:
                return "AES-128-ECB";
            case pem_cipher::aes_192_ecb:
                return "AES-192-ECB";
            case pem_cipher::aes_256_ecb:
                return "AES-256-ECB";
            case pem_cipher::aes_128_cbc:
                return "AES-128-CBC";
            case pem_cipher::aes_192_cbc:
                return "AES-192-CBC";
            case pem_cipher::aes_256_cbc:
                return "AES-256-CBC";
            case pem_cipher::aes_128_cfb:
                return "AES-128-CFB";
            case pem_cipher::aes_192_cfb:
                return "AES-192-CFB";
            case pem_cipher::aes_256_cfb:
                return "AES-256-CFB";
            case pem_cipher::aes_128_ofb:
                return "AES-128-OFB";
            case pem_cipher::aes_192_ofb:
                return "AES-192-OFB";
            case pem_cipher::aes_256_ofb:
                return "AES-256-OFB";
#ifndef OPENSSL_NO_DES
            case pem_cipher::des_ede3_ecb:
                return "DES-EDE3-ECB";
            case pem_cipher::des_ede3_cbc:
                return "DES-EDE3-CBC";
            case pem_cipher::des_ede3_cfb:
                return "DES-EDE3-CFB";
            case pem_cipher::des_ede3_ofb:
                return "DES-EDE3-OFB";
            case pem_cipher::des_ede_cbc:
                return "DES-EDE-CBC";
            case pem_cipher::des_ecb:
                return "DES-ECB";
            case pem_cipher::des_cbc:
                return "DES-CBC";
            case pem_cipher::des_cfb:
                return "DES-CFB";
            case pem_cipher::des_ofb:
                return "DES-OFB";
#endif
#ifndef OPENSSL_NO_BF
            case pem_cipher::bf_ecb:
                return "BF-ECB";
            case pem_cipher::bf_cbc:
                return "BF-CBC";
            case pem_cipher::bf_cfb:
                return "BF-CFB";
            case pem_cipher::bf_ofb:
                return "BF-OFB";
#endif
#ifndef OPENSSL_NO_CAST
            case pem_cipher::cast5_ecb:
                return "CAST5-ECB";
            case pem_cipher::cast5_cbc:
                return "CAST5-CBC";
            case pem_cipher::cast5_cfb:
                return "CAST5-CFB";
            case pem_cipher::cast5_ofb:
                return "CAST5-OFB";
#endif
#ifndef OPENSSL_NO_RC2
            case pem_cipher::rc2_ecb:
                return "RC2-ECB";
            case pem_cipher::rc2_cbc:
                return "RC2-CBC";
            case pem_cipher::rc2_cfb:
                return "RC2-CFB";
            case pem_cipher::rc2_ofb:
                return "RC2-OFB";
#endif
            default:
                return nullptr;
        }
    }

    static bool _validate_password_for_cipher(pem_cipher  cipher,
                                              std::size_t password_len)
    {
        if(cipher == pem_cipher::none)
            return true;

        // PEM encryption is KDF based; these minimums are policy checks, not
        // cipher key sizes. Eight bytes preserves the old public contract.
        return password_len >= 8;
    }

    static bool _is_encryption_padding(padding pad)
    {
        return pad == padding::pkcs1 || pad == padding::pkcs1_oaep
               || pad == padding::no_padding;
    }

    static bool _is_signature_padding(padding pad)
    {
        return pad == padding::pkcs1 || pad == padding::pkcs1_pss;
    }

    static bool
    _configure_encryption(EVP_PKEY_CTX *ctx, padding pad, digest digest_type)
    {
        if(!ctx)
            return false;

        int rsa_padding = 0;
        switch(pad)
        {
            case padding::pkcs1:
                rsa_padding = RSA_PKCS1_PADDING;
                break;
            case padding::pkcs1_oaep:
                rsa_padding = RSA_PKCS1_OAEP_PADDING;
                break;
            case padding::no_padding:
                rsa_padding = RSA_NO_PADDING;
                break;
            default:
                return false;
        }

        if(EVP_PKEY_CTX_set_rsa_padding(ctx, rsa_padding) <= 0)
            return false;

        if(pad == padding::pkcs1_oaep)
        {
            const EVP_MD *md = _select_digest(digest_type);
            if(!md)
                return false;
            if(EVP_PKEY_CTX_set_rsa_oaep_md(ctx, md) <= 0)
                return false;
            if(EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, md) <= 0)
                return false;
        }
        return true;
    }

    static bool
    _configure_signature(EVP_PKEY_CTX *ctx, padding pad, digest digest_type)
    {
        if(!ctx)
            return false;

        const EVP_MD *md = _select_digest(digest_type);
        if(!md)
            return false;

        if(pad == padding::pkcs1)
            return EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) > 0;

        if(pad == padding::pkcs1_pss)
        {
            if(EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PSS_PADDING) <= 0)
                return false;
            if(EVP_PKEY_CTX_set_rsa_pss_saltlen(ctx, -1) <= 0)
                return false;
            if(EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, md) <= 0)
                return false;
            return true;
        }

        return false;
    }

    static std::size_t _max_encrypt_plaintext(std::size_t key_size,
                                              padding     pad,
                                              digest      digest_type)
    {
        if(key_size == 0)
            return 0;

        switch(pad)
        {
            case padding::pkcs1:
                return key_size > 11 ? key_size - 11 : 0;
            case padding::pkcs1_oaep: {
                const EVP_MD *md = _select_digest(digest_type);
                if(!md)
                    return 0;
                const int hlen = EVP_MD_get_size(md);
                if(hlen <= 0
                   || key_size <= static_cast<std::size_t>(2 * hlen + 2))
                    return 0;
                return key_size - static_cast<std::size_t>(2 * hlen + 2);
            }
            case padding::no_padding:
                return key_size;
            default:
                return 0;
        }
    }

    static bool _is_plain_valid(std::size_t     plain_len,
                                padding         pad,
                                const EVP_PKEY *pkey,
                                digest          digest_type)
    {
        if(!_is_rsa_key(pkey))
            return false;
        const std::size_t key_size = _key_size(pkey);
        const std::size_t max_len =
            _max_encrypt_plaintext(key_size, pad, digest_type);
        if(max_len == 0 && pad != padding::no_padding)
            return false;
        if(pad == padding::no_padding)
            return plain_len == key_size;
        return plain_len <= max_len;
    }

    static bool _is_stream_valid(std::istream &in,
                                 padding       pad,
                                 std::size_t   key_size,
                                 digest        digest_type,
                                 bool          encryption)
    {
        if(key_size == 0)
            return false;

        const std::streampos current = in.tellg();
        if(current == std::streampos(-1))
            return false;
        in.seekg(0, std::ios::end);
        const std::streampos end = in.tellg();
        in.seekg(current);
        if(end == std::streampos(-1) || end < current)
            return false;

        const std::size_t total = static_cast<std::size_t>(end - current);
        if(!encryption)
            return total > 0 && total % key_size == 0;

        if(pad == padding::no_padding)
            return total > 0 && total % key_size == 0;

        return total > 0
               && _max_encrypt_plaintext(key_size, pad, digest_type) > 0;
    }

    static bool
    _encode_public_key(BIO *bio, const EVP_PKEY *pkey, key_format format)
    {
        if(!bio || !pkey)
            return false;

        const char *structure =
            format == key_format::pkcs1 ? "PKCS1" : "SubjectPublicKeyInfo";
        OSSL_ENCODER_CTX *ctx =
            OSSL_ENCODER_CTX_new_for_pkey(pkey,
                                          EVP_PKEY_PUBLIC_KEY,
                                          "PEM",
                                          structure,
                                          nullptr);
        if(!ctx)
            return false;

        const bool ok = OSSL_ENCODER_to_bio(ctx, bio) == 1;
        OSSL_ENCODER_CTX_free(ctx);
        return ok;
    }

    static bool _encode_private_key(BIO                 *bio,
                                    const EVP_PKEY      *pkey,
                                    key_format           format,
                                    pem_cipher           cipher,
                                    const unsigned char *password,
                                    std::size_t          password_len)
    {
        if(!bio || !pkey)
            return false;
        if(cipher != pem_cipher::none && (!password || password_len == 0))
            return false;

        const char *structure =
            format == key_format::pkcs1 ? "PKCS1" : "PrivateKeyInfo";
        OSSL_ENCODER_CTX *ctx = OSSL_ENCODER_CTX_new_for_pkey(pkey,
                                                              EVP_PKEY_KEYPAIR,
                                                              "PEM",
                                                              structure,
                                                              nullptr);
        if(!ctx)
            return false;

        bool ok = true;
        if(cipher != pem_cipher::none)
        {
            const char *cipher_name = _select_cipher_name(cipher);
            if(!cipher_name
               || OSSL_ENCODER_CTX_set_cipher(ctx, cipher_name, nullptr) <= 0
               || OSSL_ENCODER_CTX_set_passphrase(ctx, password, password_len)
                      <= 0)
                ok = false;
        }

        if(ok)
            ok = OSSL_ENCODER_to_bio(ctx, bio) == 1;

        OSSL_ENCODER_CTX_free(ctx);
        return ok;
    }
};

} // namespace hj

#endif // RSA_HPP
