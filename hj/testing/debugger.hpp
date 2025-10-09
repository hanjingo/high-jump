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

#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <mutex>

#include <fmt/core.h>
#include <boost/asio.hpp>

#ifndef DEBUG_BUF_SIZE
#define DEBUG_BUF_SIZE 4096
#endif

namespace hj
{

class debugger
{
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
    inline std::string fmt(const char *style, const Args &...args) const
    {
        return _fmt(style, std::forward<const Args &>(args)...);
    }

    template <typename... Args>
    inline void print(const char *style, const Args &...args) const
    {
        *_os << debugger::fmt(style, std::forward<const Args &>(args)...)
             << std::endl;
    }

    inline void set_ostream(std::ostream &os)
    {
        std::lock_guard<std::mutex> lock(_mu);
        _os = &os;
    }

  private:
    static std::string
    _fmt(const char *style, const unsigned char *data, const size_t len)
    {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        size_t count = 0;
        for(size_t i = 0; i < len; ++i)
        {
            if(i > 0)
                oss << ' ';

            oss << fmt::format(style, data[i]);
            if(++count >= DEBUG_BUF_SIZE)
            {
                oss << " ...";
                break;
            }
        }
        return oss.str();
    }

    static std::string _fmt(const char *style, const char *data)
    {
        std::size_t len = strlen(data);
        return _fmt(style, reinterpret_cast<const unsigned char *>(data), len);
    }

    static std::string _fmt(const char *style, const std::vector<uint8_t> &buf)
    {
        return _fmt(style, buf.data(), buf.size());
    }

    static std::string _fmt(const char                   *style,
                            const boost::asio::streambuf &buf)
    {
        std::string str(boost::asio::buffers_begin(buf.data()),
                        boost::asio::buffers_end(buf.data()));
        return _fmt(style,
                    reinterpret_cast<const unsigned char *>(str.data()),
                    str.size());
    }

    std::ostream *_os;
    std::mutex    _mu;
};

}

#ifdef DEBUG
#define PRINT(style, ...) hj::debugger::instance().print(style, ##__VA_ARGS__)
#else
#define PRINT(style, ...)
#endif

#endif