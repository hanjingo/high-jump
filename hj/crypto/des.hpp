/*
 * This file is part of high-jump(hj).
 * Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 *
 * GNU General Public License v3.0 or later.
 */

#ifndef DES_HPP
#define DES_HPP

#include <array>
#include <cstddef>
#include <climits>
#include <algorithm>
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

        std::vector<unsigned char> input;

        if(!build_padded_input(input, src, src_len, opt.pad_style))
            return error_code::invalid_plain;

        const std::size_t required = input.size();

        if(dst_capacity < required)
            return error_code::buffer_too_small;

        if(required != 0 && !dst)
            return error_code::buffer_too_small;

        /*
         * CTR is implemented explicitly using ECB as the block primitive.
         * OpenSSL does not provide a generic DES-EDE-CTR cipher name.
 * CTR is therefore constructed from the EVP ECB primitive.
         */
        if(opt.mod == mode::ctr)
        {
            return crypt_ctr(dst,
                             dst_capacity,
                             dst_len,
                             input.data(),
                             input.size(),
                             opt,
                             false);
        }

        evp_ctx_ptr ctx(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);

        if(!ctx)
            return error_code::encrypt_failed;

        auto cipher = select_cipher(opt);

        /*
         * 8-byte DES intentionally returns nullptr here.
         * We do NOT load the OpenSSL legacy provider.
         */
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

        /*
         * Padding is handled by this wrapper because it supports
         * several padding schemes that EVP does not expose uniformly.
         */
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

    /*
     * Backward-compatible raw-buffer overload.
     *
     * Caller must reserve encrypt_len_reserve(src_len) bytes.
     */
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

    /*
     * Stream encryption.
     *
     * Important:
     * no_padding always requires the total input length to be
     * a multiple of block_size, regardless of cipher mode.
     */
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
            return crypt_ctr_stream(out, in, opt, false);

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

            /*
             * Delay one block so that we know whether it is the
             * final block and therefore whether padding is required.
             */
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
                /*
                 * Empty input is not accepted by the stream API
                 * for no-padding mode.
                 */
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
            /*
             * This applies to ALL modes, including CFB/OFB/CTR.
             */
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
            /*
             * Input length is exactly aligned.
             * PKCS-style padding requires a complete padding block.
             */
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

    /*
     * Backward-compatible raw-buffer overload.
     */
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

    /*
     * Stream decryption.
     *
     * Ciphertext must always be block aligned.
     */
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
            return crypt_ctr_stream(out, in, opt, true);

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

            /*
             * Ciphertext is always required to be block aligned.
             */
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

    static int checked_int(std::size_t n)
    {
        return n > static_cast<std::size_t>(INT_MAX) ? -1 : static_cast<int>(n);
    }

    static bool is_key_valid(const unsigned char *key, std::size_t key_len)
    {
        /*
         * 8 bytes is retained as a valid key size so that
         * callers receive unsupported_algorithm rather than
         * invalid_key.
         *
         * OpenSSL 3 legacy DES is intentionally NOT loaded.
         */
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
                               mode,
                               padding pad)
    {
        if(src_len != 0 && !src)
            return false;

        /*
         * Library contract:
         *
         * no_padding always requires block alignment,
         * regardless of ECB/CBC/CFB/OFB/CTR.
         */
        if(pad == padding::no_padding && (src_len % block_size) != 0)
        {
            return false;
        }

        return true;
    }

    static bool is_ciphertext_valid(std::size_t src_len, mode, padding pad)
    {
        if(src_len == 0)
            return false;

        /*
         * All modes in this wrapper operate on complete
         * DES blocks at the public API boundary.
         */
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
                /*
                 * Single DES is only available through
                 * OpenSSL's legacy provider.
                 *
                 * This implementation intentionally does
                 * NOT load or depend on that provider.
                 */
                return evp_cipher_ptr(nullptr, EVP_CIPHER_free);

            case 16:
                /*
                 * 2-key Triple-DES / TDEA.
                 */
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
                /*
                 * 3-key Triple-DES / TDEA.
                 */
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

    /*
     * CTR implementation.
     *
     * DES-EDE-ECB / DES-EDE3-ECB is used strictly as the
     * block primitive to generate the CTR keystream.
     */
    static error_code crypt_ctr(unsigned char       *dst,
                                std::size_t          dst_capacity,
                                std::size_t         &dst_len,
                                const unsigned char *src,
                                std::size_t          src_len,
                                const options       &opt,
                                bool                 decrypting)
    {
        (void) decrypting;

        if(dst_capacity < src_len)
            return error_code::buffer_too_small;

        if(src_len != 0 && (!dst || !src))
            return error_code::invalid_input;

        evp_ctx_ptr ctx(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);

        if(!ctx)
        {
            return decrypting ? error_code::decrypt_failed
                              : error_code::encrypt_failed;
        }

        auto cipher = select_cipher(opt);

        if(!cipher)
            return error_code::unsupported_algorithm;

        if(EVP_EncryptInit_ex(ctx.get(),
                              cipher.get(),
                              nullptr,
                              opt.key,
                              nullptr)
               != 1
           || EVP_CIPHER_CTX_set_padding(ctx.get(), 0) != 1)
        {
            return decrypting ? error_code::decrypt_failed
                              : error_code::encrypt_failed;
        }

        std::array<unsigned char, block_size> counter{};

        std::memcpy(counter.data(), opt.iv, block_size);

        std::array<unsigned char, block_size> stream{};

        std::size_t offset = 0;

        while(offset < src_len)
        {
            int generated = 0;

            if(EVP_EncryptUpdate(ctx.get(),
                                 stream.data(),
                                 &generated,
                                 counter.data(),
                                 block_size)
                   != 1
               || generated != static_cast<int>(block_size))
            {
                return decrypting ? error_code::decrypt_failed
                                  : error_code::encrypt_failed;
            }

            const std::size_t n = (std::min) (block_size, src_len - offset);

            for(std::size_t i = 0; i < n; ++i)
            {
                dst[offset + i] = src[offset + i] ^ stream[i];
            }

            increment_counter(counter);

            offset += n;
        }

        dst_len = src_len;

        if(decrypting && opt.pad_style != padding::no_padding)
        {
            if(!remove_padding(dst, dst_len, opt.pad_style))
            {
                secure_clear(dst, src_len);

                dst_len = 0;

                return error_code::invalid_padding;
            }
        }

        return error_code::ok;
    }

    static error_code crypt_ctr_stream(std::ostream  &out,
                                       std::istream  &in,
                                       const options &opt,
                                       bool           decrypting)
    {
        evp_ctx_ptr ctx(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);

        if(!ctx)
        {
            return decrypting ? error_code::decrypt_failed
                              : error_code::encrypt_failed;
        }

        auto cipher = select_cipher(opt);

        if(!cipher)
            return error_code::unsupported_algorithm;

        if(EVP_EncryptInit_ex(ctx.get(),
                              cipher.get(),
                              nullptr,
                              opt.key,
                              nullptr)
               != 1
           || EVP_CIPHER_CTX_set_padding(ctx.get(), 0) != 1)
        {
            return decrypting ? error_code::decrypt_failed
                              : error_code::encrypt_failed;
        }

        std::array<unsigned char, block_size> counter{};

        std::array<unsigned char, block_size> input{};

        std::array<unsigned char, block_size> pending{};

        std::array<unsigned char, block_size> stream{};

        std::memcpy(counter.data(), opt.iv, block_size);

        const bool padded = opt.pad_style != padding::no_padding;

        bool have_pending  = false;
        bool processed_any = false;

        std::size_t pending_len = 0;

        auto transform = [&](const unsigned char *src,
                             std::size_t          len) -> error_code {
            int generated = 0;

            if(EVP_EncryptUpdate(ctx.get(),
                                 stream.data(),
                                 &generated,
                                 counter.data(),
                                 block_size)
                   != 1
               || generated != static_cast<int>(block_size))
            {
                return decrypting ? error_code::decrypt_failed
                                  : error_code::encrypt_failed;
            }

            for(std::size_t i = 0; i < len; ++i)
                stream[i] ^= src[i];

            out.write(reinterpret_cast<const char *>(stream.data()),
                      static_cast<std::streamsize>(len));

            if(!out)
                return error_code::file_io_failed;

            increment_counter(counter);

            processed_any = true;

            return error_code::ok;
        };

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
                if(auto ec = transform(pending.data(), block_size);
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
            if(!padded)
            {
                return processed_any ? error_code::ok
                                     : error_code::invalid_input;
            }

            if(decrypting)
                return error_code::invalid_input;

            unsigned char pad_block[block_size]{};

            make_padding_block(pad_block, 0, nullptr, opt.pad_style);

            return transform(pad_block, block_size);
        }

        if(!padded)
        {
            /*
             * CTR also obeys the public no-padding contract:
             * input must be block aligned.
             */
            if(pending_len != block_size)
                return error_code::invalid_padding;

            return transform(pending.data(), block_size);
        }

        if(!decrypting)
        {
            unsigned char pad_block[block_size]{};

            make_padding_block(pad_block,
                               pending_len,
                               pending.data(),
                               opt.pad_style);

            return transform(pad_block, block_size);
        }

        /*
         * For padded CTR decryption, the final block is transformed
         * locally so that padding can be validated before writing it.
         */
        int generated = 0;

        if(EVP_EncryptUpdate(ctx.get(),
                             stream.data(),
                             &generated,
                             counter.data(),
                             block_size)
               != 1
           || generated != static_cast<int>(block_size))
        {
            return error_code::decrypt_failed;
        }

        for(std::size_t i = 0; i < block_size; ++i)
        {
            stream[i] ^= pending[i];
        }

        std::size_t final_len = block_size;

        if(!remove_padding(stream.data(), final_len, opt.pad_style))
        {
            return error_code::invalid_padding;
        }

        out.write(reinterpret_cast<const char *>(stream.data()),
                  static_cast<std::streamsize>(final_len));

        return out ? error_code::ok : error_code::file_io_failed;
    }

    static void
    increment_counter(std::array<unsigned char, block_size> &counter)
    {
        /*
         * Big-endian counter increment.
         */
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