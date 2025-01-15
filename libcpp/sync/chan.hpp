#ifndef CHAN_HPP
#define CHAN_HPP

#include <concurrentqueue/moodycamel/blockingconcurrentqueue.h>

namespace libcpp
{

template<typename T>
struct chan {
    chan(std::size_t capa = 6 * moodycamel::BlockingConcurrentQueue<T>::BLOCK_SIZE) : q_{capa} {};
    ~chan() {};

    inline chan& operator>>(T& t)
    {
        q_.wait_dequeue(t);
        return *this;
    }

    inline chan& operator<<(const T& t)
    {
        q_.enqueue(t);
        return *this;
    }

private:
    moodycamel::BlockingConcurrentQueue<T> q_;
};

}

#endif