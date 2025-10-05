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

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

#ifndef BASE64_BUF_SIZE
#define BASE64_BUF_SIZE 16384 // 16KB
#endif

namespace hj
{

class base64
{
  public:
    enum class error_code
    {
        ok = 0,
        invalid_input,
        buffer_overflow,
        encode_failed,
        decode_failed,
        file_open_failed,
        file_read_failed,
        file_write_failed
    };

  public:
    // bytes -> base64 bytes
    static error_code encode(unsigned char       *dst,
                             std::size_t         &dst_len,
                             const unsigned char *src,
                             const std::size_t    src_len)
    {
        if(src_len == 0 || dst_len == 0)
            return error_code::invalid_input; // Invalid input or output length

        BUF_MEM *buffer_ptr;
        BIO     *bio = BIO_new(BIO_s_mem());
        if(!bio)
            return error_code::
                encode_failed; // Failed to create BIO from memory buffer

        BIO *b64 = BIO_new(BIO_f_base64());
        if(!b64)
        {
            BIO_free(bio);
            return error_code::encode_failed; // Failed to create base64 BIO
        }

        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        BIO_push(b64, bio);
        BIO_write(b64, src, static_cast<int>(src_len));
        BIO_flush(b64);
        BIO_get_mem_ptr(b64, &buffer_ptr);
        if(buffer_ptr->length > dst_len)
        {
            BIO_free_all(b64);
            return error_code::
                buffer_overflow; // Not enough space in destination buffer
        }

        memcpy(dst, buffer_ptr->data, buffer_ptr->length);
        dst_len = buffer_ptr->length;
        BIO_free_all(b64);
        return error_code::ok;
    }

    // string -> base64 string
    static error_code encode(std::string &dst, const std::string &src)
    {
        std::size_t dst_len = encode_len_reserve(src.size());
        dst.resize(dst_len); // Base64 encoding increases size by ~33%
        auto ec = encode(
            reinterpret_cast<unsigned char *>(const_cast<char *>(dst.data())),
            dst_len,
            reinterpret_cast<const unsigned char *>(src.c_str()),
            src.size());
        if(ec != error_code::ok)
        {
            dst.clear();
            return ec;
        }

        dst.resize(dst_len); // Resize to actual length
        return error_code::ok;
    }

    // base64 stream -> stream
    static error_code encode(std::ostream &out, std::istream &in)
    {
        if(!in || !out)
            return error_code::invalid_input;

        BIO *b64 = BIO_new(BIO_f_base64());
        if(!b64)
            return error_code::encode_failed;

        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        BIO *bio_out = BIO_new(BIO_s_mem());
        if(!bio_out)
        {
            BIO_free(b64);
            return error_code::encode_failed;
        }

        BIO_push(b64, bio_out);
        char buffer[BASE64_BUF_SIZE];
        while(in.read(buffer, sizeof(buffer)) || in.gcount() > 0)
        {
            std::streamsize n = in.gcount();
            if(BIO_write(b64, buffer, static_cast<int>(n)) != n)
            {
                BIO_free_all(b64);
                return error_code::encode_failed;
            }
        }

        BIO_flush(b64);
        BUF_MEM *mem_ptr = nullptr;
        BIO_get_mem_ptr(b64, &mem_ptr);
        if(mem_ptr && mem_ptr->length > 0)
            out.write(mem_ptr->data, mem_ptr->length);

        BIO_free_all(b64);
        return error_code::ok;
    }

    // file -> base64 file
    static error_code encode_file(const char *dst_file_path,
                                  const char *src_file_path)
    {
        FILE *in = fopen(src_file_path, "rb");
        if(!in)
            return error_code::file_open_failed;

        FILE *out = fopen(dst_file_path, "wb");
        if(!out)
        {
            fclose(in);
            return error_code::file_open_failed;
        }

        BIO *bio_out = BIO_new_fp(out, BIO_NOCLOSE);
        BIO *b64     = BIO_new(BIO_f_base64());
        if(!b64)
        {
            BIO_free(bio_out);
            fclose(in);
            fclose(out);
            return error_code::encode_failed;
        }

        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        BIO_push(b64, bio_out);

        char        buffer[BASE64_BUF_SIZE];
        std::size_t n;
        while((n = fread(buffer, 1, sizeof(buffer), in)) > 0)
        {
            if(BIO_write(b64, buffer, static_cast<int>(n)) == (int) n)
                continue;

            BIO_free_all(b64);
            fclose(in);
            fclose(out);
            return error_code::encode_failed;
        }

        BIO_flush(b64);
        BIO_free_all(b64);
        fclose(in);
        fclose(out);
        return error_code::ok;
    }

    // file -> base64 file
    static error_code encode_file(const std::string &dst_file_path,
                                  const std::string &src_file_path)
    {
        return encode_file(dst_file_path.c_str(), src_file_path.c_str());
    }

    // base64 bytes -> bytes
    static error_code decode(unsigned char       *dst,
                             std::size_t         &dst_len,
                             const unsigned char *src,
                             const std::size_t    src_len)
    {
        if(src_len % 4 != 0)
            return error_code::invalid_input; // Invalid base64 input length

        BIO *bio = BIO_new_mem_buf(src, static_cast<int>(src_len));
        if(!bio)
            return error_code::
                decode_failed; // Failed to create BIO from memory buffer

        BIO *b64 = BIO_new(BIO_f_base64());
        if(!b64)
        {
            BIO_free(bio);
            return error_code::decode_failed; // Failed to create base64 BIO
        }

        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        BIO_push(b64, bio);
        int decoded_length = BIO_read(b64, dst, static_cast<int>(dst_len));
        if(decoded_length < 0)
        {
            BIO_free_all(b64);
            return error_code::decode_failed;
        }

        dst_len = static_cast<std::size_t>(decoded_length);
        BIO_free_all(b64);
        return error_code::ok;
    }

    // base64 string -> string
    static error_code decode(std::string &dst, const std::string &src)
    {
        dst.resize(decode_len_reserve(src.size()));
        std::size_t dst_len = dst.size();
        auto        ec      = decode(
            reinterpret_cast<unsigned char *>(const_cast<char *>(dst.data())),
            dst_len,
            reinterpret_cast<const unsigned char *>(src.c_str()),
            src.size());
        if(ec != error_code::ok)
        {
            dst.clear(); // Clear the string if decoding fails
            return ec;
        }

        dst.resize(dst_len);
        return error_code::ok;
    }

    // base64 stream -> stream
    static error_code decode(std::ostream &out, std::istream &in)
    {
        BIO *b64 = BIO_new(BIO_f_base64());
        if(!b64)
            return error_code::decode_failed;

        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        std::string input_buf;
        char        buffer[BASE64_BUF_SIZE];
        while(in.read(buffer, sizeof(buffer)) || in.gcount() > 0)
            input_buf.append(buffer, static_cast<size_t>(in.gcount()));

        BIO *bio_in = BIO_new_mem_buf(input_buf.data(),
                                      static_cast<int>(input_buf.size()));
        if(!bio_in)
        {
            BIO_free(b64);
            return error_code::decode_failed;
        }

        BIO_push(b64, bio_in);
        char outbuf[BASE64_BUF_SIZE];
        int  n;
        while((n = BIO_read(b64, outbuf, sizeof(outbuf))) > 0)
            out.write(outbuf, n);

        BIO_free_all(b64);
        return error_code::ok;
    }

    // base64 file -> file
    static error_code decode_file(const char *dst_file_path,
                                  const char *src_file_path)
    {
        FILE *in = fopen(src_file_path, "rb");
        if(!in)
            return error_code::file_open_failed;

        FILE *out = fopen(dst_file_path, "wb");
        if(!out)
        {
            fclose(in);
            return error_code::file_open_failed;
        }

        BIO *bio_in = BIO_new_fp(in, BIO_NOCLOSE);
        BIO *b64    = BIO_new(BIO_f_base64());
        if(!b64)
        {
            BIO_free(bio_in);
            fclose(in);
            fclose(out);
            return error_code::decode_failed;
        }

        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        BIO_push(b64, bio_in);
        char buffer[4096];
        int  n;
        while((n = BIO_read(b64, buffer, sizeof(buffer))) > 0)
        {
            if(fwrite(buffer, 1, n, out) == (size_t) n)
                continue;

            BIO_free_all(b64);
            fclose(in);
            fclose(out);
            return error_code::file_write_failed;
        }

        BIO_free_all(b64);
        fclose(in);
        fclose(out);
        return error_code::ok;
    }

    // base64 file -> file
    static error_code decode_file(const std::string &dst_file_path,
                                  const std::string &src_file_path)
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

    static bool is_valid(const unsigned char *buf, const std::size_t len)
    {
        if(len == 0 || buf == nullptr)
            return false;

        if(len % 4 != 0)
            return false;

        std::size_t pad = 0;
        for(std::size_t i = 0; i < len; ++i)
        {
            char c = buf[i];
            if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
               || (c >= '0' && c <= '9') || c == '+' || c == '/')
            {
                if(pad > 0)
                    return false;

                continue;
            }

            if(c == '=')
            {
                ++pad;
                if(pad > 2)
                    return false;

                if(i < len - 2)
                    return false;
            } else
            {
                return false;
            }
        }
        return true;
    }

    static bool is_valid(const std::string &str)
    {
        return is_valid(reinterpret_cast<const unsigned char *>(str.c_str()),
                        str.length());
    }

    static bool is_valid(std::ifstream &in)
    {
        if(!in.is_open())
            return false;

        unsigned char           buf[BASE64_BUF_SIZE];
        std::size_t             total_len = 0;
        std::ifstream::pos_type start_pos = in.tellg();
        std::streamsize         n;
        while(in)
        {
            in.read(reinterpret_cast<char *>(buf), BASE64_BUF_SIZE);
            n = in.gcount();
            if(n == 0)
                break;

            total_len += n;
            if(!is_valid(buf, n))
                return false;
        }

        in.clear();
        in.seekg(start_pos);
        if(total_len == 0 || total_len % 4 != 0)
            return false;

        return true;
    }

    static bool is_valid_file(const std::string &file_path)
    {
        std::ifstream file(file_path, std::ios::binary);
        if(!file.is_open())
            return false;

        return is_valid(file);
    }

  private:
    base64()                          = default;
    ~base64()                         = default;
    base64(const base64 &)            = delete;
    base64 &operator=(const base64 &) = delete;
    base64(base64 &&)                 = delete;
    base64 &operator=(base64 &&)      = delete;
};

}

#endif