#ifndef RING_BUFFER_HPP
#define RING_BUFFER_HPP

#include <boost/circular_buffer.hpp>

namespace hj
{

template<typename T>
using ring_buffer = boost::circular_buffer<T>;

}

#endif