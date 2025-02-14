#ifndef TCP_CHAN_HPP
#define TCP_CHAN_HPP

#include <queue>

namespace libcpp
{

template<typename T>
class tcp_chan {
public:
    tcp_chan(std::size_t capa = 1) : q_{} {};
    ~tcp_chan() {};

    inline tcp_chan& operator>>(T& t)
    {
        t = q_.front();
        q_.pop();
        return *this;
    }

    inline tcp_chan& operator<<(const T& t)
    {
        q_.push(t);
        return *this;
    }

private:
    std::queue<T> q_;
};

}

#endif