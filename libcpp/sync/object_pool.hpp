#ifndef OBJECT_POOL_HPP
#define OBJECT_POOL_HPP

#include <queue>

#include <boost/pool/object_pool.hpp>

namespace libcpp
{

template <typename T>
class object_pool
{
public:
    object_pool()
    {
    }
    ~object_pool() 
    {
    }

    template<typename ... Args>
    void construct(Args&& ... args)
    {
        auto ptr = pool_.malloc();
        new(ptr) typename boost::object_pool<T>::element_type(std::forward<Args>(args)...);
        push(ptr);
    }

    typename boost::object_pool<T>::element_type* pop()
    {
        if (container_.empty())
            return nullptr;

        auto ret = container_.front();
        container_.pop();
        return ret;
    }

    inline void push(typename boost::object_pool<T>::element_type* obj)
    {
        container_.push(obj);
    }

    inline std::size_t size()
    {
        return container_.size();
    }

private:
    object_pool(const object_pool&) = delete;
    object_pool& operator=(const object_pool&) = delete;
    object_pool(const object_pool&&) = delete;
    object_pool& operator=(const object_pool&&) = delete;

private:
    boost::object_pool<T>                                     pool_;
    std::queue<typename boost::object_pool<T>::element_type*> container_;
};

}

#endif