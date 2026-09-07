/*
 * This file is part of high-jump(hj).
 *
 * Copyright 2025 hanjingo <hehehunanchina@live.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DES_HPP
#define DES_HPP

#include <algorithm>
#include <array>
#include <climits>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <istream>
#include <memory>
#include <ostream>
#include <random>
#include <string>
#include <vector>

#include <openssl/evp.h>

namespace hj
{

class des
{
  public:
    enum class error_code
    {
        ok = 0,
        invalid_key,
        invalid_iv,
        invalid_plain,
        invalid_padding,
        invalid_input,
        invalid_output,
        buffer_too_small,
        encrypt_failed,
        decrypt_failed,
        file_io_failed,
        unsupported_algorithm,
        unknown
    };

    enum class mode
    {
        ecb,
        cbc,
        cfb,
        ofb,
        ctr
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

    static constexpr std::size_t block_size = 8;

    struct options
    {
        const unsigned char *key       = nullptr;
        std::size_t          key_len   = 0;
        const unsigned char *iv        = nullptr;
        std::size_t          iv_len    = 0;
        mode                 mod       = mode::ecb;
        padding              pad_style = padding::pkcs7;

        options() = default;

        options(const unsigned char *k,
                std::size_t          kl,
                mode                 m  = mode::ecb,
                padding              p  = padding::pkcs7,
                const unsigned char *i  = nullptr,
                std::size_t          il = 0)
            : key(k)
            , key_len(kl)
            , iv(i)
            , iv_len(il)
            , mod(m)
            , pad_style(p)
        {
            if(mod == mode::ecb)
            {
                iv     = nullptr;
                iv_len = 0;
            }
        }

        void reset()
        {
            key       = nullptr;
            key_len   = 0;
            iv        = nullptr;
            iv_len    = 0;
            mod       = mode::ecb;
            pad_style = padding::pkcs7;
        }
    };

  public:
    static error_code encrypt(unsigned char       *dst,
                              std::size_t          dst_capacity,
                              std::size_t         &dst_len,
                              const unsigned char *src,
                              std::size_t          src_len,
                              const options       &opt)
    {
        dst_len = 0;

        if(!dst && dst_capacity != 0)
            return error_code::invalid_output;

        if(!is_key_valid(opt.key, opt.key_len))
            return error_code::invalid_key;

        if(!is_iv_valid(opt.mod, opt.iv, opt.iv_len))
            return error_code::invalid_iv;

        if(!is_plain_valid(src, src_len, opt.mod, opt.pad_style))
            return error_code::invalid_plain;

        /*
         * CTR only supports no_padding and works on arbitrary byte streams.
         */
        if(opt.mod == mode::ctr)
        {
            if(opt.pad_style != padding::no_padding)
                return error_code::invalid_padding;

            return crypt_ctr(dst,
                             dst_capacity,
                             dst_len,
                             src,
                             src_len,
                             opt,
                             false);
        }

        std::vector<unsigned char> input;

        if(!build_padded_input(input, src, src_len, opt.pad_style))
            return error_code::invalid_plain;

        const std::size_t required = input.size();

        if(dst_capacity < required)
            return error_code::buffer_too_small;

        if(required != 0 && !dst)
            return error_code::buffer_too_small;

        evp_ctx_ptr ctx(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);

        if(!ctx)
            return error_code::encrypt_failed;

        auto cipher = select_cipher(opt);

        if(!cipher)
            return error_code::unsupported_algorithm;

        if(EVP_EncryptInit_ex(ctx.get(),
                              cipher.get(),
                              nullptr,
                              opt.key,
                              select_iv(opt))
           != 1)
        {
            return error_code::encrypt_failed;
        }

        if(EVP_CIPHER_CTX_set_padding(ctx.get(), 0) != 1)
            return error_code::encrypt_failed;

        int out_len = 0;

        if(!input.empty())
        {
            if(checked_int(input.size()) < 0)
                return error_code::invalid_input;

            if(EVP_EncryptUpdate(ctx.get(),
                                 dst,
                                 &out_len,
                                 input.data(),
                                 checked_int(input.size()))
               != 1)
            {
                return error_code::encrypt_failed;
            }
        }

        int final_len = 0;

        if(EVP_EncryptFinal_ex(ctx.get(), dst + out_len, &final_len) != 1)
        {
            return error_code::encrypt_failed;
        }

        dst_len = static_cast<std::size_t>(out_len + final_len);

        return error_code::ok;
    }

    static error_code encrypt(unsigned char       *dst,
                              std::size_t         &dst_len,
                              const unsigned char *src,
                              std::size_t          src_len,
                              const options       &opt)
    {
        const std::size_t capacity = encrypt_len_reserve(src_len);

        return encrypt(dst, capacity, dst_len, src, src_len, opt);
    }

    static error_code
    encrypt(std::string &dst, const std::string &src, const options &opt)
    {
        dst.resize(encrypt_len_reserve(src.size()));

        std::size_t dst_len = 0;

        auto ec = encrypt(reinterpret_cast<unsigned char *>(dst.data()),
                          dst.size(),
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
    encrypt(std::ostream &out, std::istream &in, const options &opt)
    {
        if(!in)
            return error_code::invalid_input;

        if(!out)
            return error_code::invalid_output;

        if(!is_key_valid(opt.key, opt.key_len))
            return error_code::invalid_key;

        if(!is_iv_valid(opt.mod, opt.iv, opt.iv_len))
            return error_code::invalid_iv;

        if(opt.mod == mode::ctr)
        {
            if(opt.pad_style != padding::no_padding)
                return error_code::invalid_padding;

            return crypt_ctr_stream(out, in, opt, false);
        }

        evp_ctx_ptr ctx(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);

        if(!ctx)
            return error_code::encrypt_failed;

        auto cipher = select_cipher(opt);

        if(!cipher)
            return error_code::unsupported_algorithm;

        if(EVP_EncryptInit_ex(ctx.get(),
                              cipher.get(),
                              nullptr,
                              opt.key,
                              select_iv(opt))
               != 1
           || EVP_CIPHER_CTX_set_padding(ctx.get(), 0) != 1)
        {
            return error_code::encrypt_failed;
        }

        std::array<unsigned char, block_size>                        pending{};
        std::array<unsigned char, block_size>                        input{};
        std::array<unsigned char, block_size + EVP_MAX_BLOCK_LENGTH> outbuf{};

        std::size_t pending_len  = 0;
        bool        have_pending = false;

        while(true)
        {
            in.read(reinterpret_cast<char *>(input.data()),
                    static_cast<std::streamsize>(block_size));

            const std::streamsize n = in.gcount();

            if(n == 0)
            {
                if(in.bad())
                    return error_code::file_io_failed;

                break;
            }

            const std::size_t nbytes = static_cast<std::size_t>(n);

            if(have_pending)
            {
                if(auto ec = evp_update_write(ctx.get(),
                                              out,
                                              outbuf,
                                              pending.data(),
                                              block_size);
                   ec != error_code::ok)
                {
                    return ec;
                }
            }

            std::memcpy(pending.data(), input.data(), nbytes);

            pending_len  = nbytes;
            have_pending = true;

            if(nbytes < block_size)
                break;
        }

        if(!have_pending)
        {
            if(opt.pad_style == padding::no_padding)
            {
                return error_code::invalid_plain;
            }

            unsigned char pad_block[block_size]{};

            make_padding_block(pad_block, 0, nullptr, opt.pad_style);

            if(auto ec = evp_update_write(ctx.get(),
                                          out,
                                          outbuf,
                                          pad_block,
                                          block_size);
               ec != error_code::ok)
            {
                return ec;
            }
        } else if(opt.pad_style == padding::no_padding)
        {
            if(pending_len != block_size)
                return error_code::invalid_padding;

            if(auto ec = evp_update_write(ctx.get(),
                                          out,
                                          outbuf,
                                          pending.data(),
                                          block_size);
               ec != error_code::ok)
            {
                return ec;
            }
        } else if(pending_len == block_size)
        {
            if(auto ec = evp_update_write(ctx.get(),
                                          out,
                                          outbuf,
                                          pending.data(),
                                          block_size);
               ec != error_code::ok)
            {
                return ec;
            }

            unsigned char pad_block[block_size]{};

            make_padding_block(pad_block, 0, nullptr, opt.pad_style);

            if(auto ec = evp_update_write(ctx.get(),
                                          out,
                                          outbuf,
                                          pad_block,
                                          block_size);
               ec != error_code::ok)
            {
                return ec;
            }
        } else
        {
            unsigned char pad_block[block_size]{};

            make_padding_block(pad_block,
                               pending_len,
                               pending.data(),
                               opt.pad_style);

            if(auto ec = evp_update_write(ctx.get(),
                                          out,
                                          outbuf,
                                          pad_block,
                                          block_size);
               ec != error_code::ok)
            {
                return ec;
            }
        }

        int final_len = 0;

        if(EVP_EncryptFinal_ex(ctx.get(), outbuf.data(), &final_len) != 1)
        {
            return error_code::encrypt_failed;
        }

        if(final_len > 0)
        {
            out.write(reinterpret_cast<const char *>(outbuf.data()), final_len);
        }

        return out ? error_code::ok : error_code::file_io_failed;
    }

    static error_code encrypt_file(const char    *dst_file_path,
                                   const char    *src_file_path,
                                   const options &opt)
    {
        if(!dst_file_path || !src_file_path)
            return error_code::invalid_input;

        std::ifstream src(src_file_path, std::ios::binary);

        if(!src)
            return error_code::file_io_failed;

        std::ofstream dst(dst_file_path, std::ios::binary | std::ios::trunc);

        if(!dst)
            return error_code::file_io_failed;

        return encrypt(dst, src, opt);
    }

    static error_code encrypt_file(const std::string &dst_file_path,
                                   const std::string &src_file_path,
                                   const options     &opt)
    {
        return encrypt_file(dst_file_path.c_str(), src_file_path.c_str(), opt);
    }

    static error_code decrypt(unsigned char       *dst,
                              std::size_t          dst_capacity,
                              std::size_t         &dst_len,
                              const unsigned char *src,
                              std::size_t          src_len,
                              const options       &opt)
    {
        dst_len = 0;

        if(!dst && dst_capacity != 0)
            return error_code::invalid_output;

        if(!is_key_valid(opt.key, opt.key_len))
            return error_code::invalid_key;

        if(!is_iv_valid(opt.mod, opt.iv, opt.iv_len))
            return error_code::invalid_iv;

        if(src_len == 0 || !src)
            return error_code::invalid_input;

        if(!is_ciphertext_valid(src_len, opt.mod, opt.pad_style))
        {
            return error_code::invalid_input;
        }

        if(opt.mod == mode::ctr)
        {
            if(opt.pad_style != padding::no_padding)
                return error_code::invalid_padding;

            return crypt_ctr(dst,
                             dst_capacity,
                             dst_len,
                             src,
                             src_len,
                             opt,
                             true);
        }

        evp_ctx_ptr ctx(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);

        if(!ctx)
            return error_code::decrypt_failed;

        auto cipher = select_cipher(opt);

        if(!cipher)
            return error_code::unsupported_algorithm;

        if(EVP_DecryptInit_ex(ctx.get(),
                              cipher.get(),
                              nullptr,
                              opt.key,
                              select_iv(opt))
               != 1
           || EVP_CIPHER_CTX_set_padding(ctx.get(), 0) != 1)
        {
            return error_code::decrypt_failed;
        }

        if(dst_capacity < src_len)
            return error_code::buffer_too_small;

        if(!dst)
            return error_code::buffer_too_small;

        if(checked_int(src_len) < 0)
            return error_code::invalid_input;

        int out_len = 0;

        if(EVP_DecryptUpdate(ctx.get(),
                             dst,
                             &out_len,
                             src,
                             checked_int(src_len))
           != 1)
        {
            return error_code::decrypt_failed;
        }

        int final_len = 0;

        if(EVP_DecryptFinal_ex(ctx.get(), dst + out_len, &final_len) != 1)
        {
            return error_code::decrypt_failed;
        }

        dst_len = static_cast<std::size_t>(out_len + final_len);

        if(opt.pad_style != padding::no_padding)
        {
            if(!remove_padding(dst, dst_len, opt.pad_style))
            {
                secure_clear(dst, dst_len);
                dst_len = 0;

                return error_code::invalid_padding;
            }
        }

        return error_code::ok;
    }

    static error_code decrypt(unsigned char       *dst,
                              std::size_t         &dst_len,
                              const unsigned char *src,
                              std::size_t          src_len,
                              const options       &opt)
    {
        const std::size_t capacity = decrypt_len_reserve(src_len);

        return decrypt(dst, capacity, dst_len, src, src_len, opt);
    }

    static error_code
    decrypt(std::string &dst, const std::string &src, const options &opt)
    {
        if(src.empty())
        {
            dst.clear();
            return error_code::invalid_input;
        }

        dst.resize(decrypt_len_reserve(src.size()));

        std::size_t dst_len = 0;

        auto ec = decrypt(reinterpret_cast<unsigned char *>(dst.data()),
                          dst.size(),
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

        if(!is_key_valid(opt.key, opt.key_len))
            return error_code::invalid_key;

        if(!is_iv_valid(opt.mod, opt.iv, opt.iv_len))
            return error_code::invalid_iv;

        if(opt.mod == mode::ctr)
        {
            if(opt.pad_style != padding::no_padding)
                return error_code::invalid_padding;

            return crypt_ctr_stream(out, in, opt, true);
        }

        evp_ctx_ptr ctx(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);

        if(!ctx)
            return error_code::decrypt_failed;

        auto cipher = select_cipher(opt);

        if(!cipher)
            return error_code::unsupported_algorithm;

        if(EVP_DecryptInit_ex(ctx.get(),
                              cipher.get(),
                              nullptr,
                              opt.key,
                              select_iv(opt))
               != 1
           || EVP_CIPHER_CTX_set_padding(ctx.get(), 0) != 1)
        {
            return error_code::decrypt_failed;
        }

        std::array<unsigned char, block_size> pending_cipher{};

        std::array<unsigned char, block_size> input{};

        std::array<unsigned char, block_size + EVP_MAX_BLOCK_LENGTH> plainbuf{};

        bool have_pending = false;

        while(true)
        {
            in.read(reinterpret_cast<char *>(input.data()),
                    static_cast<std::streamsize>(block_size));

            const std::streamsize n = in.gcount();

            if(n == 0)
            {
                if(in.bad())
                    return error_code::file_io_failed;

                break;
            }

            if(n != static_cast<std::streamsize>(block_size))
                return error_code::invalid_padding;

            if(have_pending)
            {
                int out_len = 0;

                if(EVP_DecryptUpdate(ctx.get(),
                                     plainbuf.data(),
                                     &out_len,
                                     pending_cipher.data(),
                                     block_size)
                   != 1)
                {
                    return error_code::decrypt_failed;
                }

                if(out_len)
                {
                    out.write(reinterpret_cast<const char *>(plainbuf.data()),
                              out_len);
                }

                if(!out)
                    return error_code::file_io_failed;
            }

            std::memcpy(pending_cipher.data(), input.data(), block_size);

            have_pending = true;
        }

        if(!have_pending)
            return error_code::invalid_input;

        int final_plain_len = 0;

        if(EVP_DecryptUpdate(ctx.get(),
                             plainbuf.data(),
                             &final_plain_len,
                             pending_cipher.data(),
                             block_size)
           != 1)
        {
            return error_code::decrypt_failed;
        }

        int final_len = 0;

        if(EVP_DecryptFinal_ex(ctx.get(),
                               plainbuf.data() + final_plain_len,
                               &final_len)
           != 1)
        {
            return error_code::decrypt_failed;
        }

        std::size_t total_final =
            static_cast<std::size_t>(final_plain_len + final_len);

        if(opt.pad_style != padding::no_padding)
        {
            if(!remove_padding(plainbuf.data(), total_final, opt.pad_style))
            {
                return error_code::invalid_padding;
            }
        }

        out.write(reinterpret_cast<const char *>(plainbuf.data()),
                  static_cast<std::streamsize>(total_final));

        return out ? error_code::ok : error_code::file_io_failed;
    }

    static error_code decrypt_file(const char    *dst_file_path,
                                   const char    *src_file_path,
                                   const options &opt)
    {
        if(!dst_file_path || !src_file_path)
            return error_code::invalid_input;

        std::ifstream src(src_file_path, std::ios::binary);

        if(!src)
            return error_code::file_io_failed;

        std::ofstream dst(dst_file_path, std::ios::binary | std::ios::trunc);

        if(!dst)
            return error_code::file_io_failed;

        return decrypt(dst, src, opt);
    }

    static error_code decrypt_file(const std::string &dst_file_path,
                                   const std::string &src_file_path,
                                   const options     &opt)
    {
        return decrypt_file(dst_file_path.c_str(), src_file_path.c_str(), opt);
    }

    static std::size_t encrypt_len_reserve(std::size_t src_len)
    {
        return src_len + block_size;
    }

    static std::size_t decrypt_len_reserve(std::size_t src_len)
    {
        return src_len;
    }

  private:
    using evp_ctx_ptr =
        std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;

    using evp_cipher_ptr =
        std::unique_ptr<EVP_CIPHER, decltype(&EVP_CIPHER_free)>;

    /*
     * CTR context helper to manage state and keystream generation consistently.
     */
    struct ctr_context
    {
        evp_ctx_ptr                           ctx{nullptr, EVP_CIPHER_CTX_free};
        std::array<unsigned char, block_size> counter{};

        ctr_context() = default;

        static bool create(ctr_context &out_ctr, const options &opt)
        {
            evp_ctx_ptr ctx(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
            if(!ctx)
                return false;

            auto cipher = select_cipher(opt);
            if(!cipher)
                return false;

            if(EVP_EncryptInit_ex(ctx.get(),
                                  cipher.get(),
                                  nullptr,
                                  opt.key,
                                  nullptr)
                   != 1
               || EVP_CIPHER_CTX_set_padding(ctx.get(), 0) != 1)
            {
                return false;
            }

            out_ctr.ctx = std::move(ctx);
            std::memcpy(out_ctr.counter.data(), opt.iv, block_size);
            return true;
        }

        bool
        update(unsigned char *dst, const unsigned char *src, std::size_t len)
        {
            std::array<unsigned char, block_size> stream{};
            std::size_t                           offset = 0;

            while(offset < len)
            {
                int generated = 0;

                if(EVP_EncryptUpdate(ctx.get(),
                                     stream.data(),
                                     &generated,
                                     counter.data(),
                                     static_cast<int>(block_size))
                       != 1
                   || generated != static_cast<int>(block_size))
                {
                    return false;
                }

                const std::size_t n = (std::min) (block_size, len - offset);

                for(std::size_t i = 0; i < n; ++i)
                {
                    dst[offset + i] = src[offset + i] ^ stream[i];
                }

                increment_counter(counter);
                offset += n;
            }

            return true;
        }
    };

    static int checked_int(std::size_t n)
    {
        return n > static_cast<std::size_t>(INT_MAX) ? -1 : static_cast<int>(n);
    }

    static bool is_key_valid(const unsigned char *key, std::size_t key_len)
    {
        return key != nullptr
               && (key_len == 8 || key_len == 16 || key_len == 24);
    }

    static bool
    is_iv_valid(mode mod, const unsigned char *iv, std::size_t iv_len)
    {
        if(mod == mode::ecb)
            return iv == nullptr && iv_len == 0;

        return iv != nullptr && iv_len == block_size;
    }

    static bool is_block_mode(mode mod)
    {
        return mod == mode::ecb || mod == mode::cbc;
    }

    static bool is_plain_valid(const unsigned char *src,
                               std::size_t          src_len,
                               mode                 mod,
                               padding              pad)
    {
        if(src_len != 0 && !src)
            return false;

        /*
         * CTR is stream mode and allows arbitrary length.
         * For block modes with no_padding, block alignment is required.
         */
        if(mod != mode::ctr && pad == padding::no_padding
           && (src_len % block_size) != 0)
        {
            return false;
        }

        return true;
    }

    static bool is_ciphertext_valid(std::size_t src_len, mode mod, padding pad)
    {
        if(src_len == 0)
            return false;

        /*
         * CTR mode allows arbitrary length ciphertexts.
         */
        if(mod == mode::ctr)
            return true;

        if(src_len % block_size != 0)
            return false;

        if(pad != padding::no_padding && src_len < block_size)
        {
            return false;
        }

        return true;
    }

    static const unsigned char *select_iv(const options &opt)
    {
        return opt.mod == mode::ecb ? nullptr : opt.iv;
    }

    static evp_cipher_ptr select_cipher(const options &opt)
    {
        const char *name = nullptr;

        switch(opt.key_len)
        {
            case 8:
                return evp_cipher_ptr(nullptr, EVP_CIPHER_free);

            case 16:
                switch(opt.mod)
                {
                    case mode::ecb:
                    case mode::ctr:
                        name = "DES-EDE-ECB";
                        break;

                    case mode::cbc:
                        name = "DES-EDE-CBC";
                        break;

                    case mode::cfb:
                        name = "DES-EDE-CFB";
                        break;

                    case mode::ofb:
                        name = "DES-EDE-OFB";
                        break;
                }
                break;

            case 24:
                switch(opt.mod)
                {
                    case mode::ecb:
                    case mode::ctr:
                        name = "DES-EDE3-ECB";
                        break;

                    case mode::cbc:
                        name = "DES-EDE3-CBC";
                        break;

                    case mode::cfb:
                        name = "DES-EDE3-CFB";
                        break;

                    case mode::ofb:
                        name = "DES-EDE3-OFB";
                        break;
                }
                break;

            default:
                return evp_cipher_ptr(nullptr, EVP_CIPHER_free);
        }

        if(!name)
        {
            return evp_cipher_ptr(nullptr, EVP_CIPHER_free);
        }

        return evp_cipher_ptr(EVP_CIPHER_fetch(nullptr, name, nullptr),
                              EVP_CIPHER_free);
    }

    static bool is_padding_mode(padding p) { return p != padding::no_padding; }

    static bool build_padded_input(std::vector<unsigned char> &dst,
                                   const unsigned char        *src,
                                   std::size_t                 src_len,
                                   padding                     pad)
    {
        if(!src && src_len != 0)
            return false;

        if(pad == padding::no_padding)
        {
            dst.resize(src_len);

            if(src_len)
                std::memcpy(dst.data(), src, src_len);

            return true;
        }

        const std::size_t rem = src_len % block_size;

        const std::size_t pad_len = block_size - rem;

        dst.resize(src_len + pad_len);

        const std::size_t final_src_len = rem;

        const std::size_t final_offset = src_len - final_src_len;

        if(src_len)
        {
            std::memcpy(dst.data(), src, src_len);
        }

        make_padding_block(dst.data() + final_offset,
                           final_src_len,
                           final_src_len ? src + final_offset : nullptr,
                           pad);

        return true;
    }

    static void make_padding_block(unsigned char       *dst,
                                   std::size_t          src_len,
                                   const unsigned char *src,
                                   padding              pad)
    {
        std::memset(dst, 0, block_size);

        if(src && src_len)
        {
            std::memcpy(dst, src, src_len);
        }

        const unsigned char pad_len =
            static_cast<unsigned char>(block_size - src_len);

        switch(pad)
        {
            case padding::pkcs5:
            case padding::pkcs7:
                for(std::size_t i = src_len; i < block_size; ++i)
                {
                    dst[i] = pad_len;
                }
                break;

            case padding::iso10126: {
                std::random_device rd;

                for(std::size_t i = src_len; i + 1 < block_size; ++i)
                {
                    dst[i] = static_cast<unsigned char>(rd());
                }

                dst[block_size - 1] = pad_len;
                break;
            }

            case padding::ansix923:
                dst[block_size - 1] = pad_len;
                break;

            case padding::iso_iec_7816_4:
                if(src_len < block_size)
                    dst[src_len] = 0x80;
                break;

            case padding::zero:
            case padding::no_padding:
                break;
        }
    }

    static bool
    remove_padding(unsigned char *buf, std::size_t &len, padding pad)
    {
        if(pad == padding::no_padding)
            return true;

        if(!buf || len == 0 || len % block_size != 0)
        {
            return false;
        }

        switch(pad)
        {
            case padding::pkcs5:
            case padding::pkcs7: {
                const unsigned char n = buf[len - 1];

                if(n == 0 || n > block_size || n > len)
                {
                    return false;
                }

                for(std::size_t i = len - n; i < len; ++i)
                {
                    if(buf[i] != n)
                        return false;
                }

                len -= n;

                secure_clear(buf + len, n);

                return true;
            }

            case padding::iso10126: {
                const unsigned char n = buf[len - 1];

                if(n == 0 || n > block_size || n > len)
                {
                    return false;
                }

                len -= n;

                secure_clear(buf + len, n);

                return true;
            }

            case padding::ansix923: {
                const unsigned char n = buf[len - 1];

                if(n == 0 || n > block_size || n > len)
                {
                    return false;
                }

                for(std::size_t i = len - n; i + 1 < len; ++i)
                {
                    if(buf[i] != 0)
                        return false;
                }

                len -= n;

                secure_clear(buf + len, n);

                return true;
            }

            case padding::iso_iec_7816_4: {
                std::size_t i = len;

                while(i > 0 && buf[i - 1] == 0)
                {
                    --i;
                }

                if(i == 0 || buf[i - 1] != 0x80)
                {
                    return false;
                }

                --i;

                secure_clear(buf + i, len - i);

                len = i;

                return true;
            }

            case padding::zero:
                while(len > 0 && buf[len - 1] == 0)
                {
                    --len;
                }

                return true;

            case padding::no_padding:
                return true;
        }

        return false;
    }

    static error_code evp_update_write(
        EVP_CIPHER_CTX                                               *ctx,
        std::ostream                                                 &out,
        std::array<unsigned char, block_size + EVP_MAX_BLOCK_LENGTH> &buf,
        const unsigned char                                          *src,
        std::size_t                                                   src_len)
    {
        if(checked_int(src_len) < 0)
            return error_code::invalid_input;

        int out_len = 0;

        if(EVP_EncryptUpdate(ctx,
                             buf.data(),
                             &out_len,
                             src,
                             checked_int(src_len))
           != 1)
        {
            return error_code::encrypt_failed;
        }

        if(out_len > 0)
        {
            out.write(reinterpret_cast<const char *>(buf.data()), out_len);
        }

        return out ? error_code::ok : error_code::file_io_failed;
    }

    static error_code crypt_ctr(unsigned char       *dst,
                                std::size_t          dst_capacity,
                                std::size_t         &dst_len,
                                const unsigned char *src,
                                std::size_t          src_len,
                                const options       &opt,
                                bool                 decrypting)
    {
        if(dst_capacity < src_len)
            return error_code::buffer_too_small;

        if(src_len != 0 && (!dst || !src))
            return error_code::invalid_input;

        ctr_context ctr;

        if(!ctr_context::create(ctr, opt))
        {
            return decrypting ? error_code::decrypt_failed
                              : error_code::encrypt_failed;
        }

        if(!ctr.update(dst, src, src_len))
        {
            return decrypting ? error_code::decrypt_failed
                              : error_code::encrypt_failed;
        }

        dst_len = src_len;
        return error_code::ok;
    }

    static error_code crypt_ctr_stream(std::ostream  &out,
                                       std::istream  &in,
                                       const options &opt,
                                       bool           decrypting)
    {
        ctr_context ctr;

        if(!ctr_context::create(ctr, opt))
        {
            return decrypting ? error_code::decrypt_failed
                              : error_code::encrypt_failed;
        }

        std::array<unsigned char, 4096> in_buf{};
        std::array<unsigned char, 4096> out_buf{};

        while(true)
        {
            in.read(reinterpret_cast<char *>(in_buf.data()),
                    static_cast<std::streamsize>(in_buf.size()));

            const std::streamsize n = in.gcount();

            if(n == 0)
            {
                if(in.bad())
                    return error_code::file_io_failed;

                break;
            }

            const std::size_t nbytes = static_cast<std::size_t>(n);

            if(!ctr.update(out_buf.data(), in_buf.data(), nbytes))
            {
                return decrypting ? error_code::decrypt_failed
                                  : error_code::encrypt_failed;
            }

            out.write(reinterpret_cast<const char *>(out_buf.data()),
                      static_cast<std::streamsize>(nbytes));

            if(!out)
                return error_code::file_io_failed;
        }

        return error_code::ok;
    }

    static void
    increment_counter(std::array<unsigned char, block_size> &counter)
    {
        for(std::size_t i = block_size; i-- > 0;)
        {
            if(++counter[i] != 0)
                break;
        }
    }

    static void secure_clear(void *ptr, std::size_t len)
    {
        volatile unsigned char *p = static_cast<volatile unsigned char *>(ptr);

        while(len--)
            *p++ = 0;
    }

    des()                       = default;
    ~des()                      = default;
    des(const des &)            = delete;
    des &operator=(const des &) = delete;
    des(des &&)                 = delete;
    des &operator=(des &&)      = delete;
};

} // namespace hj

#endif // DES_HPP