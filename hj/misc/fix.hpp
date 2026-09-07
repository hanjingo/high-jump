/*
 * This file is part of high-jump(hj).
 *
 * Copyright 2025 hanjingo <hehehunanchina@live.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FIX_HPP
#define FIX_HPP

#include <hffix.hpp>
#include <string_view>
#include <optional>
#include <vector>
#include <cstdint>
#include <cstring>
#include <variant>

namespace hj
{
namespace fix
{

class builder
{
  public:
    explicit builder(size_t bufsize = 1024)
        : _storage(std::vector<char>(bufsize))
        , _writer(std::get<std::vector<char>>(_storage).data(), bufsize)
    {
    }

    builder(char *buf, size_t bufsize) noexcept
        : _storage(buf)
        , _writer(buf, bufsize)
    {
    }

    ~builder()                          = default;
    builder(const builder &)            = delete;
    builder &operator=(const builder &) = delete;
    builder(builder &&)                 = default;
    builder &operator=(builder &&)      = default;

    inline void begin(std::string_view begin_string = "FIX.4.4") noexcept
    {
        if(begin_string == "FIX.4.4")
        {
            _writer.push_back_header("FIX.4.4");
        } else if(begin_string == "FIX.4.2")
        {
            _writer.push_back_header("FIX.4.2");
        } else
        {
            std::string temp(begin_string);
            _writer.push_back_header(temp.c_str());
        }
    }

    inline void add_string(int tag, std::string_view value) noexcept
    {
        _writer.push_back_string(tag,
                                 value.data(),
                                 value.data() + value.size());
    }

    template <typename IntType>
    inline void add_int(int tag, IntType value) noexcept
    {
        _writer.push_back_int(tag, value);
    }

    inline void add_char(int tag, char value) noexcept
    {
        _writer.push_back_char(tag, value);
    }

    template <typename IntType>
    inline void
    add_decimal(int tag, IntType mantissa, IntType exponent) noexcept
    {
        _writer.push_back_decimal(tag, mantissa, exponent);
    }

    inline void end() noexcept { _writer.push_back_trailer(); }

    [[nodiscard]] inline const char *data() const noexcept
    {
        return _writer.message_begin();
    }

    [[nodiscard]] inline size_t size() const noexcept
    {
        return _writer.message_size();
    }

    [[nodiscard]] inline std::string_view view() const noexcept
    {
        return std::string_view(data(), size());
    }

  private:
    std::variant<std::vector<char>, char *> _storage;
    hffix::message_writer                   _writer;
};

class parser
{
  public:
    parser(const char *data, size_t size) noexcept
        : _reader(data, size)
    {
    }

    explicit parser(std::string_view sv) noexcept
        : _reader(sv.data(), sv.size())
    {
    }

    ~parser()                         = default;
    parser(const parser &)            = delete;
    parser &operator=(const parser &) = delete;
    parser(parser &&)                 = default;
    parser &operator=(parser &&)      = default;

    [[nodiscard]] inline bool valid() const noexcept
    {
        return _reader.is_valid();
    }
    [[nodiscard]] inline bool complete() const noexcept
    {
        return _reader.is_complete();
    }

    template <typename Visitor>
    inline void for_each(Visitor &&visitor) const
    {
        for(auto it = _reader.begin(); it != _reader.end(); ++it)
        {
            visitor(it->tag(), it->value());
        }
    }

    [[nodiscard]] inline std::optional<std::string_view>
    get_string(int tag) const noexcept
    {
        for(auto it = _reader.begin(); it != _reader.end(); ++it)
        {
            if(it->tag() == tag)
            {
                const auto &val = it->value();
                return std::string_view(val.begin(), val.size());
            }
        }
        return std::nullopt;
    }

    template <typename IntType = int>
    [[nodiscard]] inline std::optional<IntType> get_int(int tag) const noexcept
    {
        for(auto it = _reader.begin(); it != _reader.end(); ++it)
        {
            if(it->tag() == tag)
            {
                try
                {
                    return it->value().template as_int<IntType>();
                }
                catch(...)
                {
                    return std::nullopt;
                }
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] inline std::optional<char> get_char(int tag) const noexcept
    {
        for(auto it = _reader.begin(); it != _reader.end(); ++it)
        {
            if(it->tag() == tag)
            {
                return it->value().as_char();
            }
        }
        return std::nullopt;
    }

  private:
    hffix::message_reader _reader;
};

} // namespace fix
} // namespace hj

#endif // FIX_HPP