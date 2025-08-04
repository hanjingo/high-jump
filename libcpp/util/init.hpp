#ifndef INIT_HPP
#define INIT_HPP

#include <functional>
#include <list>

namespace libcpp
{

class init final
{
  public:
    explicit init (std::function<void ()> &&cb)
    {
        cb_ = std::move (cb);

        if (cb_) {
            cb_ ();
        }
    }
    ~init () {}

    init (const init &other) = delete;
    init &operator= (const init &) = delete;

    init (init &&other) = delete;
    init &operator= (init &&other) = delete;

  private:
    std::function<void ()> cb_;
};

} // namespace libcpp

#define __init_cat(a, b) a##b
#define _init_cat(a, b) __init_cat (a, b)

#define INIT(cmd)                                                              \
    ::libcpp::init _init_cat (__simulate_go_init__,                            \
                              __COUNTER__) ([] () { cmd; });


#endif