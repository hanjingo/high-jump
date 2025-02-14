#ifndef INI_HPP
#define INI_HPP

#include <boost/any.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace libcpp
{

class ini
{
    using any_t = boost::any;
    using ptree_t = boost::property_tree::ptree;

public:
    ini() {}
    ini(ptree_t ptree) : ptree_{ptree} {}
    ~ini() {}

    static ini parse(const std::string& text)
    {
        std::stringstream ss(text);
        ptree_t tree;
        boost::property_tree::ini_parser::read_ini(ss, tree);
        return ini(tree);
    }

private:
    ptree_t ptree_;
};

}

#endif