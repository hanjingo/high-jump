#ifndef DEFER_HPP
#define DEFER_HPP

#include <functional>

namespace hj
{

class defer final
{
  public:
    defer() {}
    ~defer()
    {
        if(_cb)
        {
            _cb();
        }
    }

    defer(const defer &other)       = delete;
    defer &operator=(const defer &) = delete;

    defer(defer &&other)            = delete;
    defer &operator=(defer &&other) = delete;

    explicit defer(std::function<void()> &&cb) { _cb = std::move(cb); }

  private:
    std::function<void()> _cb;
};

}

#define __defer_cat(a, b) a##b
#define _defer_cat(a, b) __defer_cat(a, b)

#define DEFER(cmd)                                                             \
    ::hj::defer _defer_cat(__simulate_go_defer__, __COUNTER__)([&]() { cmd; });
#define DEFER_CLASS(cmd)                                                       \
    ::hj:: ::defer _defer_cat(__simulate_go_defer_class__,                     \
                              __COUNTER__)([&, this]() { cmd; });

#endif // DEFER_HPP
