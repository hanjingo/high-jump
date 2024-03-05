#ifndef ERROR_HPP
#define ERROR_HPP

#include <string>
#include <iostream>
#include <system_error>
#include <unordered_map>

namespace libcpp
{

template<typename Code>
class error
{
    using any_t = void*;

public:
    error() = delete;
    error(const error& rhs) : code_{rhs.code_} {}
    error(Code code) : code_{code} {}

    ~error() {}

    inline error& operator=(const error& e)
    {
        code_ = e.code_;
        return *this;
    }

    inline error& operator=(const Code& arg)
    {
        code_ = arg;
        return *this;
    }

    inline friend bool operator==(const error& e1, const error& e2)
    {
        return e1.code_ == e2.code_;
    }

    inline friend bool operator!=(const error& e1, const error& e2)
    {
        return e1.code_ != e2.code_;
    }

    inline friend std::ostream& operator<<(std::ostream& out, const error& e)
    {
        out << e.code_;
        return out;
    }

    inline Code code()
    {
        return code_;
    }

private:
    Code code_;
};

class error_factory
{
public:
    error_factory() {}
    ~error_factory() {}

    template <typename Code = std::string>
    static error<Code> create(Code code)
    {
        return error<Code>(code);
    }
};

#define ERROR(...) \
    libcpp::error_factory::create(__VA_ARGS__)

}

#endif