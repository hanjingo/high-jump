#ifndef BASE64_HPP
#define BASE64_HPP

#include <string>
#include <sstream>

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>

namespace libcpp
{

class base64
{
    using encode_std_str_itr = boost::archive::iterators::base64_from_binary <
                               boost::archive::iterators::transform_width<std::string::const_iterator, 6, 8 >>;
    using decode_std_str_itr = boost::archive::iterators::transform_width <
                               boost::archive::iterators::binary_from_base64<std::string::const_iterator>, 8, 6 >;

public:
    static std::string encode(const std::string& in)
    {
        std::stringstream buf;
        std::copy(encode_std_str_itr(in.begin()), encode_std_str_itr(in.end()), std::ostream_iterator<char>(buf));

        std::size_t equal_count = (3 - in.length() % 3) % 3;
        for (size_t i = 0; i < equal_count; i++) {
            buf.put('=');
        }

        return buf.str();
    }

    static std::string decode(const std::string& in)
    {
        std::stringstream buf;
        std::copy(decode_std_str_itr(in.begin()), decode_std_str_itr(in.end()), std::ostream_iterator<char>(buf));
        
        return buf.str();
    }
};

}

#endif