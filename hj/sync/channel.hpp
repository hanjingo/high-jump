#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <concurrentqueue/blockingconcurrentqueue.h>

namespace hj
{

template <typename T>
struct channel
{
    channel(const std::size_t min_capa)
        : _q{min_capa * moodycamel::BlockingConcurrentQueue<T>::BLOCK_SIZE} {};
    ~channel() {};

    inline channel &operator>>(T &t)
    {
        _q.try_dequeue(t); // noblock
        return *this;
    }

    inline channel &operator<<(const T &t)
    {
        _q.enqueue(t);
        return *this;
    }

    inline void wait_dequeue(T &t) { _q.wait_dequeue(t); }

    inline bool wait_dequeue_timeout(T &t, std::int64_t us)
    {
        return _q.wait_dequeue_timed(t, us);
    }

    inline bool try_dequeue(T &t) { return _q.try_dequeue(t); }

    inline void enqueue(const T &t) { _q.enqueue(t); }

  private:
    moodycamel::BlockingConcurrentQueue<T> _q;
};

}

#endif