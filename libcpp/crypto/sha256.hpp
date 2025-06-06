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

#include <iostream>
#include <vector>
#include <openssl/sha.h>

namespace libcpp
{

class sha256
{
public:
    sha256() {};
    ~sha256() {};

    static void encode(const std::string& src, std::string& dst)
    {
        dst.resize(256 / 8);
        SHA256(reinterpret_cast<const unsigned char *>(&src[0]), 
               src.size(), 
               reinterpret_cast<unsigned char *>(&dst[0]));
    };

    static void encode(std::istream& in, std::string& dst)
    {
        SHA256_CTX ctx;
        SHA256_Init(&ctx);
        std::streamsize sz;
        std::vector<char> buf(1024 * 128);
        while ((sz = in.read(&buf[0], sz).gcount()) > 0)
            SHA256_Update(&ctx, buf.data(), sz);

        dst.resize(256 / 8);
        SHA256_Final(reinterpret_cast<unsigned char *>(&dst[0]), &ctx);
    };
};

}

#endif