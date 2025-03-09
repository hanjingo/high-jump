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
    dbuffer() : _back{&_back_data}, _front{&_front_data} {}
    ~dbuffer() {}

    bool write(
        Container& value, 
        copy_fn copy = [](Container& src, Container& dst)->bool{ dst = src; return true; })
    {
        _mu.lock();
        Container* back = _back.load();
        if (!copy(value, *back))
        {
            _mu.unlock();
            return false;
        }
        _mu.unlock();

        swap();
        return true;
    }

    bool read(
        Container& value,
        copy_fn copy = [](Container& src, Container& dst)->bool{ dst = src; return true; })
    {
        Container* front = _front.load();
        return copy(*front, value);
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
    std::mutex _mu;
};

}

#endif