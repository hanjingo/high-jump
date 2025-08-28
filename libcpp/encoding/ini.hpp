#ifndef INI_HPP
#define INI_HPP

#include <functional>

// c++ std::unary_function compatibility
#ifndef LIBCPP_UNARY_FUNCTION_DEFINED
    #if defined(_MSC_VER)
        #if (_MSC_VER >= 1910)
            #define LIBCPP_UNARY_FUNCTION_DEFINED 0
        #else
            #define LIBCPP_UNARY_FUNCTION_DEFINED 1
        #endif
    #elif (__cplusplus >= 201703L)
        #if defined(__GLIBCXX__)
            #define LIBCPP_UNARY_FUNCTION_DEFINED 1
        #elif defined(_LIBCPP_VERSION)
            #define LIBCPP_UNARY_FUNCTION_DEFINED 0
        #else
            #define LIBCPP_UNARY_FUNCTION_DEFINED 0
        #endif
    #else
        #define LIBCPP_UNARY_FUNCTION_DEFINED 1
    #endif
#endif

#if !LIBCPP_UNARY_FUNCTION_DEFINED
namespace std {
    template <class Arg, class Result>
    struct unary_function {
        typedef Arg argument_type;
        typedef Result result_type;
    };
}
#undef LIBCPP_UNARY_FUNCTION_DEFINED
#define LIBCPP_UNARY_FUNCTION_DEFINED 1
#endif

#include <string>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace libcpp
{

class ini : public boost::property_tree::ptree
{
public:
    ini() {}
    ini(const ini& rhs) : boost::property_tree::ptree(rhs) {}
    ini(const boost::property_tree::ptree& tree) : boost::property_tree::ptree(tree) {}
    ~ini() {}

    ini& operator=(const ini& rhs) 
    {
        boost::property_tree::ptree::operator=(rhs);
        return *this;
    }

    ini& operator=(const boost::property_tree::ptree& rhs) 
    {
        boost::property_tree::ptree::operator=(rhs);
        return *this;
    }

    static ini parse(const char* text)
    {
        std::stringstream ss(text);
        boost::property_tree::ptree tree;
        boost::property_tree::ini_parser::read_ini(ss, tree);
        return ini(tree);
    }

    bool read_file(const char* filepath)
    {
        if (!boost::filesystem::exists(filepath))
            return false;

        boost::property_tree::ptree tree;
        boost::property_tree::ini_parser::read_ini(filepath, tree);
        *this = tree;
        return true;
    }

    void write_file(const char* filepath)
    {
        boost::property_tree::ini_parser::write_ini(filepath, *this);
    }

    std::string str()
    {
        std::ostringstream ss;
        boost::property_tree::write_ini(ss, *this);
        return ss.str();
    }
};

}

#endif