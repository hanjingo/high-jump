/*
 *  This file is part of libcpp.
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

#include <string.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include <openssl/md5.h>

#ifndef MD5_BUF_SZ
#define MD5_BUF_SZ 128 * 1024
#endif

namespace libcpp
{

// because of unidirectionality, md5 not support decode
class md5
{
public:
    enum out_case {
        lower_case,
        upper_case
    };

public:
    explicit md5() {};
    virtual ~md5() {};

    static bool encode(const char* src, 
                       const std::size_t src_len, 
                       char* dst, 
                       std::size_t& dst_len)
    {
        if (dst_len < MD5_DIGEST_LENGTH)
            return false;

        MD5_CTX ctx;
        MD5_Init(&ctx);
        MD5_Update(&ctx, src, src_len);
        MD5_Final(reinterpret_cast<unsigned char*>(dst), &ctx);
        dst_len = MD5_DIGEST_LENGTH;
        return true;
    }

    static bool encode(const std::string& src, 
                       std::string& dst,
                       libcpp::md5::out_case ocase = libcpp::md5::lower_case)
    {
        std::size_t len = MD5_DIGEST_LENGTH;
        char hash[MD5_DIGEST_LENGTH];
        if (!encode(src.c_str(), src.size(), hash, len))
            return false;

        std::stringstream ss;
        for (size_t i = 0; i < MD5_DIGEST_LENGTH; ++i) 
        {
            if (ocase == libcpp::md5::upper_case)
                ss << std::setiosflags(std::ios::uppercase) << std::hex << std::setw(2)
                   << std::setfill('0') << static_cast<int>(hash[i]);
            else
                ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
        }

        dst = ss.str();
        return true;
    };

    static void encode(std::istream &in, std::string& out)
    {
        MD5_CTX ctx;
        MD5_Init(&ctx);
        std::streamsize sz;
        std::vector<char> buffer(MD5_BUF_SZ);
        while ((sz = in.read(&buffer[0], MD5_BUF_SZ).gcount()) > 0)
            MD5_Update(&ctx, buffer.data(), sz);

        out.resize(128 / 8);
        MD5_Final(reinterpret_cast<unsigned char*>(&out[0]), &ctx);
    };

private:
    md5() = default;
    ~md5() = default;
    md5(const md5&) = delete;
    md5& operator=(const md5&) = delete;
    md5(md5&&) = delete;
    md5& operator=(md5&&) = delete;
};

}

#endif