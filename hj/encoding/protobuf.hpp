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