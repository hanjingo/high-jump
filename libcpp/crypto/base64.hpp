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

#ifndef BASE64_HPP
#define BASE64_HPP

// disable msvc safe check warning
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

// support deprecated api for low version openssl
#ifndef OPENSSL_SUPPRESS_DEPRECATED
#define OPENSSL_SUPPRESS_DEPRECATED
#endif

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>

#ifndef BASE64_BUF_SIZE
#define BASE64_BUF_SIZE 4096
#endif

namespace libcpp {

class base64
{
  public:
    // bytes -> base64 bytes
    static bool encode(unsigned char* dst,
                       std::size_t& dst_len,
                       const unsigned char* src,
                       const std::size_t src_len)
    {
        if (src_len == 0 || dst_len == 0)
            return false;  // Invalid input or output length

        BUF_MEM* buffer_ptr;
        BIO* bio = BIO_new(BIO_s_mem());
        if (!bio)
            return false;  // Failed to create BIO from memory buffer
        BIO* b64 = BIO_new(BIO_f_base64());
        if (!b64)
        {
            BIO_free(bio);
            return false;  // Failed to create base64 BIO
        }

        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        BIO_push(b64, bio);
        BIO_write(b64, src, static_cast<int>(src_len));
        BIO_flush(b64);
        BIO_get_mem_ptr(b64, &buffer_ptr);
        if (buffer_ptr->length > dst_len)
        {
            BIO_free_all(b64);
            return false;  // Not enough space in destination buffer
        }

        memcpy(dst, buffer_ptr->data, buffer_ptr->length);
        dst_len = buffer_ptr->length;
        BIO_free_all(b64);
        return true;
    }

    // string -> base64 string
    static bool encode(std::string& dst, const std::string& src)
    {
        std::size_t dst_len = encode_len_reserve(src.size());
        dst.resize(dst_len);  // Base64 encoding increases size by ~33%
        if (!encode(
                reinterpret_cast<unsigned char*>(const_cast<char*>(dst.data())),
                dst_len,
                reinterpret_cast<const unsigned char*>(src.c_str()),
                src.size()))
        {
            dst.clear();
            return false;
        }

        dst.resize(dst_len);  // Resize to actual length
        return true;
    }

    // base64 stream -> stream
    static bool encode(std::ostream& out, std::istream& in)
    {
        if (!in || !out)
            return false;

        BIO* b64 = BIO_new(BIO_f_base64());
        if (!b64)
            return false;

        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        BIO* bio_out = BIO_new(BIO_s_mem());
        if (!bio_out)
        {
            BIO_free(b64);
            return false;
        }
        BIO_push(b64, bio_out);

        char buffer[BASE64_BUF_SIZE];
        while (in.read(buffer, sizeof(buffer)) || in.gcount() > 0)
        {
            std::streamsize n = in.gcount();
            if (BIO_write(b64, buffer, static_cast<int>(n)) != n)
            {
                BIO_free_all(b64);
                return false;
            }
        }

        BIO_flush(b64);
        BUF_MEM* mem_ptr = nullptr;
        BIO_get_mem_ptr(b64, &mem_ptr);
        if (mem_ptr && mem_ptr->length > 0)
            out.write(mem_ptr->data, mem_ptr->length);

        BIO_free_all(b64);
        return true;
    }

    // file -> base64 file
    static bool encode_file(const char* dst_file_path,
                            const char* src_file_path)
    {
        FILE* in = fopen(src_file_path, "rb");
        if (!in)
            return false;

        FILE* out = fopen(dst_file_path, "wb");
        if (!out)
        {
            fclose(in);
            return false;
        }

        BIO* bio_out = BIO_new_fp(out, BIO_NOCLOSE);
        BIO* b64 = BIO_new(BIO_f_base64());
        if (!b64)
        {
            BIO_free(bio_out);
            fclose(in);
            fclose(out);
            return false;
        }

        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        BIO_push(b64, bio_out);

        char buffer[BASE64_BUF_SIZE];
        std::size_t n;
        while ((n = fread(buffer, 1, sizeof(buffer), in)) > 0)
        {
            if (BIO_write(b64, buffer, static_cast<int>(n)) == (int)n)
                continue;

            BIO_free_all(b64);
            fclose(in);
            fclose(out);
            return false;
        }

        BIO_flush(b64);
        BIO_free_all(b64);
        fclose(in);
        fclose(out);
        return true;
    }

    // file -> base64 file
    static bool encode_file(const std::string& dst_file_path,
                            const std::string& src_file_path)
    {
        return encode_file(dst_file_path.c_str(), src_file_path.c_str());
    }

    // base64 bytes -> bytes
    static bool decode(unsigned char* dst,
                       std::size_t& dst_len,
                       const unsigned char* src,
                       const std::size_t src_len)
    {
        if (src_len % 4 != 0)
            return false;  // Invalid base64 input length

        BIO* bio = BIO_new_mem_buf(src, static_cast<int>(src_len));
        if (!bio)
            return false;  // Failed to create BIO from memory buffer

        BIO* b64 = BIO_new(BIO_f_base64());
        if (!b64)
        {
            BIO_free(bio);
            return false;  // Failed to create base64 BIO
        }

        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        BIO_push(b64, bio);
        int decoded_length = BIO_read(b64, dst, static_cast<int>(dst_len));
        if (decoded_length < 0)
        {
            BIO_free_all(b64);
            return false;
        }

        dst_len = static_cast<std::size_t>(decoded_length);
        BIO_free_all(b64);
        return true;
    }

    // base64 string -> string
    static bool decode(std::string& dst, const std::string& src)
    {
        dst.resize(decode_len_reserve(src.size()));
        std::size_t dst_len = dst.size();
        if (!decode(
                reinterpret_cast<unsigned char*>(const_cast<char*>(dst.data())),
                dst_len,
                reinterpret_cast<const unsigned char*>(src.c_str()),
                src.size()))
        {
            dst.clear();  // Clear the string if decoding fails
            return false;
        }

        dst.resize(dst_len);
        return true;
    }

    // base64 stream -> stream
    static bool decode(std::ostream& out, std::istream& in)
    {
        BIO* b64 = BIO_new(BIO_f_base64());
        if (!b64)
            return false;

        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

        std::string input_buf;
        char buffer[BASE64_BUF_SIZE];
        while (in.read(buffer, sizeof(buffer)) || in.gcount() > 0)
        {
            input_buf.append(buffer, static_cast<size_t>(in.gcount()));
        }

        BIO* bio_in = BIO_new_mem_buf(input_buf.data(),
                                      static_cast<int>(input_buf.size()));
        if (!bio_in)
        {
            BIO_free(b64);
            return false;
        }
        BIO_push(b64, bio_in);

        char outbuf[BASE64_BUF_SIZE];
        int n;
        while ((n = BIO_read(b64, outbuf, sizeof(outbuf))) > 0)
        {
            out.write(outbuf, n);
        }

        BIO_free_all(b64);
        return true;
    }

    // base64 file -> file
    static bool decode_file(const char* dst_file_path,
                            const char* src_file_path)
    {
        FILE* in = fopen(src_file_path, "rb");
        if (!in)
            return false;

        FILE* out = fopen(dst_file_path, "wb");
        if (!out)
        {
            fclose(in);
            return false;
        }

        BIO* bio_in = BIO_new_fp(in, BIO_NOCLOSE);
        BIO* b64 = BIO_new(BIO_f_base64());
        if (!b64)
        {
            BIO_free(bio_in);
            fclose(in);
            fclose(out);
            return false;
        }

        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        BIO_push(b64, bio_in);

        char buffer[4096];
        int n;
        while ((n = BIO_read(b64, buffer, sizeof(buffer))) > 0)
        {
            if (fwrite(buffer, 1, n, out) == (size_t)n)
                continue;

            BIO_free_all(b64);
            fclose(in);
            fclose(out);
            return false;
        }

        BIO_free_all(b64);
        fclose(in);
        fclose(out);
        return true;
    }

    // base64 file -> file
    static bool decode_file(const std::string& dst_file_path,
                            const std::string& src_file_path)
    {
        return decode_file(dst_file_path.c_str(), src_file_path.c_str());
    }

    // reserve encode dst buf size
    static std::size_t encode_len_reserve(const std::size_t src_len)
    {
        return (src_len + 2) / 3 * 4;
    }

    // reserve decode dst buf size
    static std::size_t decode_len_reserve(const std::size_t src_len)
    {
        return (src_len / 4) * 3;
    }

  private:
    base64() = default;
    ~base64() = default;
    base64(const base64&) = delete;
    base64& operator=(const base64&) = delete;
    base64(base64&&) = delete;
    base64& operator=(base64&&) = delete;
};

}  // namespace libcpp

#endif