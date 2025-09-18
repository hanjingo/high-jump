#ifndef INIT_HPP
#define INIT_HPP

#include <list>
#include <functional>

namespace libcpp
{

class init final
{
public:
    explicit init(std::function<void()>&& cb)
    {
        cb_ = std::move(cb);

        if (cb_) {
            cb_();
        }
    }
    ~init() {}

    init(const init& other) = delete;
    init& operator=(const init&) = delete;

    init(init&& other) = delete;
    init& operator=(init&& other) = delete;

private:
    std::function<void()> cb_;
};

}

#define __init_cat(a, b) a##b
#define _init_cat(a, b) __init_cat(a, b)

#define INIT(cmd) namespace { ::libcpp::init _init_cat(__simulate_go_init__, __COUNTER__)([](){ cmd; }); }

#endif