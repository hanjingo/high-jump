#include <iostream>

#include <libcpp/testing/error.hpp>
#include <boost/optional.hpp>

enum err {
    err_ok = 0,
    err_fail,
    err_fatal,
};
using enum_err_t = libcpp::error<err>;
using enum_err_v2_t = libcpp::error_v2<err>;
void on_err_fail()
{
    std::cout << "do handle err_fail" << std::endl;
}
std::string on_err_fatal(int& arg)
{
    arg = 5;
    return std::string("on_err_fatal return");
}

using std_err_t = libcpp::error<std::error_code>;
class std_err_category : public std::error_category
{
public:
    const char* name() const noexcept override { return "my_err_category"; }
    std::string message(int ev) const override
    {
        switch (ev)
        {
        case 0:
            return "OK";
        case 1:
            return "FAIL";
        default:
            return "";
        }
    } 
};
std_err_category my_err_category{};

using opt_err_t = libcpp::error<boost::optional<int> >;
opt_err_t hello(bool cond)
{
    if (cond)
        return opt_err_t(0);
    else
        return boost::optional<int>();
}

int main(int argc, char* argv[])
{
    // enum
    std::cout << "libcpp::error<err> example>>>>>>>>>>>>>>>>>" << std::endl;
    std::cout << "enum_err_t(err_ok) = " << enum_err_t(err_ok) << std::endl;
    std::cout << std::endl;

    std::cout << "error_factory::create(err_ok) = " 
        << libcpp::error_factory::create(err_ok) << std::endl;
    std::cout << "error_factory::create(err_fail) = " 
        << libcpp::error_factory::create(err_fail) << std::endl;
    std::cout << std::endl;

    std::cout << "ERROR(err_ok) = " << ERROR(err_ok) << std::endl;
    std::cout << "(ERROR(err_fail) == ERROR(err_fail)) = " 
        << (ERROR(err_fail) == ERROR(err_fail)) << std::endl;
    std::cout << std::endl;

    // std::error_code
    std::cout << "libcpp::error<std::error_code> example>>>>>>>>>>>>>>>>>" << std::endl;
    std::cout << "std_err_t({0, my_err_category}) = " 
        << std_err_t({0, my_err_category}) << std::endl;
    std::cout << "std_err_t(std::error_code(0, my_err_category)).message() = " 
        << std_err_t({1, my_err_category}).value().message() << std::endl;
    std::cout << std::endl;

    std::cout << "error_factory::create(std_err_t({0, my_err_category})) = " 
        << libcpp::error_factory::create(std_err_t({0, my_err_category})) << std::endl;
    std::cout << "error_factory::create(std_err_t({1, my_err_category})) = " 
        << libcpp::error_factory::create(std_err_t({1, my_err_category})) << std::endl;
    std::cout << std::endl;

    std::cout << "ERROR(std_err_t({0, my_err_category})) = " 
        << ERROR(std_err_t({0, my_err_category})) << std::endl;
    std::cout << "ERROR(std_err_t({1, my_err_category})) = " 
        << ERROR(std_err_t({1, my_err_category})) << std::endl;
    std::cout << std::endl;

    // boost::optinal
    std::cout << "libcpp::error<boost::optional<int> > example>>>>>>>>>>>>>>>>>" << std::endl;
    std::cout << "opt_err_t(1).value().get() = " << opt_err_t(1).value().get() << std::endl;
    std::cout << "hello(true).value().get() = " << hello(true).value().get() << std::endl;
    std::cout << std::endl;

    // error_v2 + enum
    std::cout << "libcpp::error_v2<err> example>>>>>>>>>>>>>>>>>" << std::endl;
    std::cout << "enum_err_v2_t(err_ok) = " << enum_err_v2_t(err_ok) << std::endl;
    auto err_v2_fail = enum_err_v2_t(err_fail, (void*)(on_err_fail));
    std::cout << "err_v2_fail = " << err_v2_fail << std::endl;
    std::cout << "err_v2_fail.handle()> " << std::endl;
    err_v2_fail.handle();
    auto err_v2_fatal = enum_err_v2_t(err_fatal, (void*)(on_err_fatal));
    std::cout << "err_v2_fatal = " << err_v2_fatal << std::endl;
    int arg = 0;
    std::cout << "err_v2_fatal.handle<std::string>(arg) = " << err_v2_fatal.handle<std::string>(arg) 
                << ", arg = " << arg << std::endl;
    std::cout << std::endl;

    auto err_v2_fail_create = libcpp::error_factory::create(err_fail, (void*)(on_err_fail));
    std::cout << "err_v2_fail_create = " << err_v2_fail_create << std::endl;
    std::cout << "err_v2_fail_create.handle()> " << std::endl;
    err_v2_fail_create.handle();
    std::cout << std::endl;

    auto err_v2_fatal_macro = ERROR(err_fatal, (void*)(on_err_fatal));
    std::cout << "err_v2_fatal_macro = " << err_v2_fatal_macro<< std::endl;
    int arg_macro = 1;
    std::cout << "err_v2_fatal_macro.handle<std::string>(arg_macro) = " << err_v2_fatal_macro.handle<std::string>(arg_macro) 
                << ", arg_macro = " << arg_macro << std::endl;
    std::cout << std::endl;

    std::cin.get();
    return 0;
}