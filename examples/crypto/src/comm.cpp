#include "comm.h"

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

std::string select_encrypt_output_fmt(
    std::string& fmt,
    const std::string& out,
    const std::string& algo)
{
    if (algo == "base64" || algo == "md5")
        fmt = "";

    if (algo == "aes" || algo == "des" || algo == "sha256" || algo == "rsa")
    {
        // to console
        if (out.empty())
            fmt = (fmt == "") ? "base64" : fmt;
        else // to file
            fmt = "";
    }

    return fmt;
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
    switch (otype)
    {
        case output_type::console:
            for (const auto& msg : msgs)
                std::cout << msg << std::endl;
            break;
        default:
            break;
    }
}