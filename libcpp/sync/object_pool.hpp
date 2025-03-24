#ifndef OBJECT_POOL_HPP
#define OBJECT_POOL_HPP

#include <vector>

#include <boost/pool/object_pool.hpp>

namespace libcpp
{

template <typename T>
class object_pool
{
public:
    object_pool()
        : _pool{boost::object_pool<T>()}
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
        return ptr;
    }

    inline boost::object_pool<T>::element_type* pop()
    {
        return (_container.empty() ? nullptr : _container.pop_front());
    }

    inline void push(boost::object_pool<T>::element_type* obj)
    {
        _container.push_back();
    }

    inline std::size_t size()
    {
        return _container.size();
    }

private:
    boost::object_pool<T> _pool;
    std::vector<boost::object_pool<T>::element_type*> _container;
};

}

#endif