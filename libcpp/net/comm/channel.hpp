#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <concurrentqueue/blockingconcurrentqueue.h>

namespace libcpp
{

template<typename T>
struct channel {
    channel(const std::size_t min_capa) 
        : q_{min_capa * moodycamel::BlockingConcurrentQueue<T>::BLOCK_SIZE} 
    {};
    ~channel() 
    {};

    inline channel& operator>>(T& t)
    {
        q_.try_dequeue(t); // noblock
        return *this;
    }

    inline channel& operator<<(const T& t)
    {
        q_.enqueue(t);
        return *this;
    }

    template<typename T>
    inline void wait_dequeue(T& t)
    {
        q_.wait_dequeue(t);
    }

    template<typename T>
	inline bool wait_dequeue_timeout(T& t, std::int64_t us)
    {
        return q_.wait_dequeue_timed(t, us);
    }

    template<typename T>
    inline bool try_dequeue(T& t)
    {
        return q_.try_dequeue(t);
    }

    template<typename T>
    inline void enqueue(const T& t)
    {
        q_.enqueue(t);
    }

private:
    moodycamel::BlockingConcurrentQueue<T> q_;
};

}

#endif