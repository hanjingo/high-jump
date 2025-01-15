#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#ifdef BOOST_ENABLE
#include <boost/serialization/singleton.hpp>
namespace libcpp
{

template <typename T>
using singleton = boost::serialization::singleton<T>;

}

#else

#include <thread>
#include <mutex>
#include <assert.h>

namespace libcpp
{

class noncopyable
{
protected:
    noncopyable() {}
    ~noncopyable() {}
private:
    noncopyable(const noncopyable&);
    const noncopyable& operator=(const noncopyable&);
};

template<typename T>
class singleton_guard : std::unique_lock<std::mutex>, public noncopyable
{
public:
    explicit singleton_guard(T* inst, std::mutex& mt): std::unique_lock<std::mutex>(mt), guard_(inst)
    {
    }
    singleton_guard(singleton_guard&& guard) : guard_(guard.guard_)
    {
        guard.guard_ = nullptr;
    }
    T* operator->()const
    {
        return guard_;
    }
private:
    T* guard_;
};

template<typename T>
class singleton_wrapper : public T
{
public:
    static bool b_destroyed_;
    ~singleton_wrapper()
    {
        b_destroyed_ = true;
    }
};

template<typename T>
bool singleton_wrapper< T >::b_destroyed_ = false;

template<typename T>
class singleton_unsafe : public noncopyable
{
public:
    static T& get_instance()
    {
        static singleton_wrapper< T > t;
        //assert(! singleton_wrapper< T >::b_destroyed_);
        use(instance);
        return static_cast<T&>(t);
    }
private:
    static T& instance;
    static void use(T const&) {}

};

template<typename T>
T& singleton_unsafe< T >::instance = singleton_unsafe< T >::get_instance();

template<typename T>
class singleton : public noncopyable
{
public:
    static singleton_guard<T> get_mutable_instance()
    {
        return singleton_guard<T>(&get_instance(), mu_);
    }

    static const T& get_const_instance()
    {
        return get_instance();
    }
private:
    static T& instance;
    static void use(T const&) {}
    static T& get_instance()
    {
        static singleton_wrapper< T > t;
        assert(!singleton_wrapper<T>::b_destroyed_);
        use(instance);
        return static_cast<T&>(t);
    }
    static std::mutex mu_;
};
template<typename T>
std::mutex singleton< T >::mu_;

template<typename T>
T& singleton< T >::instance = singleton< T >::get_instance();

}
#endif

#endif
