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

#ifndef MD5_HPP
#define MD5_HPP

#include <openssl/evp.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace hj
{

class md5
{
  public:
    static constexpr std::size_t buf_sz        = 131072; // 128KB
    static constexpr std::size_t digest_length = 16;
    using digest_type = std::array<uint8_t, digest_length>;

    enum class error_code
    {
        ok = 0,
        invalid_input,
        invalid_output,
        buffer_too_small,
        crypto_error,
        unknown
    };

    // RAII Context Struct
    using evp_ctx_ptr = std::unique_ptr<EVP_MD_CTX, void (*)(EVP_MD_CTX *)>;

    [[nodiscard]] static evp_ctx_ptr make_ctx()
    {
        return evp_ctx_ptr(EVP_MD_CTX_new(), EVP_MD_CTX_free);
    }

    [[nodiscard]] static error_code encode(uint8_t          *dst,
                                           std::size_t      &dst_len,
                                           const uint8_t    *src,
                                           const std::size_t src_len)
    {
        if(dst_len < digest_length)
            return error_code::buffer_too_small;

        auto ctx = make_ctx();
        if(!ctx)
            return error_code::crypto_error;

        if(EVP_DigestInit_ex(ctx.get(), EVP_md5(), nullptr) != 1
           || EVP_DigestUpdate(ctx.get(), src, src_len) != 1
           || EVP_DigestFinal_ex(ctx.get(), dst, nullptr) != 1)
        {
            return error_code::crypto_error;
        }

        dst_len = digest_length;
        return error_code::ok;
    }

    [[nodiscard]] static error_code encode(digest_type           &dst,
                                           const std::string_view src)
    {
        std::size_t len = dst.size();
        auto        ec  = encode(dst.data(),
                                 len,
                                 reinterpret_cast<const uint8_t *>(src.data()),
                                 src.size());
        if(ec != error_code::ok)
        {
            dst.fill(0);
            return ec;
        }
        return error_code::ok;
    }

    [[nodiscard]] static error_code encode(digest_type &dst, std::istream &in)
    {
        if(!in.good())
        {
            dst.fill(0);
            return error_code::invalid_input;
        }

        auto ctx = make_ctx();
        if(!ctx)
            return error_code::crypto_error;

        if(EVP_DigestInit_ex(ctx.get(), EVP_md5(), nullptr) != 1)
            return error_code::crypto_error;

        std::streamsize   sz;
        std::vector<char> buffer(buf_sz);
        while((sz = in.read(buffer.data(), buf_sz).gcount()) > 0)
        {
            if(EVP_DigestUpdate(ctx.get(),
                                buffer.data(),
                                static_cast<std::size_t>(sz))
               != 1)
            {
                dst.fill(0);
                return error_code::crypto_error;
            }
        }

        if(EVP_DigestFinal_ex(ctx.get(), dst.data(), nullptr) != 1)
        {
            dst.fill(0);
            return error_code::crypto_error;
        }

        return error_code::ok;
    }

    [[nodiscard]] static error_code encode(std::ostream &out, std::istream &in)
    {
        if(!in.good())
            return error_code::invalid_input;
        if(!out.good())
            return error_code::invalid_output;

        digest_type md;
        auto        ec = encode(md, in);
        if(ec != error_code::ok)
            return ec;

        out.write(reinterpret_cast<const char *>(md.data()), md.size());
        out.flush();
        md.fill(0);
        return error_code::ok;
    }

    [[nodiscard]] static error_code
    encode_file(const std::string &dst_file_path,
                const std::string &src_file_path)
    {
        std::ifstream in(src_file_path, std::ios::binary);
        if(!in.is_open())
            return error_code::invalid_input;

        std::ofstream out(dst_file_path, std::ios::binary);
        if(!out.is_open())
            return error_code::invalid_output;

        return encode(out, in);
    }

    [[nodiscard]] static std::string to_hex(const digest_type &digest,
                                            bool upper_case = false)
    {
        static constexpr char hex_lower[] = "0123456789abcdef";
        static constexpr char hex_upper[] = "0123456789ABCDEF";
        const char           *lut         = upper_case ? hex_upper : hex_lower;

        std::string hex;
        hex.resize(digest_length * 2);
        for(std::size_t i = 0; i < digest_length; ++i)
        {
            hex[i * 2]     = lut[(digest[i] >> 4) & 0x0F];
            hex[i * 2 + 1] = lut[digest[i] & 0x0F];
        }
        return hex;
    }

    [[nodiscard]] static error_code encode_hex(std::string           &hex_out,
                                               const std::string_view src,
                                               bool upper_case = false)
    {
        digest_type md;
        auto        ec = encode(md, src);
        if(ec != error_code::ok)
        {
            hex_out.clear();
            return ec;
        }
        hex_out = to_hex(md, upper_case);
        return error_code::ok;
    }

    [[nodiscard]] static error_code
    encode_hex(std::string &hex_out, std::istream &in, bool upper_case = false)
    {
        digest_type md;
        auto        ec = encode(md, in);
        if(ec != error_code::ok)
        {
            hex_out.clear();
            return ec;
        }
        hex_out = to_hex(md, upper_case);
        return error_code::ok;
    }

    [[nodiscard]] static constexpr std::size_t encode_len_reserve() noexcept
    {
        return digest_length;
    }

  private:
    md5()                       = delete;
    ~md5()                      = delete;
    md5(const md5 &)            = delete;
    md5 &operator=(const md5 &) = delete;
    md5(md5 &&)                 = delete;
    md5 &operator=(md5 &&)      = delete;
};

class md5_category_impl : public std::error_category
{
  public:
    [[nodiscard]] const char *name() const noexcept override
    {
        return "hj::md5";
    }

    [[nodiscard]] std::string message(int ev) const override
    {
        switch(static_cast<md5::error_code>(ev))
        {
            case md5::error_code::ok:
                return "Success";
            case md5::error_code::invalid_input:
                return "Invalid input stream or source";
            case md5::error_code::invalid_output:
                return "Invalid output stream or destination";
            case md5::error_code::buffer_too_small:
                return "Destination buffer too small";
            case md5::error_code::crypto_error:
                return "OpenSSL EVP crypto operation failed";
            case md5::error_code::unknown:
            default:
                return "Unknown error";
        }
    }
};

[[nodiscard]] inline const std::error_category &md5_category() noexcept
{
    static const md5_category_impl instance;
    return instance;
}

[[nodiscard]] inline std::error_code make_error_code(md5::error_code e) noexcept
{
    return {static_cast<int>(e), md5_category()};
}

} // namespace hj

template <>
struct std::is_error_code_enum<hj::md5::error_code> : std::true_type
{
};

#endif // MD5_HPP