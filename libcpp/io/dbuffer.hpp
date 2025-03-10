#ifndef DBUFFER_HPP
#define DBUFFER_HPP

#include <atomic>
#include <mutex>
#include <iostream>
#include <functional>

namespace libcpp
{

template <typename Container>
class dbuffer
{
public:
    using copy_fn = std::function<bool(Container& src, Container& dst)>;

public:
    dbuffer(copy_fn&& copy = [](Container& src, Container& dst)->bool{ dst = src; return true; })
        : _back{&_back_data}, _front{&_front_data}, _copy{std::move(copy)} {}
    ~dbuffer() {}

    bool write(Container& value)
    {
        Container* back = _back.load();
        if (!_copy(value, *back))
        {
            return false;
        }

        swap();
        return true;
    }

    bool read(Container& value)
    {
        Container* front = _front.load();
        return _copy(*front, value);
    }

    void swap()
    {
        auto tmp = _front.load();
        _front.store(_back.load());
        _back.store(tmp);
    }

private:
    Container _back_data;
    Container _front_data;
    std::atomic<Container*> _back;
    std::atomic<Container*> _front;
    copy_fn _copy;
    std::mutex _mu;
};

}

#endif