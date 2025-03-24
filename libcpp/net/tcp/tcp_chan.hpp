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
        _q->wait_dequeue(t);
        return *this;
    }

    inline tcp_chan& operator<<(const T& t)
    {
        _q->enqueue(t);
        return *this;
    }

    std::size_t resize(const std::size_t sz)
    {
        moodycamel::BlockingConcurrentQueue<T>* tmp = 
            new moodycamel::BlockingConcurrentQueue<T>(sz * moodycamel::BlockingConcurrentQueue<T>::BLOCK_SIZE);
        for (; sz > 0 && !_q->empty(); sz--)
            tmp << _q;

        delete _q;
        _q = tmp;
    }

private:
    moodycamel::BlockingConcurrentQueue<T>* _q;
};

}

#endif