#include "comm.h"

#include <iostream>

std::string fmt_strs(const std::vector<std::string>& vec)
{
    std::string s;
    for (const auto& e : vec)
        s += e + ", ";

    s = s.substr(0, s.size() - 2);  // remove trailing comma and space
    return s;
}

output_type select_output_type(const std::string& out)
{
    if (out.empty()) 
        return output_type::console;
    else 
        return output_type::file;
}

void print(const std::string& msg, const output_type& otype)
{
    switch (otype)
    {
        case output_type::console:
            std::cout << msg << std::endl;
            break;
        default:
            break;
    }
}

void print(const std::vector<std::string>& msgs, const output_type& otype)
{
    for (const auto& msg : msgs)
        print(msg, otype);
}

void print(const std::vector<std::vector<std::string>>& msgs, const output_type& otype)
{
    for (const auto& row : msgs)
        for (const auto& msg : row)
            print(msg, otype);
}