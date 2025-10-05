/*
 *  This file is part of hj.
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
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <array>
#include <openssl/sha.h>

#ifndef SHA_BUF_SIZE
#define SHA_BUF_SIZE 131072 // 128 KiB
#endif

namespace hj
{
template <std::size_t N>
using hash_array = std::array<uint8_t, N>;

class sha
{
  public:
    enum class error_code
    {
        ok = 0,
        invalid_input,
        invalid_output,
        buffer_too_small,
        not_supported_algo,
        unknown,
    };

    enum class algorithm
    {
        sha1,   // SHA-1
        sha224, // SHA-224
        sha256, // SHA-256
        sha384, // SHA-384
        sha512, // SHA-512
    };

    static std::size_t get_digest_length(algorithm algo)
    {
        switch(algo)
        {
            case algorithm::sha1:
                return SHA_DIGEST_LENGTH; // 20byte
            case algorithm::sha224:
                return SHA224_DIGEST_LENGTH; // 28byte
            case algorithm::sha256:
                return SHA256_DIGEST_LENGTH; // 32byte
            case algorithm::sha384:
                return SHA384_DIGEST_LENGTH; // 48byte
            case algorithm::sha512:
                return SHA512_DIGEST_LENGTH; // 64byte
            default:
                return SHA256_DIGEST_LENGTH; // default SHA-256
        }
    }

    static error_code encode(unsigned char       *dst,
                             std::size_t         &dst_len,
                             const unsigned char *src,
                             const std::size_t    src_len,
                             const algorithm      algo = algorithm::sha256)
    {
        std::size_t required_len = get_digest_length(algo);
        if(dst_len < required_len)
            return error_code::buffer_too_small;

        switch(algo)
        {
            case algorithm::sha1:
                SHA1(src, src_len, dst);
                break;
            case algorithm::sha224:
                SHA224(src, src_len, dst);
                break;
            case algorithm::sha256:
                SHA256(src, src_len, dst);
                break;
            case algorithm::sha384:
                SHA384(src, src_len, dst);
                break;
            case algorithm::sha512:
                SHA512(src, src_len, dst);
                break;
            default:
                return error_code::not_supported_algo;
        }

        dst_len = required_len;
        return error_code::ok;
    }

    static error_code encode(std::string       &dst,
                             const std::string &src,
                             const algorithm    algo = algorithm::sha256)
    {
        std::size_t digest_len = get_digest_length(algo);
        dst.resize(digest_len);
        unsigned char *dst_ptr =
            reinterpret_cast<unsigned char *>(const_cast<char *>(dst.data()));
        const unsigned char *src_ptr =
            reinterpret_cast<const unsigned char *>(src.c_str());
        switch(algo)
        {
            case algorithm::sha1:
                SHA1(src_ptr, src.size(), dst_ptr);
                break;
            case algorithm::sha224:
                SHA224(src_ptr, src.size(), dst_ptr);
                break;
            case algorithm::sha256:
                SHA256(src_ptr, src.size(), dst_ptr);
                break;
            case algorithm::sha384:
                SHA384(src_ptr, src.size(), dst_ptr);
                break;
            case algorithm::sha512:
                SHA512(src_ptr, src.size(), dst_ptr);
                break;
            default:
                dst.clear();
                return error_code::not_supported_algo;
        }

        return error_code::ok;
    }

    template <std::size_t N>
    static error_code encode(hash_array<N>     &dst,
                             const std::string &src,
                             const algorithm    algo = algorithm::sha256)
    {
        std::size_t dst_len = dst.size();
        auto        ec      = encode(dst.data(),
                         dst_len,
                         reinterpret_cast<const unsigned char *>(src.c_str()),
                         src.size(),
                         algo);
        if(ec != error_code::ok)
        {
            dst.fill(0);
            return ec;
        }

        return error_code::ok;
    }

    static error_code encode(std::string    &dst,
                             std::istream   &in,
                             const algorithm algo = algorithm::sha256)
    {
        if(!in.good())
        {
            dst.clear();
            return error_code::invalid_output;
        }

        std::size_t digest_len = get_digest_length(algo);
        dst.resize(digest_len);
        switch(algo)
        {
            case algorithm::sha1:
                return _encode_stream_sha1(dst, in) ? error_code::ok
                                                    : error_code::unknown;
            case algorithm::sha224:
                return _encode_stream_sha224(dst, in) ? error_code::ok
                                                      : error_code::unknown;
            case algorithm::sha256:
                return _encode_stream_sha256(dst, in) ? error_code::ok
                                                      : error_code::unknown;
            case algorithm::sha384:
                return _encode_stream_sha384(dst, in) ? error_code::ok
                                                      : error_code::unknown;
            case algorithm::sha512:
                return _encode_stream_sha512(dst, in) ? error_code::ok
                                                      : error_code::unknown;
            default:
                dst.clear();
                return error_code::not_supported_algo;
        }
    }

    static error_code encode(std::ostream   &out,
                             std::istream   &in,
                             const algorithm algo = algorithm::sha256)
    {
        if(!in.good())
            return error_code::invalid_input;
        if(!out.good())
            return error_code::invalid_output;

        std::string hash_result;
        auto        ec = encode(hash_result, in, algo);
        if(ec != error_code::ok)
        {
            hash_result.clear();
            return ec;
        }

        out.write(hash_result.data(), hash_result.size());
        out.flush();
        std::fill(hash_result.begin(), hash_result.end(), 0);
        return error_code::ok;
    }

    static error_code encode_file(const char     *dst_file_path,
                                  const char     *src_file_path,
                                  const algorithm algo = algorithm::sha256)
    {
        std::ifstream src_file(src_file_path, std::ios::binary);
        if(!src_file.is_open())
            return error_code::invalid_input;

        std::ofstream dst_file(dst_file_path, std::ios::binary);
        if(!dst_file.is_open())
            return error_code::invalid_output;

        auto ec = encode(dst_file, src_file, algo);
        dst_file.close();
        src_file.close();
        return ec;
    }

    static error_code encode_file(const std::string &dst_file_path,
                                  const std::string &src_file_path,
                                  const algorithm    algo = algorithm::sha256)
    {
        return encode_file(dst_file_path.c_str(), src_file_path.c_str(), algo);
    }

    static std::size_t
    encode_len_reserve(const algorithm algo = algorithm::sha256)
    {
        return get_digest_length(algo);
    }

    static error_code sha1(std::string &dst, const std::string &src)
    {
        return encode(dst, src, algorithm::sha1);
    }

    static error_code sha224(std::string &dst, const std::string &src)
    {
        return encode(dst, src, algorithm::sha224);
    }

    static error_code sha256(std::string &dst, const std::string &src)
    {
        return encode(dst, src, algorithm::sha256);
    }

    static error_code sha384(std::string &dst, const std::string &src)
    {
        return encode(dst, src, algorithm::sha384);
    }

    static error_code sha512(std::string &dst, const std::string &src)
    {
        return encode(dst, src, algorithm::sha512);
    }

  private:
    static bool _encode_stream_sha1(std::string &dst, std::istream &in)
    {
        SHA_CTX ctx;
        SHA1_Init(&ctx);
        std::vector<char> buf(SHA_BUF_SIZE);
        std::streamsize   sz;
        while((sz = in.read(buf.data(), SHA_BUF_SIZE).gcount()) > 0)
            SHA1_Update(&ctx, buf.data(), static_cast<std::size_t>(sz));

        SHA1_Final(reinterpret_cast<unsigned char *>(&dst[0]), &ctx);
        memset(&ctx, 0, sizeof(ctx));
        return true;
    }

    static bool _encode_stream_sha224(std::string &dst, std::istream &in)
    {
        SHA256_CTX ctx;
        SHA224_Init(&ctx);
        std::vector<char> buf(SHA_BUF_SIZE);
        std::streamsize   sz;
        while((sz = in.read(buf.data(), SHA_BUF_SIZE).gcount()) > 0)
            SHA224_Update(&ctx, buf.data(), static_cast<std::size_t>(sz));

        SHA224_Final(reinterpret_cast<unsigned char *>(&dst[0]), &ctx);
        memset(&ctx, 0, sizeof(ctx));
        return true;
    }

    static bool _encode_stream_sha256(std::string &dst, std::istream &in)
    {
        SHA256_CTX ctx;
        SHA256_Init(&ctx);
        std::vector<char> buf(SHA_BUF_SIZE);
        std::streamsize   sz;
        while((sz = in.read(buf.data(), SHA_BUF_SIZE).gcount()) > 0)
            SHA256_Update(&ctx, buf.data(), static_cast<std::size_t>(sz));

        SHA256_Final(reinterpret_cast<unsigned char *>(&dst[0]), &ctx);
        memset(&ctx, 0, sizeof(ctx));
        return true;
    }

    static bool _encode_stream_sha384(std::string &dst, std::istream &in)
    {
        SHA512_CTX ctx;
        SHA384_Init(&ctx);
        std::vector<char> buf(SHA_BUF_SIZE);
        std::streamsize   sz;
        while((sz = in.read(buf.data(), SHA_BUF_SIZE).gcount()) > 0)
            SHA384_Update(&ctx, buf.data(), static_cast<std::size_t>(sz));

        SHA384_Final(reinterpret_cast<unsigned char *>(&dst[0]), &ctx);
        memset(&ctx, 0, sizeof(ctx));
        return true;
    }

    static bool _encode_stream_sha512(std::string &dst, std::istream &in)
    {
        SHA512_CTX ctx;
        SHA512_Init(&ctx);
        std::vector<char> buf(SHA_BUF_SIZE);
        std::streamsize   sz;
        while((sz = in.read(buf.data(), SHA_BUF_SIZE).gcount()) > 0)
            SHA512_Update(&ctx, buf.data(), static_cast<std::size_t>(sz));

        SHA512_Final(reinterpret_cast<unsigned char *>(&dst[0]), &ctx);
        memset(&ctx, 0, sizeof(ctx));
        return true;
    }

  private:
    sha()                       = default;
    ~sha()                      = default;
    sha(const sha &)            = delete;
    sha &operator=(const sha &) = delete;
    sha(sha &&)                 = delete;
    sha &operator=(sha &&)      = delete;
};

}

#endif