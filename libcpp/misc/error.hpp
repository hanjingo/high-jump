#ifndef ERROR_HPP
#define ERROR_HPP

#include <string>

namespace libcpp
{
class error
{
public:
    error(uint32_t err_code = 0, const std::string& desc = "") : code {err_code}, desc {desc} {}
    ~error() {}

    inline bool operator==(const error& e) const
    {
        return code == e.code;
    }

    inline bool operator!=(const error& e) const
    {
        return code != e.code;
    }

    inline error& operator=(const error& e)
    {
        code = e.code;
        desc = e.desc;
        return *this;
    }

    uint32_t code;
    std::string desc;
};

}

#endif