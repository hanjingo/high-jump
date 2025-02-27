#ifndef HANDLER_MAP_HPP
#define HANDLER_MAP_HPP

#include <string>
#include <assert.h>
#include <functional>
#include <unordered_map>

// See Also:
//  1. https://blog.csdn.net/wangzhicheng1983/article/details/103498172
//  2. https://www.soinside.com/question/sJGEZPqmnTeBEeqwEhKF2N
//  3. https://blog.csdn.net/yyx112358/article/details/78515420

namespace libcpp
{

template<typename Key>
class multiplexer
{
    using any_t = void*;

public:
    multiplexer() {}
    ~multiplexer() {}

    template <typename F>
    void reg(const Key& key, F&& fn)
    {
        if (m_handler_.find(key) != m_handler_.end()) 
            throw "key already exist, please remove it before regist";

        m_handler_[key] = (any_t)(std::move(fn));
    }

    template<typename Ret = void, typename... Types>
    Ret on(const Key& key, Types&& ... args)
    {
        auto itr = m_handler_.find(key);
        if (itr == m_handler_.end()) 
            throw "key not found";

        try {
            auto fn = (Ret(*)(Types...))(itr->second);
            if (!std::is_same<Ret, void>())
                return fn(std::forward<Types>(args)...);

            fn(std::forward<Types>(args)...);
        } catch (std::exception e) {
            on_exception(e);
        }
    }

    virtual void on_exception(std::exception& e)
    {
        assert(e.what());
    }

    static libcpp::multiplexer<Key>* instance()
    {
        static libcpp::multiplexer<Key> inst_;
        return &inst_;
    }

private:
    std::unordered_map<Key, any_t> m_handler_;
};

class init_ final
{
public:
    explicit init_(std::function<void()>&& cb)
    {
        cb_ = std::move(cb);

        if (cb_) {
            cb_();
        }
    }
    ~init_() {}

    init_(const init_& other) = delete;
    init_& operator=(const init_&) = delete;

    init_(init_&& other) = delete;
    init_& operator=(init_&& other) = delete;

private:
    std::function<void()> cb_;
};

}

#define __mux_cat(a, b) a##b
#define _mux_cat(a, b) __mux_cat(a, b)

#define MUX(key, cmd, ...) \
    libcpp::init_ _mux_cat(__mux_init__, __COUNTER__)( \
        [](){ libcpp::multiplexer<decltype(key)>::instance()->reg<void(*)(__VA_ARGS__)>(key, [](__VA_ARGS__){ cmd }); } \
    ); \

#define ON(key, ...) \
    libcpp::multiplexer<decltype(key)>::instance()->on<void>(key, ##__VA_ARGS__); \

#endif
