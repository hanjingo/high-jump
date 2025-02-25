#ifndef INI_HPP
#define INI_HPP

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

    static ini parse(const char* text)
    {
        std::stringstream ss(text);
        boost::property_tree::ptree tree;
        boost::property_tree::ini_parser::read_ini(ss, tree);
        return ini(tree);
    }

    bool read_file(const char* filepath)
    {
        std::cout << "fuck" << std::endl;
        auto path = boost::filesystem::path(filepath);
        std::cout << "fuck1" << std::endl;
        if (!boost::filesystem::exists(path))
            return false;

        std::cout << "fuck2 = " << path.string() << std::endl;
        boost::property_tree::ptree tree;
        boost::property_tree::ini_parser::read_ini(path.string(), tree);
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