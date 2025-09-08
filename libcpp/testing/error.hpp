#ifndef ERROR_HPP
#define ERROR_HPP

#include <map>
#include <mutex>
#include <string>
#include <sstream>
#include <system_error>

namespace libcpp
{

namespace err_detail 
{

class error_category : public std::error_category 
{
public:
    error_category(const char* n) : _name(n) {}
    const char* name() const noexcept override { return _name.c_str(); }
    std::string message(int ev) const override 
    {
        std::lock_guard<std::mutex> lock(_mtx);
        auto it = _messages.find(ev);
        if (it != _messages.end())
            return it->second;

        return "unknown error";
    }

    void register_message(int ec, const std::string& msg) 
    {
        std::lock_guard<std::mutex> lock(_mtx);
        _messages[ec] = msg;
    }

private:
    std::string _name;
    mutable std::mutex _mtx;
    std::map<int, std::string> _messages;
};

inline error_category& category(const char* name) 
{
    static std::mutex mtx;
    static std::map<std::string, error_category*> cats;
    std::lock_guard<std::mutex> lock(mtx);
    auto it = cats.find(name);
    if (it != cats.end()) return *it->second;
    auto* cat = new error_category(name);
    cats[name] = cat;
    return *cat;
}

} // namespace err_detail

static inline void register_err(const char* category, int ec, const std::string& desc = "") 
{
    libcpp::err_detail::category(category).register_message(ec, desc);
}

static inline std::error_code make_err(int e, const char* catname = "") 
{
    return std::error_code(e, err_detail::category(catname));
}

template <typename T>
static int ec_to_int(const T err, int mask = (~0)) 
{ 
    return static_cast<int>(err) & mask; 
}

template <typename T>
static std::string ec_to_hex(const T err, bool upper_case = true, std::string prefix = "0x") 
{
    std::ostringstream ss;
    ss << (upper_case ? std::uppercase : std::nouppercase) << std::hex << static_cast<int>(err);
    return prefix.append(ss.str());
}

}

#endif