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
    static bool encode(unsigned char* dst, 
                       std::size_t& dst_len,
                       const unsigned char* src, 
                       const std::size_t src_len)
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

    static bool encode(std::string& dst,
                       const std::string& src)
    {
        dst.resize(MD5_DIGEST_LENGTH);
        std::size_t len = dst.size();
        if (!encode(reinterpret_cast<unsigned char*>(const_cast<char*>(dst.data())), 
                    len,
                    reinterpret_cast<const unsigned char*>(src.c_str()), 
                    src.size()))
            return false;

        dst.resize(len);
        return true;
    };

    static bool encode(std::string& out,
                       std::istream &in)
    {
        if (!in.good())
            return false;

        MD5_CTX ctx;
        MD5_Init(&ctx);
        std::streamsize sz;
        std::vector<char> buffer(MD5_BUF_SZ);
        while ((sz = in.read(&buffer[0], MD5_BUF_SZ).gcount()) > 0)
            MD5_Update(&ctx, buffer.data(), static_cast<std::size_t>(sz));

        out.resize(128 / 8);
        MD5_Final(reinterpret_cast<unsigned char*>(&out[0]), &ctx);
    };

    static bool encode(std::ostream& out, 
                       std::istream& in)
    {
        if (!in.good() || !out.good())
            return false;

        MD5_CTX ctx;
        MD5_Init(&ctx);
        std::vector<char> buffer(MD5_BUF_SZ);
        std::streamsize sz;
        while ((sz = in.read(buffer.data(), MD5_BUF_SZ).gcount()) > 0)
            MD5_Update(&ctx, buffer.data(), static_cast<std::size_t>(sz));

        unsigned char md[MD5_DIGEST_LENGTH];
        MD5_Final(md, &ctx);

        out.write(reinterpret_cast<const char*>(md), MD5_DIGEST_LENGTH);
        out.flush();
        return true;
    };

    // file -> encode string
    static bool encode_file(std::string& dst,
                            const char* src_file_path)
    {
        std::size_t dst_len = MD5_DIGEST_LENGTH;
        dst.resize(dst_len);
        return encode_file(const_cast<char*>(dst.data()), dst_len, src_file_path);
    }

    // file -> encode char*
    static bool encode_file(char* dst,
                            std::size_t& dst_len,
                            const char* src_file_path)
    {
        std::ifstream src_file(src_file_path, std::ios::binary);
        if (!src_file.is_open())
            return false;

        std::ostringstream dst_stream;
        if (!encode(dst_stream, src_file))
            return false;

        std::string dst_str = dst_stream.str();
        dst_len = dst_str.size();
        if (dst_len > MD5_DIGEST_LENGTH)
            return false;

        memcpy(dst, dst_str.data(), dst_len);
        return true;
    }

    static std::string to_hex(const std::string& str, bool upper_case = false)
    {
        std::stringstream ss;
        for (size_t i = 0; i < MD5_DIGEST_LENGTH && i < str.size(); ++i) 
        {
            if (upper_case)
                ss << std::uppercase;
            else
                ss << std::nouppercase;
            ss << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<unsigned int>(static_cast<unsigned char>(str[i]));
        }

        return ss.str();
    };

    // reserve encode dst buf size
	static std::size_t encode_len_reserve()
	{
		return MD5_DIGEST_LENGTH;
	}

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