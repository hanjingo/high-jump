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

using msg_t = ::google::protobuf::Message;

// ------------ serialize ----------------
inline bool serialize(std::string &dst, const msg_t &msg)
{
    return msg.SerializeToString(&dst);
}

inline bool serialize(std::ostream &dst, const msg_t &msg)
{
    std::string buf;
    if(!msg.SerializeToString(&buf))
        return false;

    dst.write(buf.data(), buf.size());
    return dst.good();
}

// ----------------- deserialize -------------------
inline bool deserialize(msg_t &dst, const std::string &src)
{
    return dst.ParseFromString(src);
}

inline bool deserialize(msg_t &dst, std::istream &src)
{
    std::string buf((std::istreambuf_iterator<char>(src)),
                    std::istreambuf_iterator<char>());
    return dst.ParseFromString(buf);
}


} // namespace pb
} // namespace hj

#endif