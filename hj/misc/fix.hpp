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
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace hj
{
namespace fix
{

class builder
{
  public:
    builder(size_t bufsize = 1024)
        : _buffer{new std::vector<char>(bufsize)}
        , _writer{_buffer->data(), _buffer->size()}
    {
    }
    builder(char *buf, size_t bufsize)
        : _buffer{buf ? nullptr : new std::vector<char>(bufsize)}
        , _writer{buf ? buf : _buffer->data(), bufsize}
    {
    }
    ~builder()                          = default;
    builder(const builder &)            = delete;
    builder &operator=(const builder &) = delete;
    builder(builder &&)                 = default;
    builder &operator=(builder &&)      = default;

    inline void begin(const std::string &begin_string = "FIX.4.4")
    {
        _writer.push_back_header(begin_string.c_str());
    }

    inline void add_string(int tag, const std::string &value)
    {
        _writer.push_back_string(tag, value);
    }

    template <typename IntType>
    inline void add_int(int tag, IntType value)
    {
        _writer.push_back_int(tag, value);
    }

    inline void add_char(int tag, char value)
    {
        _writer.push_back_char(tag, value);
    }

    template <typename IntType>
    inline void add_decimal(int tag, IntType mantissa, IntType exponent)
    {
        _writer.push_back_decimal(tag, mantissa, exponent);
    }

    inline void        end() { _writer.push_back_trailer(); }
    inline const char *data() const { return _writer.message_begin(); }
    inline size_t      size() const { return _writer.message_size(); }
    inline std::string str() const { return std::string(data(), size()); }

  private:
    std::unique_ptr<std::vector<char>> _buffer;
    hffix::message_writer              _writer;
};

class parser
{
  public:
    parser(const char *data, size_t size)
        : _reader(data, size)
    {
    }

    ~parser()                         = default;
    parser(const parser &)            = delete;
    parser &operator=(const parser &) = delete;
    parser(parser &&)                 = default;
    parser &operator=(parser &&)      = default;

    inline bool valid() const noexcept { return _reader.is_valid(); }
    inline bool complete() const noexcept { return _reader.is_complete(); }

    inline std::string get_string(int tag) const
    {
        for(auto it = _reader.begin(); it != _reader.end(); ++it)
        {
            if(it->tag() == tag)
                return it->value().as_string();
        }
        return "";
    }

    template <typename IntType>
    inline IntType get_int(int tag) const
    {
        for(auto it = _reader.begin(); it != _reader.end(); ++it)
        {
            if(it->tag() == tag)
                return it->value().as_int<IntType>();
        }
        return IntType();
    }

    inline char get_char(int tag) const
    {
        for(auto it = _reader.begin(); it != _reader.end(); ++it)
        {
            if(it->tag() == tag)
                return it->value().as_char();
        }
        return '\0';
    }

  private:
    hffix::message_reader _reader;
};

} // namespace fix
} // namespace hj

#endif // FIX_HPP