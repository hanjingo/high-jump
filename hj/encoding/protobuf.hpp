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

#ifndef PROTOBUF_HPP
#define PROTOBUF_HPP

#include <string>
#include <fstream>
#include <iostream>
#include <google/protobuf/message.h>

namespace hj
{
namespace pb
{

using msg_t      = ::google::protobuf::Message;
using msg_lite_t = ::google::protobuf::MessageLite;

enum class error_code
{
    ok = 0,
    serialize_fail,
    deserialize_fail,
    io_fail
};

// ------------ serialize ----------------
inline error_code serialize(std::string &dst, const msg_t &msg)
{
    return msg.SerializeToString(&dst) ? error_code::ok
                                       : error_code::serialize_fail;
}

inline error_code serialize(std::ostream &dst, const msg_t &msg)
{
    if(!dst.good())
        return error_code::io_fail;

    std::string buf;
    if(!msg.SerializeToString(&buf))
        return error_code::serialize_fail;

    dst.write(buf.data(), buf.size());
    return dst.good() ? error_code::ok : error_code::io_fail;
}

// ----------------- deserialize -------------------
inline error_code deserialize(msg_t &dst, const std::string &src)
{
    return dst.ParseFromString(src) ? error_code::ok
                                    : error_code::deserialize_fail;
}

inline error_code deserialize(msg_t &dst, std::istream &src)
{
    if(!src.good())
        return error_code::io_fail;

    std::string buf((std::istreambuf_iterator<char>(src)),
                    std::istreambuf_iterator<char>());
    return dst.ParseFromString(buf) ? error_code::ok
                                    : error_code::deserialize_fail;
}

inline error_code serialize(std::string &dst, const msg_lite_t &msg)
{
    return msg.SerializeToString(&dst) ? error_code::ok
                                       : error_code::serialize_fail;
}

inline error_code deserialize(msg_lite_t &dst, const std::string &src)
{
    return dst.ParseFromString(src) ? error_code::ok
                                    : error_code::deserialize_fail;
}


} // namespace pb
} // namespace hj

#endif