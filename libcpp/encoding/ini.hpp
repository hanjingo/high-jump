#ifndef INI_HPP
#define INI_HPP

#include <unistd.h>
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

    static ini parse(const char* text)
    {
        std::stringstream ss(text);
        boost::property_tree::ptree tree;
        boost::property_tree::ini_parser::read_ini(ss, tree);
        return ini(tree);
    }

    bool read_file(const char* filepath)
    {
        if (access(filepath, F_OK) == -1)
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
};

}

#endif