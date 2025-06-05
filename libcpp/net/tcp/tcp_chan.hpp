#ifndef TCP_CHAN_HPP
#define TCP_CHAN_HPP

#include <concurrentqueue/blockingconcurrentqueue.h>

namespace libcpp
{

template<typename T>
struct tcp_chan {
    tcp_chan(const std::size_t min_capa) 
        : q_{min_capa * moodycamel::BlockingConcurrentQueue<T>::BLOCK_SIZE} 
    {};
    ~tcp_chan() 
    {};

    inline tcp_chan& operator>>(T& t)
    {
        q_.wait_dequeue(t); // blocks until an item is available
        return *this;
    }

    inline tcp_chan& operator<<(const T& t)
    {
        q_.enqueue(t);
        return *this;
    }

private:
    moodycamel::BlockingConcurrentQueue<T> q_;
};

}

#endif