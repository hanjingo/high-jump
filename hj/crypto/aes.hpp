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

#ifndef AES_HPP
#define AES_HPP

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

namespace hj
{

class aes
{
  public:
    static inline constexpr std::size_t buf_sz = 16384; // 16KB

    enum class error_code
    {
        ok = 0,
        key_invalid,
        iv_invalid,
        plain_invalid,
        padding_style_invalid,
        input_stream_invalid,
        output_stream_invalid,
        ctx_alloc_failed,
        cipher_ctl_failed,
        get_tag_failed,
        tag_verify_failed,
        file_open_failed,
        file_read_failed,
        file_write_failed,
        param_invalid,
        passwd_invalid,
        pbkdf2_derive_failed,
        rand_generate_failed,

        encrypt_failed,
        encrypt_init_failed,
        encrypt_update_failed,
        encrypt_final_failed,

        decrypt_failed,
        decrypt_init_failed,
        decrypt_update_failed,
        decrypt_final_failed,
    };

    enum class mode
    {
        ecb,
        cbc,
        cfb1,
        cfb8,
        cfb128,
        cfb = cfb128,
        ofb,
        ctr,
        gcm,
        ccm,
        xts,
        wrap,
        wrap_pad,
        cbc_hmac_sha1,
        cbc_hmac_sha256,

#ifndef OPENSSL_NO_OCB
        ocb,
#endif
    };

    enum class padding
    {
        pkcs5,
        pkcs7,
        zero,
        iso10126,
        ansix923,
        iso_iec_7816_4,
        no_padding
    };

    struct evp_ctx_deleter
    {
        void operator()(EVP_CIPHER_CTX *ctx) const
        {
            if(ctx)
                EVP_CIPHER_CTX_free(ctx);
        }
    };

    struct options
    {
        const unsigned char *key       = nullptr;
        std::size_t          key_len   = 0;
        const unsigned char *iv        = nullptr;
        std::size_t          iv_len    = 0;
        mode                 mod       = mode::gcm;
        padding              pad_style = padding::pkcs7;

        options() = default;
        options(const unsigned char *k,
                std::size_t          kl,
                const unsigned char *i  = nullptr,
                std::size_t          il = 0,
                mode                 m  = mode::gcm,
                padding              p  = padding::pkcs7)
            : key(k)
            , key_len(kl)
            , iv(i)
            , iv_len(il)
            , mod(m)
            , pad_style(p)
        {
        }

        options &operator=(const options &other)
        {
            if(this != &other)
            {
                key       = other.key;
                key_len   = other.key_len;
                iv        = other.iv;
                iv_len    = other.iv_len;
                mod       = other.mod;
                pad_style = other.pad_style;
            }
            return *this;
        }

        void reset()
        {
            key       = nullptr;
            key_len   = 0;
            iv        = nullptr;
            iv_len    = 0;
            mod       = mode::gcm;
            pad_style = padding::pkcs7;
        }
    };

    using safe_evp_ctx = std::unique_ptr<EVP_CIPHER_CTX, evp_ctx_deleter>;

  public:
    static inline error_code encrypt(unsigned char       *dst,
                                     std::size_t         &dst_len,
                                     const unsigned char *src,
                                     const std::size_t    src_len,
                                     const options       &opts)
    {
        if(is_aead_mode(opts.mod))
            return error_code::param_invalid;
        if(!is_plain_valid(opts.mod, opts.key_len, opts.pad_style, src_len))
            return error_code::plain_invalid;
        if(!is_key_valid(opts.mod, opts.key, opts.key_len))
            return error_code::key_invalid;
        if(opts.mod != mode::ecb
           && !is_iv_valid(opts.mod, opts.iv, opts.iv_len))
            return error_code::iv_invalid;

        std::vector<unsigned char> padded_src;
        std::size_t                padded_len = 0;
        _padding_block(padded_src,
                       padded_len,
                       opts.mod,
                       opts.key_len,
                       opts.pad_style,
                       src,
                       src_len);

        safe_evp_ctx ctx(EVP_CIPHER_CTX_new());
        if(!ctx)
            return error_code::ctx_alloc_failed;

        const EVP_CIPHER *cipher = _select_cipher(opts.mod, opts.key_len);
        if(!cipher)
            return error_code::encrypt_init_failed;

        if(EVP_EncryptInit_ex(ctx.get(), cipher, nullptr, opts.key, opts.iv)
           != 1)
            return error_code::encrypt_init_failed;

        EVP_CIPHER_CTX_set_padding(ctx.get(), 0);

        int outlen1 = 0, outlen2 = 0;
        if(EVP_EncryptUpdate(ctx.get(),
                             dst,
                             &outlen1,
                             padded_src.data(),
                             static_cast<int>(padded_len))
           != 1)
        {
            OPENSSL_cleanse(padded_src.data(), padded_src.size());
            return error_code::encrypt_update_failed;
        }

        if(EVP_EncryptFinal_ex(ctx.get(), dst + outlen1, &outlen2) != 1)
        {
            OPENSSL_cleanse(padded_src.data(), padded_src.size());
            return error_code::encrypt_final_failed;
        }

        dst_len = static_cast<std::size_t>(outlen1 + outlen2);
        OPENSSL_cleanse(padded_src.data(), padded_src.size());
        return error_code::ok;
    }

    static inline error_code decrypt(unsigned char       *dst,
                                     std::size_t         &dst_len,
                                     const unsigned char *src,
                                     const std::size_t    src_len,
                                     const options       &opts)
    {
        if(is_aead_mode(opts.mod))
            return error_code::param_invalid;
        if(!is_key_valid(opts.mod, opts.key, opts.key_len))
            return error_code::key_invalid;
        if(opts.mod != mode::ecb
           && !is_iv_valid(opts.mod, opts.iv, opts.iv_len))
            return error_code::iv_invalid;

        safe_evp_ctx ctx(EVP_CIPHER_CTX_new());
        if(!ctx)
            return error_code::ctx_alloc_failed;

        const EVP_CIPHER *cipher = _select_cipher(opts.mod, opts.key_len);
        if(!cipher)
            return error_code::decrypt_init_failed;

        if(EVP_DecryptInit_ex(ctx.get(), cipher, nullptr, opts.key, opts.iv)
           != 1)
            return error_code::decrypt_init_failed;

        EVP_CIPHER_CTX_set_padding(ctx.get(), 0);

        int outlen1 = 0, outlen2 = 0;
        if(EVP_DecryptUpdate(ctx.get(),
                             dst,
                             &outlen1,
                             src,
                             static_cast<int>(src_len))
           != 1)
            return error_code::decrypt_update_failed;

        if(EVP_DecryptFinal_ex(ctx.get(), dst + outlen1, &outlen2) != 1)
            return error_code::decrypt_final_failed;

        std::size_t raw_dec_len = static_cast<std::size_t>(outlen1 + outlen2);
        return _unpadding_block(dst,
                                dst_len,
                                raw_dec_len,
                                opts.mod,
                                opts.key_len,
                                opts.pad_style);
    }

    static inline error_code
    encrypt(std::string &dst, const std::string &src, const options &opts)
    {
        std::size_t                max_dst_len = src.size() + 32;
        std::vector<unsigned char> outbuf(max_dst_len);
        std::size_t                out_len = 0;

        error_code ec =
            encrypt(outbuf.data(),
                    out_len,
                    reinterpret_cast<const unsigned char *>(src.data()),
                    src.size(),
                    opts);

        if(ec == error_code::ok)
            dst.assign(reinterpret_cast<char *>(outbuf.data()), out_len);

        OPENSSL_cleanse(outbuf.data(), outbuf.size());
        return ec;
    }

    static inline error_code
    decrypt(std::string &dst, const std::string &src, const options &opts)
    {
        std::vector<unsigned char> outbuf(src.size() + 16);
        std::size_t                out_len = 0;

        error_code ec =
            decrypt(outbuf.data(),
                    out_len,
                    reinterpret_cast<const unsigned char *>(src.data()),
                    src.size(),
                    opts);

        if(ec == error_code::ok)
            dst.assign(reinterpret_cast<char *>(outbuf.data()), out_len);

        OPENSSL_cleanse(outbuf.data(), outbuf.size());
        return ec;
    }

    static inline error_code encrypt_aead(unsigned char       *dst_cipher,
                                          std::size_t         &cipher_len,
                                          unsigned char       *out_tag,
                                          const std::size_t    tag_len,
                                          const unsigned char *src_plain,
                                          const std::size_t    plain_len,
                                          const unsigned char *aad,
                                          const std::size_t    aad_len,
                                          const options       &opts)
    {
        if(!is_aead_mode(opts.mod))
            return error_code::param_invalid;
        if(!is_key_valid(opts.mod, opts.key, opts.key_len))
            return error_code::key_invalid;
        if(!opts.iv || opts.iv_len == 0)
            return error_code::iv_invalid;

        safe_evp_ctx ctx(EVP_CIPHER_CTX_new());
        if(!ctx)
            return error_code::ctx_alloc_failed;

        const EVP_CIPHER *cipher = _select_cipher(opts.mod, opts.key_len);
        if(!cipher
           || EVP_EncryptInit_ex(ctx.get(), cipher, nullptr, nullptr, nullptr)
                  != 1)
            return error_code::encrypt_init_failed;

        if(opts.iv_len != 12)
        {
            if(EVP_CIPHER_CTX_ctrl(ctx.get(),
                                   EVP_CTRL_AEAD_SET_IVLEN,
                                   static_cast<int>(opts.iv_len),
                                   nullptr)
               != 1)
                return error_code::cipher_ctl_failed;
        }

        if(EVP_EncryptInit_ex(ctx.get(), nullptr, nullptr, opts.key, opts.iv)
           != 1)
            return error_code::encrypt_init_failed;

        int unused = 0;
        if(aad && aad_len > 0)
        {
            if(EVP_EncryptUpdate(ctx.get(),
                                 nullptr,
                                 &unused,
                                 aad,
                                 static_cast<int>(aad_len))
               != 1)
                return error_code::encrypt_update_failed;
        }

        int outlen1 = 0, outlen2 = 0;
        if(EVP_EncryptUpdate(ctx.get(),
                             dst_cipher,
                             &outlen1,
                             src_plain,
                             static_cast<int>(plain_len))
           != 1)
            return error_code::encrypt_update_failed;

        if(EVP_EncryptFinal_ex(ctx.get(), dst_cipher + outlen1, &outlen2) != 1)
            return error_code::encrypt_final_failed;

        cipher_len = static_cast<std::size_t>(outlen1 + outlen2);
        if(EVP_CIPHER_CTX_ctrl(ctx.get(),
                               EVP_CTRL_AEAD_GET_TAG,
                               static_cast<int>(tag_len),
                               out_tag)
           != 1)
            return error_code::get_tag_failed;

        return error_code::ok;
    }

    static inline error_code decrypt_aead(unsigned char       *dst_plain,
                                          std::size_t         &plain_len,
                                          const unsigned char *src_cipher,
                                          const std::size_t    cipher_len,
                                          const unsigned char *tag,
                                          const std::size_t    tag_len,
                                          const unsigned char *aad,
                                          const std::size_t    aad_len,
                                          const options       &opts)
    {
        if(!is_aead_mode(opts.mod))
            return error_code::param_invalid;
        if(!is_key_valid(opts.mod, opts.key, opts.key_len))
            return error_code::key_invalid;
        if(!opts.iv || opts.iv_len == 0 || !tag)
            return error_code::iv_invalid;

        safe_evp_ctx ctx(EVP_CIPHER_CTX_new());
        if(!ctx)
            return error_code::ctx_alloc_failed;

        const EVP_CIPHER *cipher = _select_cipher(opts.mod, opts.key_len);
        if(!cipher
           || EVP_DecryptInit_ex(ctx.get(), cipher, nullptr, nullptr, nullptr)
                  != 1)
            return error_code::decrypt_init_failed;

        if(opts.iv_len != 12)
        {
            if(EVP_CIPHER_CTX_ctrl(ctx.get(),
                                   EVP_CTRL_AEAD_SET_IVLEN,
                                   static_cast<int>(opts.iv_len),
                                   nullptr)
               != 1)
                return error_code::cipher_ctl_failed;
        }

        if(EVP_DecryptInit_ex(ctx.get(), nullptr, nullptr, opts.key, opts.iv)
           != 1)
            return error_code::decrypt_init_failed;

        int unused = 0;
        if(aad && aad_len > 0)
        {
            if(EVP_DecryptUpdate(ctx.get(),
                                 nullptr,
                                 &unused,
                                 aad,
                                 static_cast<int>(aad_len))
               != 1)
                return error_code::decrypt_update_failed;
        }

        int outlen1 = 0, outlen2 = 0;
        if(EVP_DecryptUpdate(ctx.get(),
                             dst_plain,
                             &outlen1,
                             src_cipher,
                             static_cast<int>(cipher_len))
           != 1)
            return error_code::decrypt_update_failed;

        if(EVP_CIPHER_CTX_ctrl(ctx.get(),
                               EVP_CTRL_AEAD_SET_TAG,
                               static_cast<int>(tag_len),
                               const_cast<unsigned char *>(tag))
           != 1)
            return error_code::cipher_ctl_failed;

        if(EVP_DecryptFinal_ex(ctx.get(), dst_plain + outlen1, &outlen2) <= 0)
        {
            OPENSSL_cleanse(dst_plain, outlen1);
            return error_code::tag_verify_failed;
        }

        plain_len = static_cast<std::size_t>(outlen1 + outlen2);
        return error_code::ok;
    }

    static inline error_code encrypt_aead(std::string       &dst_cipher,
                                          std::string       &out_tag,
                                          std::size_t        tag_len,
                                          const std::string &src_plain,
                                          const std::string &aad,
                                          const options     &opts)
    {
        std::vector<unsigned char> cipher_buf(src_plain.size() + 16);
        std::vector<unsigned char> tag_buf(tag_len);
        std::size_t                cipher_len = 0;
        error_code                 ec         = encrypt_aead(
            cipher_buf.data(),
            cipher_len,
            tag_buf.data(),
            tag_len,
            reinterpret_cast<const unsigned char *>(src_plain.data()),
            src_plain.size(),
            reinterpret_cast<const unsigned char *>(aad.data()),
            aad.size(),
            opts);

        if(ec == error_code::ok)
        {
            dst_cipher.assign(reinterpret_cast<char *>(cipher_buf.data()),
                              cipher_len);
            out_tag.assign(reinterpret_cast<char *>(tag_buf.data()), tag_len);
        }

        OPENSSL_cleanse(cipher_buf.data(), cipher_buf.size());
        return ec;
    }

    static inline error_code decrypt_aead(std::string       &dst_plain,
                                          const std::string &src_cipher,
                                          const std::string &tag,
                                          const std::string &aad,
                                          const options     &opts)
    {
        std::vector<unsigned char> dst_plain_buf(src_cipher.size() + 16);
        std::size_t                dst_plain_len = 0;
        const auto                *cipher_ptr =
            src_cipher.empty()
                ? nullptr
                : reinterpret_cast<const unsigned char *>(src_cipher.data());
        const auto *tag_ptr =
            tag.empty() ? nullptr
                        : reinterpret_cast<const unsigned char *>(tag.data());
        const auto *aad_ptr =
            aad.empty() ? nullptr
                        : reinterpret_cast<const unsigned char *>(aad.data());
        error_code ec = decrypt_aead(dst_plain_buf.data(),
                                     dst_plain_len,
                                     cipher_ptr,
                                     src_cipher.size(),
                                     tag_ptr,
                                     tag.size(),
                                     aad_ptr,
                                     aad.size(),
                                     opts);
        if(ec == error_code::ok)
            dst_plain.assign(
                reinterpret_cast<const char *>(dst_plain_buf.data()),
                dst_plain_len);
        else
            dst_plain.clear();

        if(!dst_plain_buf.empty())
            OPENSSL_cleanse(dst_plain_buf.data(), dst_plain_buf.size());

        return ec;
    }

    static inline error_code
    encrypt(std::ostream &out, std::istream &in, const options &opts)
    {
        if(is_aead_mode(opts.mod))
            return error_code::param_invalid;
        if(!is_key_valid(opts.mod, opts.key, opts.key_len))
            return error_code::key_invalid;
        if(opts.mod != mode::ecb
           && !is_iv_valid(opts.mod, opts.iv, opts.iv_len))
            return error_code::iv_invalid;

        safe_evp_ctx ctx(EVP_CIPHER_CTX_new());
        if(!ctx)
            return error_code::ctx_alloc_failed;

        const EVP_CIPHER *cipher = _select_cipher(opts.mod, opts.key_len);
        if(!cipher)
            return error_code::encrypt_init_failed;

        if(EVP_EncryptInit_ex(ctx.get(), cipher, nullptr, opts.key, opts.iv)
           != 1)
            return error_code::encrypt_init_failed;

        EVP_CIPHER_CTX_set_padding(ctx.get(), 0);

        constexpr std::size_t      block_size = 16;
        std::vector<unsigned char> read_buf(buf_sz);
        std::vector<unsigned char> accum_buf;
        accum_buf.reserve(buf_sz + block_size);

        std::vector<unsigned char> outbuf(buf_sz + block_size);
        int                        outlen = 0;

        while(in.good())
        {
            in.read(reinterpret_cast<char *>(read_buf.data()), read_buf.size());
            std::streamsize readlen = in.gcount();
            if(readlen <= 0)
                break;

            accum_buf.insert(accum_buf.end(),
                             read_buf.data(),
                             read_buf.data() + readlen);

            if(accum_buf.size() > block_size)
            {
                std::size_t process_len =
                    ((accum_buf.size() - 1) / block_size) * block_size;
                if(process_len > 0)
                {
                    if(EVP_EncryptUpdate(ctx.get(),
                                         outbuf.data(),
                                         &outlen,
                                         accum_buf.data(),
                                         static_cast<int>(process_len))
                       != 1)
                        return error_code::encrypt_update_failed;

                    out.write(reinterpret_cast<const char *>(outbuf.data()),
                              outlen);
                    accum_buf.erase(accum_buf.begin(),
                                    accum_buf.begin() + process_len);
                }
            }
        }

        std::vector<unsigned char> padded_tail;
        std::size_t                padded_tail_len = 0;
        _padding_block(padded_tail,
                       padded_tail_len,
                       opts.mod,
                       opts.key_len,
                       opts.pad_style,
                       accum_buf.data(),
                       accum_buf.size());

        if(padded_tail_len > 0)
        {
            if(EVP_EncryptUpdate(ctx.get(),
                                 outbuf.data(),
                                 &outlen,
                                 padded_tail.data(),
                                 static_cast<int>(padded_tail_len))
               != 1)
                return error_code::encrypt_update_failed;

            out.write(reinterpret_cast<const char *>(outbuf.data()), outlen);
        }

        if(EVP_EncryptFinal_ex(ctx.get(), outbuf.data(), &outlen) != 1)
            return error_code::encrypt_final_failed;

        if(outlen > 0)
            out.write(reinterpret_cast<const char *>(outbuf.data()), outlen);

        return error_code::ok;
    }

    static inline error_code
    decrypt(std::ostream &out, std::istream &in, const options &opts)
    {
        if(is_aead_mode(opts.mod))
            return error_code::param_invalid;
        if(!is_key_valid(opts.mod, opts.key, opts.key_len))
            return error_code::key_invalid;
        if(opts.mod != mode::ecb
           && !is_iv_valid(opts.mod, opts.iv, opts.iv_len))
            return error_code::iv_invalid;

        safe_evp_ctx ctx(EVP_CIPHER_CTX_new());
        if(!ctx)
            return error_code::ctx_alloc_failed;

        const EVP_CIPHER *cipher = _select_cipher(opts.mod, opts.key_len);
        if(!cipher)
            return error_code::decrypt_init_failed;

        if(EVP_DecryptInit_ex(ctx.get(), cipher, nullptr, opts.key, opts.iv)
           != 1)
            return error_code::decrypt_init_failed;

        EVP_CIPHER_CTX_set_padding(ctx.get(), 0);

        constexpr std::size_t      block_size = 16;
        std::vector<unsigned char> inbuf(buf_sz);
        std::vector<unsigned char> dec_accum;
        dec_accum.reserve(buf_sz + block_size);
        std::vector<unsigned char> outbuf(buf_sz + block_size);
        int                        outlen = 0;
        while(in.good())
        {
            in.read(reinterpret_cast<char *>(inbuf.data()), inbuf.size());
            std::streamsize readlen = in.gcount();
            if(readlen <= 0)
                break;

            if(EVP_DecryptUpdate(ctx.get(),
                                 outbuf.data(),
                                 &outlen,
                                 inbuf.data(),
                                 static_cast<int>(readlen))
               != 1)
                return error_code::decrypt_update_failed;

            dec_accum.insert(dec_accum.end(),
                             outbuf.data(),
                             outbuf.data() + outlen);

            if(dec_accum.size() > block_size)
            {
                std::size_t write_len = dec_accum.size() - block_size;
                out.write(reinterpret_cast<const char *>(dec_accum.data()),
                          write_len);
                dec_accum.erase(dec_accum.begin(),
                                dec_accum.begin() + write_len);
            }
        }

        if(EVP_DecryptFinal_ex(ctx.get(), outbuf.data(), &outlen) != 1)
            return error_code::decrypt_final_failed;

        if(outlen > 0)
            dec_accum.insert(dec_accum.end(),
                             outbuf.data(),
                             outbuf.data() + outlen);

        if(!dec_accum.empty())
        {
            std::size_t unpadded_len = 0;
            error_code  ec           = _unpadding_block(dec_accum.data(),
                                                        unpadded_len,
                                                        dec_accum.size(),
                                                        opts.mod,
                                                        opts.key_len,
                                                        opts.pad_style);
            if(ec != error_code::ok)
                return ec;

            if(unpadded_len > 0)
                out.write(reinterpret_cast<const char *>(dec_accum.data()),
                          unpadded_len);
        }

        return error_code::ok;
    }

    static inline error_code encrypt_file(const char    *dst_file_path,
                                          const char    *src_file_path,
                                          const options &opts)
    {
        std::ifstream in(src_file_path, std::ios::binary);
        std::ofstream out(dst_file_path, std::ios::binary);
        if(!in.is_open() || !out.is_open())
            return error_code::file_open_failed;

        return encrypt(out, in, opts);
    }

    static inline error_code decrypt_file(const char    *dst_file_path,
                                          const char    *src_file_path,
                                          const options &opts)
    {
        std::ifstream in(src_file_path, std::ios::binary);
        if(!in.is_open())
            return error_code::file_open_failed;
        std::ofstream out(dst_file_path, std::ios::binary);
        if(!out.is_open())
            return error_code::file_open_failed;

        return decrypt(out, in, opts);
    }

    static inline bool generate_random_bytes(unsigned char *buf,
                                             std::size_t    len)
    {
        return RAND_bytes(buf, static_cast<int>(len)) == 1;
    }

    // make key with password or random bytes
    static error_code keygen(unsigned char       *key,
                             const std::size_t    key_len,
                             const unsigned char *password     = nullptr,
                             const std::size_t    password_len = 0,
                             const unsigned char *salt         = nullptr,
                             const std::size_t    salt_len     = 0,
                             const int            iterations   = 10000)
    {
        if(key == nullptr || (key_len != 16 && key_len != 24 && key_len != 32))
            return error_code::param_invalid;
        if(password == nullptr || password_len == 0)
            return RAND_bytes(key, static_cast<int>(key_len)) == 1
                       ? error_code::ok
                       : error_code::rand_generate_failed;

        // Use PBKDF2 to derive the key from the password
        return PKCS5_PBKDF2_HMAC(reinterpret_cast<const char *>(password),
                                 static_cast<int>(password_len),
                                 salt,
                                 static_cast<int>(salt_len),
                                 iterations,
                                 EVP_sha256(),
                                 static_cast<int>(key_len),
                                 key)
                       == 1
                   ? error_code::ok
                   : error_code::pbkdf2_derive_failed;
    }

    static inline bool is_aead_mode(const mode mod)
    {
        return mod == mode::gcm || mod == mode::ccm;
    }

    static inline bool is_stream_mode(const mode mod)
    {
        return mod == mode::cfb1 || mod == mode::cfb8 || mod == mode::cfb128
               || mod == mode::ofb || mod == mode::ctr;
    }

    static inline bool
    is_key_valid(const mode mod, const unsigned char *key, std::size_t key_len)
    {
        if(!key)
            return false;
        if(mod == mode::xts)
            return key_len == 32 || key_len == 64;
        return key_len == 16 || key_len == 24 || key_len == 32;
    }

    static inline bool
    is_iv_valid(const mode mod, const unsigned char *iv, std::size_t iv_len)
    {
        if(mod == mode::ecb)
            return iv == nullptr || iv_len == 0;
        if(!iv)
            return false;
        if(mod == mode::gcm)
            return iv_len > 0;
        return iv_len == 16;
    }

    static inline bool is_plain_valid(const mode  mod,
                                      std::size_t key_len,
                                      padding     pad_style,
                                      std::size_t plain_len)
    {
        if(is_stream_mode(mod) || is_aead_mode(mod))
            return true;
        if(pad_style == padding::no_padding)
            return plain_len % 16 == 0;
        return true;
    }

  private:
    static inline const EVP_CIPHER *_select_cipher(const mode  mod,
                                                   std::size_t key_len)
    {
        switch(mod)
        {
            case mode::ecb:
                if(key_len == 16)
                    return EVP_aes_128_ecb();
                if(key_len == 24)
                    return EVP_aes_192_ecb();
                if(key_len == 32)
                    return EVP_aes_256_ecb();
                break;
            case mode::cbc:
                if(key_len == 16)
                    return EVP_aes_128_cbc();
                if(key_len == 24)
                    return EVP_aes_192_cbc();
                if(key_len == 32)
                    return EVP_aes_256_cbc();
                break;
            case mode::cfb128:
                if(key_len == 16)
                    return EVP_aes_128_cfb128();
                if(key_len == 24)
                    return EVP_aes_192_cfb128();
                if(key_len == 32)
                    return EVP_aes_256_cfb128();
                break;
            case mode::ofb:
                if(key_len == 16)
                    return EVP_aes_128_ofb();
                if(key_len == 24)
                    return EVP_aes_192_ofb();
                if(key_len == 32)
                    return EVP_aes_256_ofb();
                break;
            case mode::ctr:
                if(key_len == 16)
                    return EVP_aes_128_ctr();
                if(key_len == 24)
                    return EVP_aes_192_ctr();
                if(key_len == 32)
                    return EVP_aes_256_ctr();
                break;
            case mode::gcm:
                if(key_len == 16)
                    return EVP_aes_128_gcm();
                if(key_len == 24)
                    return EVP_aes_192_gcm();
                if(key_len == 32)
                    return EVP_aes_256_gcm();
                break;
            default:
                break;
        }
        return nullptr;
    }

    static inline void _padding_block(std::vector<unsigned char> &padded_src,
                                      std::size_t                &padded_len,
                                      const mode                  mod,
                                      const std::size_t           key_len,
                                      const padding               pad_style,
                                      const unsigned char        *src,
                                      const std::size_t           src_len)
    {
        if(is_aead_mode(mod) || pad_style == padding::no_padding)
        {
            padded_src.assign(src, src + src_len);
            padded_len = src_len;
            return;
        }

        std::size_t block_size = 16;
        std::size_t pad_len    = block_size - (src_len % block_size);
        padded_len             = src_len + pad_len;
        padded_src.resize(padded_len);
        std::memcpy(padded_src.data(), src, src_len);
        unsigned char *pad_ptr = padded_src.data() + src_len;

        switch(pad_style)
        {
            case padding::pkcs5:
            case padding::pkcs7:
                std::fill_n(pad_ptr,
                            pad_len,
                            static_cast<unsigned char>(pad_len));
                break;
            case padding::zero:
                std::fill_n(pad_ptr, pad_len, 0);
                break;
            case padding::iso10126:
                if(pad_len > 1)
                    generate_random_bytes(
                        pad_ptr,
                        static_cast<std::size_t>(pad_len - 1));
                pad_ptr[pad_len - 1] = static_cast<unsigned char>(pad_len);
                break;
            case padding::ansix923:
                std::fill_n(pad_ptr, pad_len - 1, 0);
                pad_ptr[pad_len - 1] = static_cast<unsigned char>(pad_len);
                break;
            case padding::iso_iec_7816_4:
                pad_ptr[0] = 0x80;
                std::fill_n(pad_ptr + 1, pad_len - 1, 0);
                break;
            default:
                break;
        }
    }

    static inline unsigned char _ct_is_eq(unsigned char a, unsigned char b)
    {
        unsigned int diff =
            static_cast<unsigned int>(a) ^ static_cast<unsigned int>(b);
        return static_cast<unsigned char>(~((diff | -diff) >> 31));
    }

    static inline unsigned char _ct_is_lt(std::size_t a, std::size_t b)
    {
        std::size_t diff = a - b;
        return static_cast<unsigned char>(~((diff ^ ((a ^ b) & (a ^ diff)))
                                            >> (sizeof(std::size_t) * 8 - 1)));
    }

    static inline error_code _unpadding_block(unsigned char    *dst,
                                              std::size_t      &dst_len,
                                              std::size_t       unpad_len,
                                              const mode        mod,
                                              const std::size_t key_len,
                                              const padding     pad_style)
    {
        dst_len = unpad_len;
        if(is_aead_mode(mod) || pad_style == padding::no_padding
           || unpad_len == 0)
            return error_code::ok;

        constexpr std::size_t block_size = 16;
        if(unpad_len < block_size || (unpad_len % block_size != 0))
            return error_code::padding_style_invalid;

        unsigned char pad_val = dst[unpad_len - 1];
        unsigned char pad_val_valid =
            _ct_is_lt(static_cast<std::size_t>(pad_val - 1), block_size);

        switch(pad_style)
        {
            case padding::pkcs5:
            case padding::pkcs7: {
                unsigned char bad         = 0;
                std::size_t   block_start = unpad_len - block_size;

                for(std::size_t i = 0; i < block_size; ++i)
                {
                    std::size_t   idx      = block_start + i;
                    unsigned char byte_val = dst[idx];
                    unsigned char in_pad_zone =
                        ~_ct_is_lt(i, block_size - pad_val);
                    bad |= in_pad_zone & (byte_val ^ pad_val);
                }

                if(pad_val_valid && bad == 0)
                {
                    dst_len = unpad_len - pad_val;
                    return error_code::ok;
                }
                return error_code::padding_style_invalid;
            }

            case padding::ansix923: {
                unsigned char bad         = 0;
                std::size_t   block_start = unpad_len - block_size;

                for(std::size_t i = 0; i < block_size - 1; ++i)
                {
                    std::size_t   idx      = block_start + i;
                    unsigned char byte_val = dst[idx];
                    unsigned char in_pad_zone =
                        ~_ct_is_lt(i, block_size - pad_val);

                    bad |= in_pad_zone & byte_val;
                }

                if(pad_val_valid && bad == 0)
                {
                    dst_len = unpad_len - pad_val;
                    return error_code::ok;
                }
                return error_code::padding_style_invalid;
            }

            case padding::iso10126: {
                if(pad_val_valid)
                {
                    dst_len = unpad_len - pad_val;
                    return error_code::ok;
                }
                return error_code::padding_style_invalid;
            }

            case padding::zero: {
                std::size_t i = unpad_len;
                while(i > 0 && dst[i - 1] == 0)
                    --i;
                dst_len = i;
                return error_code::ok;
            }

            case padding::iso_iec_7816_4: {
                std::size_t i = unpad_len;
                while(i > 0 && dst[i - 1] == 0)
                    --i;

                if(i > 0 && dst[i - 1] == 0x80)
                {
                    dst_len = i - 1;
                    return error_code::ok;
                }
                return error_code::padding_style_invalid;
            }

            default:
                break;
        }

        return error_code::ok;
    }

  private:
    aes()  = delete;
    ~aes() = delete;
};

} // namespace hj

#endif // AES_HPP