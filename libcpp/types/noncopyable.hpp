#ifndef NONCOPYABLE_HPP
#define NONCOPYABLE_HPP

#ifdef BOOST_ENABLE
#include <boost/noncopyable.hpp>

namespace libcpp
{

using noncopyable = boost::noncopyable;

}

#else
namespace libcpp
{

class noncopyable
{
public:
    noncopyable() = default;
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
};

}

#endif

#endif