#ifndef CHAN_HPP
#define CHAN_HPP

#include <concurrentqueue/blockingconcurrentqueue.h>

#ifndef CHAN_CAPA
#define CHAN_CAPA 6
#endif

namespace libcpp
{

template<typename T>
struct chan {
    chan(std::size_t capa = CHAN_CAPA * moodycamel::BlockingConcurrentQueue<T>::BLOCK_SIZE) : q_{capa} {};
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

    inline void wait_dequeue(T& t)
    {
        q_.wait_dequeue(t);
    }

    inline bool enqueue(const T& t)
    {
        return q_.enqueue(t);
    }

private:
    moodycamel::BlockingConcurrentQueue<T> q_;
};

}

#endif