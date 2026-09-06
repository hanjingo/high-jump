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
#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <mutex>
#include <algorithm>
#include <cstring>
#include <type_traits>

#include <fmt/format.h>
#include <boost/asio.hpp>

namespace hj
{

class bytes_view
{
  public:
    constexpr bytes_view() noexcept
        : _data{nullptr}
        , _size{0}
    {
    }

    constexpr bytes_view(const void *data, size_t size) noexcept
        : _data{static_cast<const unsigned char *>(data)}
        , _size{size}
    {
    }

    template <size_t N>
    constexpr bytes_view(const unsigned char (&arr)[N]) noexcept
        : _data{arr}
        , _size{N}
    {
    }

    constexpr const unsigned char *data() const noexcept { return _data; }

    constexpr size_t size() const noexcept { return _size; }

  private:
    const unsigned char *_data;
    size_t               _size;
};

class debugger
{
  public:
    static constexpr size_t buf_sz = 4096;

  public:
    debugger()
        : _os{&std::cout}
    {
    }

    explicit debugger(std::ostream &os)
        : _os{&os}
    {
    }

    debugger(const debugger &)            = delete;
    debugger &operator=(const debugger &) = delete;
    debugger(debugger &&)                 = delete;
    debugger &operator=(debugger &&)      = delete;
    ~debugger()                           = default;

    static debugger &instance()
    {
        static debugger inst{};
        return inst;
    }

    template <typename... Args>
    inline std::string fmt(const char *style, Args &&...args) const
    {
        return _dispatch(style, std::forward<Args>(args)...);
    }

    template <typename... Args>
    inline void print(const char *style, Args &&...args) const
    {
        std::string formatted = fmt(style, std::forward<Args>(args)...);
        std::lock_guard<std::mutex> lock(_mu);
        *_os << formatted << std::endl;
    }

    inline std::ostream *set_ostream(std::ostream &os)
    {
        std::lock_guard<std::mutex> lock(_mu);
        std::ostream               *prev = _os;
        _os                              = &os;
        return prev;
    }

    inline void reset_ostream()
    {
        std::lock_guard<std::mutex> lock(_mu);
        _os = &std::cout;
    }

  private:
    template <typename T>
    struct is_custom_buffer : std::false_type
    {
    };

    template <>
    struct is_custom_buffer<bytes_view> : std::true_type
    {
    };

    template <>
    struct is_custom_buffer<std::vector<uint8_t>> : std::true_type
    {
    };

    template <>
    struct is_custom_buffer<boost::asio::streambuf> : std::true_type
    {
    };

    static std::string _fmt_bytes(bytes_view view, bool truncated = false)
    {
        const size_t len  = view.size();
        const auto  *data = view.data();
        if(len == 0)
            return "";

        if(data == nullptr)
            return "<null>";

        fmt::memory_buffer out;
        const size_t       print_len = std::min(len, buf_sz);
        for(size_t i = 0; i < print_len; ++i)
        {
            if(i > 0)
            {
                fmt::format_to(std::back_inserter(out), " ");
            }
            fmt::format_to(std::back_inserter(out),
                           fmt::runtime("{:02x}"),
                           data[i]);
        }

        if(truncated || len > buf_sz)
        {
            fmt::format_to(std::back_inserter(out), " ...");
        }

        return fmt::to_string(out);
    }

    static std::string _fmt_impl(const char *style, bytes_view view)
    {
        (void) style;
        return _fmt_bytes(view);
    }

    static std::string _fmt_impl(const char *style, const char *data)
    {
        (void) style;
        if(data == nullptr)
            return "<null>";

        return _fmt_bytes(bytes_view(data, std::strlen(data)));
    }

    static std::string _fmt_impl(const char                 *style,
                                 const std::vector<uint8_t> &buf)
    {
        (void) style;
        return _fmt_bytes(bytes_view(buf.data(), buf.size()));
    }

    static std::string _fmt_impl(const char                   *style,
                                 const boost::asio::streambuf &buf)
    {
        (void) style;
        auto   buffers   = buf.data();
        size_t total_len = boost::asio::buffer_size(buffers);
        size_t read_len  = std::min(total_len, buf_sz);

        std::vector<unsigned char> temp;
        temp.reserve(read_len);

        auto it = boost::asio::buffers_begin(buffers);
        for(size_t i = 0; i < read_len; ++i, ++it)
        {
            temp.push_back(static_cast<unsigned char>(*it));
        }

        return _fmt_bytes(bytes_view(temp.data(), temp.size()),
                          total_len > buf_sz);
    }

    static std::string _dispatch(const char *style)
    {
        return std::string(style);
    }

    template <typename Arg1>
    static std::string _dispatch(const char *style, Arg1 &&arg1)
    {
        using RawArg1 = std::decay_t<Arg1>;

        if constexpr(is_custom_buffer<RawArg1>::value)
        {
            return _fmt_impl(style, std::forward<Arg1>(arg1));
        } else if constexpr(std::is_same_v<RawArg1, const char *>
                            || std::is_same_v<RawArg1, char *>)
        {
            return _fmt_impl(style, arg1);
        } else if constexpr(std::is_array_v<RawArg1>
                            && std::is_same_v<std::remove_extent_t<RawArg1>,
                                              char>)
        {
            return _fmt_impl(style, static_cast<const char *>(arg1));
        } else
        {
            return fmt::format(fmt::runtime(style), std::forward<Arg1>(arg1));
        }
    }

    template <typename Arg1, typename Arg2, typename... Rest>
    static std::string
    _dispatch(const char *style, Arg1 &&arg1, Arg2 &&arg2, Rest &&...rest)
    {
        return fmt::format(fmt::runtime(style),
                           std::forward<Arg1>(arg1),
                           std::forward<Arg2>(arg2),
                           std::forward<Rest>(rest)...);
    }

    std::ostream      *_os;
    mutable std::mutex _mu;
};

class ostream_guard
{
  public:
    explicit ostream_guard(std::ostream &new_os)
        : _prev_os(debugger::instance().set_ostream(new_os))
    {
    }

    ~ostream_guard()
    {
        if(_prev_os)
        {
            debugger::instance().set_ostream(*_prev_os);
        } else
        {
            debugger::instance().reset_ostream();
        }
    }

    ostream_guard(const ostream_guard &)            = delete;
    ostream_guard &operator=(const ostream_guard &) = delete;

  private:
    std::ostream *_prev_os;
};

} // namespace hj

#ifdef DEBUG
#define HJ_PRINT(style, ...)                                                   \
    hj::debugger::instance().print(style, ##__VA_ARGS__)
#else
#define HJ_PRINT(style, ...) ((void) 0)
#endif

#endif // DEBUGGER_HPP