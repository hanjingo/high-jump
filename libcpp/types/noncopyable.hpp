#ifndef NONCOPYABLE_HPP
#define NONCOPYABLE_HPP

#ifdef BOOST_FOUND
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


#define DISABLE_COPY(Class) \
    Class(const Class &) = delete;\
    Class &operator=(const Class &) = delete;

#define DISABLE_MOVE(Class) \
    Class(Class &&) = delete; \
    Class &operator=(Class &&) = delete;

#define DISABLE_COPY_MOVE(Class) \
    DISABLE_COPY(Class) \
    DISABLE_MOVE(Class)

#endif