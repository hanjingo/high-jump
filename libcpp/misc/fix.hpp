#ifndef FIX_HPP
#define FIX_HPP

#include <hffix.hpp>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace libcpp
{

class FixBuilder {
public:
    FixBuilder(size_t bufsize = 1024)
        : _buffer(bufsize)
        , _writer(_buffer.data(), _buffer.size()) 
    {}

    void begin(const std::string& begin_string = "FIX.4.4") 
    {
        _writer.push_back_header(begin_string.c_str());
    }

    void add_string(int tag, const std::string& value) 
    {
        _writer.push_back_string(tag, value);
    }

    template<typename IntType>
    void add_int(int tag, IntType value) 
    {
        _writer.push_back_int(tag, value);
    }

    void add_char(int tag, char value) 
    {
        _writer.push_back_char(tag, value);
    }

    template<typename IntType>
    void add_decimal(int tag, IntType mantissa, IntType exponent) 
    {
        _writer.push_back_decimal(tag, mantissa, exponent);
    }

    void end() 
    {
        _writer.push_back_trailer();
    }

    const char* data() const { return _writer.message_begin(); }
    size_t size() const { return _writer.message_size(); }
    std::string str() const { return std::string(data(), size()); }

private:
    std::vector<char> _buffer;
    hffix::message_writer _writer;
};

class FixParser {
public:
    FixParser(const char* data, size_t size)
        : _reader(data, size) 
    {}

    bool valid() const { return _reader.is_valid(); }
    bool complete() const { return _reader.is_complete(); }

    std::string get_string(int tag) const 
    {
        for (auto it = _reader.begin(); it != _reader.end(); ++it) 
        {
            if (it->tag() == tag) 
                return it->value().as_string();
        }
        return "";
    }

    template<typename IntType>
    IntType get_int(int tag) const 
    {
        for (auto it = _reader.begin(); it != _reader.end(); ++it) 
        {
            if (it->tag() == tag) 
                return it->value().as_int<IntType>();
        }
        return IntType();
    }

    char get_char(int tag) const 
    {
        for (auto it = _reader.begin(); it != _reader.end(); ++it) 
        {
            if (it->tag() == tag) 
                return it->value().as_char();
        }
        return '\0';
    }

private:
    hffix::message_reader _reader;
};

} // namespace libcpp

#endif // FIX_HPP