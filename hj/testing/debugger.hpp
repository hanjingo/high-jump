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

#if __has_include(<boost/asio/buffer.hpp>) &&           \
    __has_include(<boost/asio/streambuf.hpp>) &&        \
    __has_include(<boost/asio/buffers_iterator.hpp>)
#include <boost/asio/buffer.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/buffers_iterator.hpp>

#ifndef HJ_DEBUGGER_HAS_BOOST_ASIO
#define HJ_DEBUGGER_HAS_BOOST_ASIO 1
#endif
#endif

namespace hj
{

class debugger;

/**
 * @brief RAII Guard for temporarily redirecting the process-wide debugger output stream.
 *
 * ### Concurrency & Scope Contract:
 * - **Process-Wide Impact**: This guard alters the global output sink of `debugger::instance()`.
 *   It is **NOT thread-local**. Redirects affect all threads calling `print()` concurrently.
 * - **LIFO Nesting Requirement**: In multi-threaded environments, interleaved instantiation and 
 *   destruction of `ostream_guard` across different threads will corrupt the stream restoration 
 *   stack, leading to dangling `std::ostream` pointers or incorrect sink destinations.
 * - **Recommended Usage**: Strictly intended for **single-threaded test fixtures**, 
 *   **sequential test execution**, or **process startup/initialization phases**.
 */
class ostream_guard
{
  public:
    explicit ostream_guard(std::ostream &new_os);
    ~ostream_guard();

    ostream_guard(const ostream_guard &)            = delete;
    ostream_guard &operator=(const ostream_guard &) = delete;
    ostream_guard(ostream_guard &&other) noexcept;
    ostream_guard &operator=(ostream_guard &&other) noexcept;

  private:
    std::ostream *_prev_os{nullptr};
    bool          _active{true};
};

/**
 * @brief Non-owning view of a contiguous sequence of bytes.
 *
 * Semantic contracts:
 * - data == nullptr && size == 0 : Empty view (valid, represents 0 bytes).
 * - data != nullptr && size == 0 : Empty view (valid, represents 0 bytes).
 * - data == nullptr && size != 0 : Invalid view (dangling or uninitialized pointer).
 */
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

    template <typename T, size_t N, typename = std::enable_if_t<sizeof(T) == 1>>
    constexpr bytes_view(T (&arr)[N]) noexcept
        : _data{reinterpret_cast<const unsigned char *>(arr)}
        , _size{N}
    {
    }

    constexpr const unsigned char *data() const noexcept { return _data; }

    constexpr size_t size() const noexcept { return _size; }

    /// Returns true if the byte sequence is logically empty (size == 0).
    constexpr bool empty() const noexcept { return _size == 0; }

    /// Returns true if this view holds an invalid dangling pointer (data == nullptr && size != 0).
    constexpr bool is_invalid() const noexcept
    {
        return _data == nullptr && _size != 0;
    }

  private:
    const unsigned char *_data;
    size_t               _size;
};

/**
 * @brief Thread-safe logging and hex-formatting utility.
 *
 * ### Concurrency & Lifetime Safety Contract:
 * - **Data Access Synchronization**: Member methods (`print`, `set_ostream`, `reset_ostream`, `flush`) 
 *   are synchronized via internal `std::mutex`. Simultaneous calls from multiple threads 
 *   will not cause data races on internal state or output interleaving per line.
 * - **Sink Lifetime Responsibility**: The `debugger` instance holds a **NON-OWNING** 
 *   pointer to `std::ostream`. It is the **CALLER'S RESPONSIBILITY** to guarantee that 
 *   the target `std::ostream` outlives all concurrent or subsequent `print()` operations.
 * - **Dangling Sink Hazard**: Dynamically replacing the output stream (e.g., via `set_ostream` 
 *   or `ostream_guard`) with a local/temporary stream while other worker threads are 
 *   actively logging can lead to USE-AFTER-FREE (dangling stream access). Ensure stream 
 *   redirects are properly scoped or synchronized at application boundaries.
 */
class debugger
{
    friend class ostream_guard;

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
        *_os << formatted << '\n';
    }

    inline void flush()
    {
        std::lock_guard<std::mutex> lock(_mu);
        if(_os)
            _os->flush();
    }

    /**
     * @brief Factory method: Redirects output stream and returns an RAII Guard.
     * @details The stream redirection is held until the returned `ostream_guard` goes out of scope.
     * @param os Target output stream.
     * @return `ostream_guard` object managing the scope of this redirection.
     */
    [[nodiscard]] inline ostream_guard set_ostream(std::ostream &os)
    {
        return ostream_guard(os);
    }

    /// Resets stream destination back to standard std::cout.
    inline void reset_ostream()
    {
        std::lock_guard<std::mutex> lock(_mu);
        _os = &std::cout;
    }

  private:
    inline std::ostream *_set_ostream_impl(std::ostream &os)
    {
        std::lock_guard<std::mutex> lock(_mu);
        std::ostream               *prev = _os;
        _os                              = &os;
        return prev;
    }

    inline void _restore_ostream_impl(std::ostream *prev)
    {
        std::lock_guard<std::mutex> lock(_mu);
        if(prev)
        {
            _os = prev;
        } else
        {
            _os = &std::cout;
        }
    }

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

#if defined(HJ_DEBUGGER_HAS_BOOST_ASIO)
    template <>
    struct is_custom_buffer<boost::asio::streambuf> : std::true_type
    {
    };
#endif

    static std::string _fmt_bytes(bytes_view view, bool truncated = false)
    {
        if(view.empty())
        {
            return "";
        }

        if(view.is_invalid())
        {
            return "<null>";
        }

        fmt::memory_buffer out;
        const size_t       len       = view.size();
        const auto        *data      = view.data();
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
        {
            return "<null>";
        }
        return _fmt_bytes(bytes_view(data, std::strlen(data)));
    }

    static std::string _fmt_impl(const char                 *style,
                                 const std::vector<uint8_t> &buf)
    {
        (void) style;
        return _fmt_bytes(bytes_view(buf.data(), buf.size()));
    }

#if defined(HJ_DEBUGGER_HAS_BOOST_ASIO)
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
#endif

    static std::string _dispatch(const char *style)
    {
        return std::string(style);
    }

    template <typename Arg1>
    static std::string _dispatch(const char *style, Arg1 &&arg1)
    {
        using RawArg1 = std::remove_cv_t<std::remove_reference_t<Arg1>>;

        if constexpr(std::is_array_v<RawArg1>
                     && sizeof(std::remove_extent_t<RawArg1>) == 1)
        {
            return _fmt_bytes(bytes_view(arg1, std::extent_v<RawArg1>));
        } else if constexpr(is_custom_buffer<RawArg1>::value)
        {
            return _fmt_impl(style, std::forward<Arg1>(arg1));
        } else if constexpr(std::is_same_v<RawArg1, const char *>
                            || std::is_same_v<RawArg1, char *>)
        {
            return _fmt_impl(style, arg1);
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

inline ostream_guard::ostream_guard(std::ostream &new_os)
    : _prev_os(debugger::instance()._set_ostream_impl(new_os))
    , _active(true)
{
}

inline ostream_guard::~ostream_guard()
{
    if(_active)
        debugger::instance()._restore_ostream_impl(_prev_os);
}

inline ostream_guard::ostream_guard(ostream_guard &&other) noexcept
    : _prev_os(other._prev_os)
    , _active(other._active)
{
    other._active = false;
}

inline ostream_guard &ostream_guard::operator=(ostream_guard &&other) noexcept
{
    if(this != &other)
    {
        if(_active)
            debugger::instance()._restore_ostream_impl(_prev_os);

        _prev_os      = other._prev_os;
        _active       = other._active;
        other._active = false;
    }
    return *this;
}

} // namespace hj

#ifdef DEBUG
#define HJ_PRINT(style, ...)                                                   \
    hj::debugger::instance().print(style, ##__VA_ARGS__)
#else
#define HJ_PRINT(style, ...) ((void) 0)
#endif

#endif // DEBUGGER_HPP