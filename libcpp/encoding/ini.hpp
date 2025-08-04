#ifndef INI_HPP
#define INI_HPP

#include <string>

#include <boost/filesystem.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace libcpp {

class ini : public boost::property_tree::ptree
{
  public:
    ini() {}
    ini(const ini& rhs) : boost::property_tree::ptree(rhs) {}
    ini(const boost::property_tree::ptree& tree)
        : boost::property_tree::ptree(tree)
    {
    }
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

}  // namespace libcpp

#endif