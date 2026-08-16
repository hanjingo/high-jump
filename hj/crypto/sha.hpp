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

#ifndef SHA_HPP
#define SHA_HPP

#include <openssl/evp.h>
#include <openssl/crypto.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>

namespace hj
{
template <std::size_t N>
using hash_array = std::array<uint8_t, N>;

namespace sha
{

static constexpr std::size_t buf_sz = 65535; // 64KB

enum class algorithm
{
    sha1,   // SHA-1
    sha224, // SHA-224
    sha256, // SHA-256
    sha384, // SHA-384
    sha512, // SHA-512
};

enum class error_code
{
    ok = 0,
    invalid_input,
    invalid_output,
    buffer_too_small,
    not_supported_algo,
    openssl_internal_error,
    unknown,
};

class sha_category_impl : public std::error_category
{
  public:
    [[nodiscard]] const char *name() const noexcept override
    {
        return "hj.sha";
    }

    [[nodiscard]] std::string message(int ev) const override
    {
        switch(static_cast<error_code>(ev))
        {
            case error_code::ok:
                return "ok";
            case error_code::invalid_input:
                return "invalid input";
            case error_code::invalid_output:
                return "invalid output";
            case error_code::buffer_too_small:
                return "buffer too small";
            case error_code::not_supported_algo:
                return "not supported algorithm";
            case error_code::openssl_internal_error:
                return "openssl internal error";
            case error_code::unknown:
            default:
                return "unknown error";
        }
    }
};

[[nodiscard]] inline const std::error_category &sha_category() noexcept
{
    static const sha_category_impl instance;
    return instance;
}

[[nodiscard]] inline std::error_code make_error_code(error_code e) noexcept
{
    return {static_cast<int>(e), sha_category()};
}

namespace detail
{
[[nodiscard]] inline const EVP_MD *get_evp_md(algorithm algo) noexcept
{
    switch(algo)
    {
        case algorithm::sha1:
            return EVP_sha1();
        case algorithm::sha224:
            return EVP_sha224();
        case algorithm::sha256:
            return EVP_sha256();
        case algorithm::sha384:
            return EVP_sha384();
        case algorithm::sha512:
            return EVP_sha512();
        default:
            return nullptr;
    }
}
} // namespace detail

class hasher
{
  public:
    explicit hasher(algorithm algo = algorithm::sha256)
        : _ctx(EVP_MD_CTX_new())
        , _algo(algo)
    {
        reset(_algo);
    }

    ~hasher()
    {
        if(_ctx)
        {
            EVP_MD_CTX_free(_ctx);
            _ctx = nullptr;
        }
    }

    hasher(const hasher &)            = delete;
    hasher &operator=(const hasher &) = delete;

    hasher(hasher &&other) noexcept
        : _ctx(other._ctx)
        , _algo(other._algo)
        , _finished(other._finished)
    {
        other._ctx      = nullptr;
        other._finished = true;
    }

    hasher &operator=(hasher &&other) noexcept
    {
        if(this != &other)
        {
            if(_ctx)
                EVP_MD_CTX_free(_ctx);
            _ctx            = other._ctx;
            _algo           = other._algo;
            _finished       = other._finished;
            other._ctx      = nullptr;
            other._finished = true;
        }
        return *this;
    }

    [[nodiscard]] bool is_valid() const noexcept
    {
        return _ctx != nullptr && !_finished;
    }

    [[nodiscard]] bool is_finished() const noexcept { return _finished; }

    [[nodiscard]] error_code reset(algorithm algo) noexcept
    {
        _algo = algo;
        return reset();
    }

    [[nodiscard]] error_code reset() noexcept
    {
        if(!_ctx)
        {
            _ctx = EVP_MD_CTX_new();
            if(!_ctx)
            {
                _finished = true;
                return error_code::openssl_internal_error;
            }
        }

        const EVP_MD *md = detail::get_evp_md(_algo);
        if(!md || EVP_DigestInit_ex(_ctx, md, nullptr) != 1)
        {
            _finished = true;
            return error_code::openssl_internal_error;
        }

        _finished = false;
        return error_code::ok;
    }

    [[nodiscard]] error_code update(const void *data, std::size_t len) noexcept
    {
        if(!_ctx)
            return error_code::openssl_internal_error;
        if(_finished)
            return error_code::invalid_input;
        if(len > 0 && data == nullptr)
            return error_code::invalid_input;

        if(EVP_DigestUpdate(_ctx, data, len) != 1)
            return error_code::openssl_internal_error;
        return error_code::ok;
    }

    [[nodiscard]] error_code update(std::string_view data) noexcept
    {
        return update(data.data(), data.size());
    }

    [[nodiscard]] error_code final(unsigned char *out,
                                   std::size_t   &out_len) noexcept
    {
        if(!_ctx)
            return error_code::openssl_internal_error;
        if(_finished)
            return error_code::invalid_input;
        if(out == nullptr)
            return error_code::invalid_output;

        std::size_t req_len = get_digest_length(_algo);
        if(out_len < req_len)
            return error_code::buffer_too_small;

        unsigned int len = 0;
        if(EVP_DigestFinal_ex(_ctx, out, &len) != 1)
        {
            _finished = true;
            return error_code::openssl_internal_error;
        }

        out_len   = len;
        _finished = true;
        return error_code::ok;
    }

    [[nodiscard]] error_code final(std::string &dst)
    {
        if(_finished)
        {
            dst.clear();
            return error_code::invalid_input;
        }

        std::size_t req_len = get_digest_length(_algo);
        dst.resize(req_len);
        std::size_t out_len = req_len;
        auto ec = final(reinterpret_cast<unsigned char *>(dst.data()), out_len);
        if(ec != error_code::ok)
        {
            dst.clear();
        }
        return ec;
    }

    [[nodiscard]] static std::size_t get_digest_length(algorithm algo) noexcept
    {
        const EVP_MD *md = detail::get_evp_md(algo);
        return md ? static_cast<std::size_t>(EVP_MD_get_size(md)) : 0;
    }

  private:
    EVP_MD_CTX *_ctx{nullptr};
    algorithm   _algo;
    bool        _finished{true};
};

[[nodiscard]] inline std::size_t get_digest_length(algorithm algo) noexcept
{
    return hasher::get_digest_length(algo);
}

[[nodiscard]] inline error_code
encode(unsigned char       *dst,
       std::size_t         &dst_len,
       const unsigned char *src,
       const std::size_t    src_len,
       const algorithm      algo = algorithm::sha256) noexcept
{
    if(dst == nullptr)
        return error_code::invalid_output;
    if(src_len > 0 && src == nullptr)
        return error_code::invalid_input;

    hasher h(algo);
    if(!h.is_valid())
        return error_code::openssl_internal_error;

    auto ec = h.update(src, src_len);
    if(ec != error_code::ok)
        return ec;

    return h.final(dst, dst_len);
}

[[nodiscard]] inline error_code encode(std::string     &dst,
                                       std::string_view src,
                                       const algorithm algo = algorithm::sha256)
{
    hasher h(algo);
    if(!h.is_valid())
    {
        dst.clear();
        return error_code::openssl_internal_error;
    }

    auto ec = h.update(src);
    if(ec != error_code::ok)
    {
        dst.clear();
        return ec;
    }

    return h.final(dst);
}

template <std::size_t N>
[[nodiscard]] inline error_code
encode(hash_array<N>   &dst,
       std::string_view src,
       const algorithm  algo = algorithm::sha256) noexcept
{
    std::size_t required_len = get_digest_length(algo);
    static_assert(N >= 20, "hash_array buffer size is too small for SHA hash.");
    if(N < required_len)
    {
        dst.fill(0);
        return error_code::buffer_too_small;
    }

    std::size_t dst_len = N;
    auto        ec = encode(dst.data(),
                            dst_len,
                            reinterpret_cast<const unsigned char *>(src.data()),
                            src.size(),
                            algo);
    if(ec != error_code::ok)
    {
        dst.fill(0);
    }
    return ec;
}

[[nodiscard]] inline error_code encode(std::string    &dst,
                                       std::istream   &in,
                                       const algorithm algo = algorithm::sha256)
{
    if(!in.good())
    {
        dst.clear();
        return error_code::invalid_input;
    }

    hasher h(algo);
    if(!h.is_valid())
    {
        dst.clear();
        return error_code::openssl_internal_error;
    }

    std::array<char, buf_sz> buf;
    while(in.good())
    {
        in.read(buf.data(), buf.size());
        std::streamsize bytes_read = in.gcount();
        if(bytes_read > 0)
        {
            auto ec =
                h.update(buf.data(), static_cast<std::size_t>(bytes_read));
            if(ec != error_code::ok)
            {
                dst.clear();
                return ec;
            }
        }
    }

    if(in.bad())
    {
        dst.clear();
        return error_code::invalid_input;
    }

    return h.final(dst);
}

[[nodiscard]] inline error_code encode(std::ostream   &out,
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
        return ec;
    }

    out.write(hash_result.data(),
              static_cast<std::streamsize>(hash_result.size()));
    out.flush();

    OPENSSL_cleanse(hash_result.data(), hash_result.size());
    return out.good() ? error_code::ok : error_code::invalid_output;
}

[[nodiscard]] inline error_code
encode_file(const char     *dst_file_path,
            const char     *src_file_path,
            const algorithm algo = algorithm::sha256)
{
    if(!dst_file_path || !src_file_path)
        return error_code::invalid_input;

    std::ifstream src_file(src_file_path, std::ios::binary);
    if(!src_file.is_open())
        return error_code::invalid_input;

    std::ofstream dst_file(dst_file_path, std::ios::binary);
    if(!dst_file.is_open())
        return error_code::invalid_output;

    return encode(dst_file, src_file, algo);
}

[[nodiscard]] inline error_code
encode_file(const std::string &dst_file_path,
            const std::string &src_file_path,
            const algorithm    algo = algorithm::sha256)
{
    return encode_file(dst_file_path.c_str(), src_file_path.c_str(), algo);
}

[[nodiscard]] inline std::size_t
encode_len_reserve(const algorithm algo = algorithm::sha256) noexcept
{
    return get_digest_length(algo);
}

[[nodiscard]] inline error_code sha1(std::string &dst, std::string_view src)
{
    return encode(dst, src, algorithm::sha1);
}

[[nodiscard]] inline error_code sha224(std::string &dst, std::string_view src)
{
    return encode(dst, src, algorithm::sha224);
}

[[nodiscard]] inline error_code sha256(std::string &dst, std::string_view src)
{
    return encode(dst, src, algorithm::sha256);
}

[[nodiscard]] inline error_code sha384(std::string &dst, std::string_view src)
{
    return encode(dst, src, algorithm::sha384);
}

[[nodiscard]] inline error_code sha512(std::string &dst, std::string_view src)
{
    return encode(dst, src, algorithm::sha512);
}

} // namespace sha
} // namespace hj

namespace std
{
template <>
struct is_error_code_enum<hj::sha::error_code> : true_type
{
};
} // namespace std

#endif // SHA_HPP