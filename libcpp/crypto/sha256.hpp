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

namespace libcpp
{

// because of unidirectionality, sha256 not support decode
class sha256
{
public:
    static bool encode(const unsigned char* src, 
                       const unsigned long src_len, 
                       unsigned char* dst, 
                       unsigned long& dst_len)
    {
        if (dst_len < SHA256_DIGEST_LENGTH )
            return false;

        SHA256(src, src_len, dst);
        dst_len = SHA256_DIGEST_LENGTH;
        return true;
    }

    static bool encode(const std::string& src, 
                       std::string& dst)
    {
        dst.resize(SHA256_DIGEST_LENGTH);
        SHA256(reinterpret_cast<const unsigned char *>(src.c_str()), 
               src.size(), 
               reinterpret_cast<unsigned char *>(const_cast<char*>(dst.data())));
        return true;
    };

    static void encode(std::istream& in, 
                       std::string& dst)
    {
        SHA256_CTX ctx;
        SHA256_Init(&ctx);
        std::streamsize sz;
        std::vector<char> buf(1024 * 128);
        while ((sz = in.read(&buf[0], sz).gcount()) > 0)
            SHA256_Update(&ctx, buf.data(), sz);

        dst.resize(SHA256_DIGEST_LENGTH);
        SHA256_Final(reinterpret_cast<unsigned char *>(&dst[0]), &ctx);
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