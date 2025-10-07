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

// disable msvc safe check warning
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

// support deprecated api for low version openssl
#ifndef OPENSSL_SUPPRESS_DEPRECATED
#define OPENSSL_SUPPRESS_DEPRECATED
#endif

#include <string.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <array>

#include <openssl/md5.h>

#ifndef MD5_BUF_SZ
#define MD5_BUF_SZ 128 * 1024
#endif

namespace hj
{

// because of unidirectionality, md5 not support decode
class md5
{
  public:
    enum class error_code
    {
        ok = 0,
        invalid_input,
        invalid_output,
        buffer_too_small,
        unknown
    };

    static error_code encode(unsigned char       *dst,
                             std::size_t         &dst_len,
                             const unsigned char *src,
                             const std::size_t    src_len)
    {
        if(dst_len < MD5_DIGEST_LENGTH)
            return error_code::buffer_too_small;

        MD5_CTX ctx;
        MD5_Init(&ctx);
        MD5_Update(&ctx, src, src_len);
        MD5_Final(reinterpret_cast<unsigned char *>(dst), &ctx);
        dst_len = MD5_DIGEST_LENGTH;
        memset(&ctx, 0, sizeof(ctx));
        return error_code::ok;
    }

    static error_code encode(std::string &dst, const std::string &src)
    {
        dst.resize(MD5_DIGEST_LENGTH);
        std::size_t len = dst.size();
        auto        ec  = encode(
            reinterpret_cast<unsigned char *>(const_cast<char *>(dst.data())),
            len,
            reinterpret_cast<const unsigned char *>(src.c_str()),
            src.size());
        if(ec != error_code::ok)
        {
            dst.clear();
            return ec;
        }

        dst.resize(len);
        return error_code::ok;
    }

    static error_code encode(std::array<uint8_t, MD5_DIGEST_LENGTH> &dst,
                             const std::string                      &src)
    {
        std::size_t len = dst.size();
        auto        ec  = encode(dst.data(),
                         len,
                         reinterpret_cast<const unsigned char *>(src.c_str()),
                         src.size());
        if(ec != error_code::ok)
        {
            dst.fill(0);
            return ec;
        }

        return error_code::ok;
    }

    static error_code encode(std::string &out, std::istream &in)
    {
        if(!in.good())
        {
            out.clear();
            return error_code::invalid_input;
        }

        MD5_CTX ctx;
        MD5_Init(&ctx);
        std::streamsize   sz;
        std::vector<char> buffer(MD5_BUF_SZ);
        while((sz = in.read(&buffer[0], MD5_BUF_SZ).gcount()) > 0)
            MD5_Update(&ctx, buffer.data(), static_cast<std::size_t>(sz));

        out.resize(MD5_DIGEST_LENGTH);
        MD5_Final(reinterpret_cast<unsigned char *>(&out[0]), &ctx);
        memset(&ctx, 0, sizeof(ctx));
        return error_code::ok;
    }

    static error_code encode(std::ostream &out, std::istream &in)
    {
        if(!in.good())
            return error_code::invalid_input;
        if(!out.good())
            return error_code::invalid_output;

        MD5_CTX ctx;
        MD5_Init(&ctx);
        std::vector<char> buffer(MD5_BUF_SZ);
        std::streamsize   sz;
        while((sz = in.read(buffer.data(), MD5_BUF_SZ).gcount()) > 0)
            MD5_Update(&ctx, buffer.data(), static_cast<std::size_t>(sz));

        unsigned char md[MD5_DIGEST_LENGTH] = {0};
        MD5_Final(md, &ctx);
        memset(&ctx, 0, sizeof(ctx));

        out.write(reinterpret_cast<const char *>(md), MD5_DIGEST_LENGTH);
        out.flush();
        memset(md, 0, sizeof(md));
        return error_code::ok;
    }

    // encode: file -> file
    static error_code encode_file(const char *dst_file_path,
                                  const char *src_file_path)
    {
        std::ifstream in(src_file_path, std::ios::binary);
        if(!in.is_open())
            return error_code::invalid_input;

        std::ofstream out(dst_file_path, std::ios::binary);
        if(!out.is_open())
            return error_code::invalid_output;

        return encode(out, in);
    }

    // encode: file -> file
    static error_code encode_file(const std::string &dst_file_path,
                                  const std::string &src_file_path)
    {
        return encode_file(dst_file_path.c_str(), src_file_path.c_str());
    }

    static std::string to_hex(const std::string &str, bool upper_case = false)
    {
        std::stringstream ss;
        for(size_t i = 0; i < MD5_DIGEST_LENGTH && i < str.size(); ++i)
        {
            if(upper_case)
                ss << std::uppercase;
            else
                ss << std::nouppercase;
            ss << std::hex << std::setw(2) << std::setfill('0')
               << static_cast<unsigned int>(static_cast<unsigned char>(str[i]));
        }

        return ss.str();
    };

    // reserve encode dst buf size
    static std::size_t encode_len_reserve() { return MD5_DIGEST_LENGTH; }

  private:
    md5()                       = default;
    ~md5()                      = default;
    md5(const md5 &)            = delete;
    md5 &operator=(const md5 &) = delete;
    md5(md5 &&)                 = delete;
    md5 &operator=(md5 &&)      = delete;
};

}

#endif