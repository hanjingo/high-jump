#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#if __has_include(<boost/config.hpp>)
#include <boost/serialization/singleton.hpp>
namespace hj
{

template <typename T>
using singleton = boost::serialization::singleton<T>;

}

#else
#error "No suitable singleton implementation found"

#endif

#endif