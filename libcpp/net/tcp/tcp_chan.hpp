#ifndef TCP_CHAN_HPP
#define TCP_CHAN_HPP

#include <concurrentqueue/moodycamel/blockingconcurrentqueue.h>

namespace libcpp
{

template<typename T>
struct tcp_chan {
    tcp_chan(std::size_t capa = 6) 
        : q_{capa * moodycamel::BlockingConcurrentQueue<T>::BLOCK_SIZE} {};
    ~tcp_chan() {};

    inline tcp_chan& operator>>(T& t)
    {
        // q_.wait_dequeue(t);
        t = (q_.try_dequeue(t) ? t : nullptr);
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