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
        auto ptr = _pool.malloc();
        new(ptr) typename boost::object_pool<T>::element_type(std::forward<Args>(args)...);
        push(ptr);
    }

    T* allocate()
    {
        return _pool.malloc();
    }

    typename boost::object_pool<T>::element_type* pop()
    {
        if (_container.empty())
            return nullptr;

        auto ret = _container.front();
        _container.pop();
        return ret;
    }

    inline void push(typename boost::object_pool<T>::element_type* obj)
    {
        _container.push(obj);
    }

    inline std::size_t size()
    {
        return _container.size();
    }

private:
    object_pool(const object_pool&) = delete;
    object_pool& operator=(const object_pool&) = delete;
    object_pool(const object_pool&&) = delete;
    object_pool& operator=(const object_pool&&) = delete;

private:
    boost::object_pool<T>                                     _pool;
    std::queue<typename boost::object_pool<T>::element_type*> _container;
};

}

#endif