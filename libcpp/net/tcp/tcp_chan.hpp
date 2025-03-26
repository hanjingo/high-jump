#ifndef TCP_CHAN_HPP
#define TCP_CHAN_HPP

#include <concurrentqueue/moodycamel/blockingconcurrentqueue.h>

#ifndef CHAN_CAPA
#define CHAN_CAPA 6
#endif

namespace libcpp
{

template<typename T>
struct tcp_chan {
    tcp_chan(std::size_t capa = CHAN_CAPA) 
        : _q{new moodycamel::BlockingConcurrentQueue<T>(capa * moodycamel::BlockingConcurrentQueue<T>::BLOCK_SIZE)} 
    {};
    ~tcp_chan() 
    {
        delete _q;
        _q = nullptr;
    };

    inline tcp_chan& operator>>(T& t)
    {
        t = _q->try_dequeue(t) ? t : nullptr;
        return *this;
    }

    inline tcp_chan& operator<<(const T& t)
    {
        _q->enqueue(t);
        return *this;
    }

private:
    moodycamel::BlockingConcurrentQueue<T>* _q;
};

}

#endif