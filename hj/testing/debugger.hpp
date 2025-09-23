#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

#include <boost/asio.hpp>
#include <sstream>
#include <iostream>
#include <iomanip>

namespace hj
{

class debugger
{
public:
    static std::string to_string(const boost::asio::streambuf& buf, bool hex = true)
    {
#ifdef DEBUG
        std::string str(boost::asio::buffers_begin(buf.data()), boost::asio::buffers_end(buf.data()));
        if (!hex)
            return str;

        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (unsigned char c : str) 
        {
            if (c == '\0' || c == '\n')
                break; // stop at null or newline character

            oss << std::setw(2) << static_cast<int>(c) << " ";
        }

        return oss.str();
#else
        return std::string();
#endif
    }
};

}

#ifdef DEBUG
// Macro to print buffer contents
#define BUF_PRINT(buf, ...) \
    std::cout << hj::debugger::to_string(buf, ##__VA_ARGS__) << std::endl;

#else
#define BUF_PRINT(buf, ...)
#endif

#endif