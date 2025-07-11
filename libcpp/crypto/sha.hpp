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

#ifndef SHA_HPP
#define SHA_HPP

// disable msvc safe check warning
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

// support deprecated api for low version openssl
#ifndef OPENSSL_SUPPRESS_DEPRECATED
#define OPENSSL_SUPPRESS_DEPRECATED
#endif

#include <iostream>
#include <vector>
#include <string>
#include <openssl/sha.h>

#ifndef SHA_BUF_SIZE
#define SHA_BUF_SIZE 131072 // 128 KiB
#endif

namespace libcpp
{

class sha
{
public:
    enum class algorithm 
    {
        sha1,       // SHA-1
        sha224,     // SHA-224
        sha256,     // SHA-256
        sha384,     // SHA-384
        sha512,     // SHA-512
    };

    static std::size_t get_digest_length(algorithm algo)
    {
        switch (algo)
        {
        case algorithm::sha1:       return SHA_DIGEST_LENGTH;      // 20byte
        case algorithm::sha224:     return SHA224_DIGEST_LENGTH;   // 28byte
        case algorithm::sha256:     return SHA256_DIGEST_LENGTH;   // 32byte
        case algorithm::sha384:     return SHA384_DIGEST_LENGTH;   // 48byte
        case algorithm::sha512:     return SHA512_DIGEST_LENGTH;   // 64byte
        default:                    return SHA256_DIGEST_LENGTH;   // default SHA-256
        }
    }

    static bool encode(unsigned char* dst, 
                       std::size_t& dst_len,
                       const unsigned char* src, 
                       const std::size_t src_len,
                       const algorithm algo = algorithm::sha256)
    {
        std::size_t required_len = get_digest_length(algo);
        if (dst_len < required_len)
            return false;

        bool result = false;
        switch (algo)
        {
        case algorithm::sha1:
            SHA1(src, src_len, dst);
            result = true;
            break;
        case algorithm::sha224:
            SHA224(src, src_len, dst);
            result = true;
            break;
        case algorithm::sha256:
            SHA256(src, src_len, dst);
            result = true;
            break;
        case algorithm::sha384:
            SHA384(src, src_len, dst);
            result = true;
            break;
        case algorithm::sha512:
            SHA512(src, src_len, dst);
            result = true;
            break;
        default:
            return false;
        }

        if (result)
            dst_len = required_len;

        return result;
    }

    static bool encode(std::string& dst,
                       const std::string& src,
                       const algorithm algo = algorithm::sha256)
    {
        std::size_t digest_len = get_digest_length(algo);
        dst.resize(digest_len);
        
        bool result = false;
        unsigned char* dst_ptr = reinterpret_cast<unsigned char*>(const_cast<char*>(dst.data()));
        const unsigned char* src_ptr = reinterpret_cast<const unsigned char*>(src.c_str());

        switch (algo)
        {
        case algorithm::sha1:
            SHA1(src_ptr, src.size(), dst_ptr);
            result = true;
            break;
        case algorithm::sha224:
            SHA224(src_ptr, src.size(), dst_ptr);
            result = true;
            break;
        case algorithm::sha256:
            SHA256(src_ptr, src.size(), dst_ptr);
            result = true;
            break;
        case algorithm::sha384:
            SHA384(src_ptr, src.size(), dst_ptr);
            result = true;
            break;
        case algorithm::sha512:
            SHA512(src_ptr, src.size(), dst_ptr);
            result = true;
            break;
        default:
            return false;
        }
        
        return result;
    }

    static bool encode(std::string& dst,
                       std::istream& in,
                       const algorithm algo = algorithm::sha256)
    {
        if (!in.good())
            return false;

        std::size_t digest_len = get_digest_length(algo);
        dst.resize(digest_len);

        bool result = false;
        switch (algo)
        {
        case algorithm::sha1:
            result = _encode_stream_sha1(dst, in);
            break;
        case algorithm::sha224:
            result = _encode_stream_sha224(dst, in);
            break;
        case algorithm::sha256:
            result = _encode_stream_sha256(dst, in);
            break;
        case algorithm::sha384:
            result = _encode_stream_sha384(dst, in);
            break;
        case algorithm::sha512:
            result = _encode_stream_sha512(dst, in);
            break;
        default:
            return false;
        }

        return result;
    }

    static bool encode(std::ostream& out, 
                       std::istream& in,
                       const algorithm algo = algorithm::sha256)
    {
        if (!in.good() || !out.good())
            return false;

        std::string hash_result;
        if (!encode(hash_result, in, algo))
            return false;

        out.write(hash_result.data(), hash_result.size());
        out.flush();
        return true;
    }

    static std::size_t encode_len_reserve(const algorithm algo = algorithm::sha256)
    {
        return get_digest_length(algo);
    }

    static bool sha1(std::string& dst, const std::string& src)
    {
        return encode(dst, src, algorithm::sha1);
    }

    static bool sha224(std::string& dst, const std::string& src)
    {
        return encode(dst, src, algorithm::sha224);
    }

    static bool sha256(std::string& dst, const std::string& src)
    {
        return encode(dst, src, algorithm::sha256);
    }

    static bool sha384(std::string& dst, const std::string& src)
    {
        return encode(dst, src, algorithm::sha384);
    }

    static bool sha512(std::string& dst, const std::string& src)
    {
        return encode(dst, src, algorithm::sha512);
    }

private:
    static bool _encode_stream_sha1(std::string& dst, std::istream& in)
    {
        SHA_CTX ctx;
        SHA1_Init(&ctx);
        
        std::vector<char> buf(SHA_BUF_SIZE);
        std::streamsize sz;
        while ((sz = in.read(buf.data(), SHA_BUF_SIZE).gcount()) > 0)
            SHA1_Update(&ctx, buf.data(), static_cast<std::size_t>(sz));

        SHA1_Final(reinterpret_cast<unsigned char*>(&dst[0]), &ctx);
        return true;
    }

    static bool _encode_stream_sha224(std::string& dst, std::istream& in)
    {
        SHA256_CTX ctx;
        SHA224_Init(&ctx);
        
        std::vector<char> buf(SHA_BUF_SIZE);
        std::streamsize sz;
        while ((sz = in.read(buf.data(), SHA_BUF_SIZE).gcount()) > 0)
            SHA224_Update(&ctx, buf.data(), static_cast<std::size_t>(sz));

        SHA224_Final(reinterpret_cast<unsigned char*>(&dst[0]), &ctx);
        return true;
    }

    static bool _encode_stream_sha256(std::string& dst, std::istream& in)
    {
        SHA256_CTX ctx;
        SHA256_Init(&ctx);
        
        std::vector<char> buf(SHA_BUF_SIZE);
        std::streamsize sz;
        while ((sz = in.read(buf.data(), SHA_BUF_SIZE).gcount()) > 0)
            SHA256_Update(&ctx, buf.data(), static_cast<std::size_t>(sz));

        SHA256_Final(reinterpret_cast<unsigned char*>(&dst[0]), &ctx);
        return true;
    }

    static bool _encode_stream_sha384(std::string& dst, std::istream& in)
    {
        SHA512_CTX ctx;
        SHA384_Init(&ctx);
        
        std::vector<char> buf(SHA_BUF_SIZE);
        std::streamsize sz;
        while ((sz = in.read(buf.data(), SHA_BUF_SIZE).gcount()) > 0)
            SHA384_Update(&ctx, buf.data(), static_cast<std::size_t>(sz));

        SHA384_Final(reinterpret_cast<unsigned char*>(&dst[0]), &ctx);
        return true;
    }

    static bool _encode_stream_sha512(std::string& dst, std::istream& in)
    {
        SHA512_CTX ctx;
        SHA512_Init(&ctx);
        
        std::vector<char> buf(SHA_BUF_SIZE);
        std::streamsize sz;
        while ((sz = in.read(buf.data(), SHA_BUF_SIZE).gcount()) > 0)
            SHA512_Update(&ctx, buf.data(), static_cast<std::size_t>(sz));

        SHA512_Final(reinterpret_cast<unsigned char*>(&dst[0]), &ctx);
        return true;
    }

private:
    sha() = default;
    ~sha() = default;
    sha(const sha&) = delete;
    sha& operator=(const sha&) = delete;
    sha(sha&&) = delete;
    sha& operator=(sha&&) = delete;
};

}

#endif