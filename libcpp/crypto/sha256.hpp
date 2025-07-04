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

#ifndef SHA256_HPP
#define SHA256_HPP

// disable msvc safe check warning
#define _CRT_SECURE_NO_WARNINGS

// support deprecated api for low version openssl
#define OPENSSL_SUPPRESS_DEPRECATED

#include <iostream>
#include <vector>
#include <openssl/sha.h>

#ifndef SHA256_BUF_SIZE
#define SHA256_BUF_SIZE 131072 // 128 KiB
#endif

namespace libcpp
{

class sha256
{
public:
    static bool encode(unsigned char* dst, 
                       unsigned long& dst_len,
                       const unsigned char* src, 
                       const unsigned long src_len)
    {
        if (dst_len < SHA256_DIGEST_LENGTH )
            return false;

        SHA256(src, src_len, dst);
        dst_len = SHA256_DIGEST_LENGTH;
        return true;
    }

    static bool encode(std::string& dst,
                       const std::string& src)
    {
        dst.resize(SHA256_DIGEST_LENGTH);
        SHA256(reinterpret_cast<const unsigned char *>(src.c_str()), 
               src.size(), 
               reinterpret_cast<unsigned char *>(const_cast<char*>(dst.data())));
        return true;
    };

    static bool encode(std::string& dst,
                       std::istream& in)
    {
        if (!in.good())
            return false;
            
        SHA256_CTX ctx;
        SHA256_Init(&ctx);
        std::streamsize sz;
        std::vector<char> buf(SHA256_BUF_SIZE);
        while ((sz = in.read(&buf[0], SHA256_BUF_SIZE).gcount()) > 0)
            SHA256_Update(&ctx, buf.data(), static_cast<std::size_t>(sz));

        dst.resize(SHA256_DIGEST_LENGTH);
        SHA256_Final(reinterpret_cast<unsigned char *>(&dst[0]), &ctx);
        return true;
    };

    static bool encode(std::ostream& out, 
                       std::istream& in)
    {
        if (!in.good() || !out.good())
            return false;

        SHA256_CTX ctx;
        SHA256_Init(&ctx);
        std::vector<char> buf(SHA256_BUF_SIZE);
        std::streamsize sz;
        while ((sz = in.read(buf.data(), SHA256_BUF_SIZE).gcount()) > 0)
            SHA256_Update(&ctx, buf.data(), static_cast<std::size_t>(sz));

        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_Final(hash, &ctx);

        out.write(reinterpret_cast<const char*>(hash), SHA256_DIGEST_LENGTH);
        out.flush();
        return true;
    };

    // reserve encode dst buf size
	static unsigned long encode_len_reserve()
	{
		return SHA256_DIGEST_LENGTH;
	}

private:
    sha256() = default;
    ~sha256() = default;
    sha256(const sha256&) = delete;
    sha256& operator=(const sha256&) = delete;
    sha256(sha256&&) = delete;
    sha256& operator=(sha256&&) = delete;
};

}

#endif