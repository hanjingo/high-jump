#ifndef SHARED_MEMORY_HPP
#define SHARED_MEMORY_HPP

#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/named_condition.hpp>

namespace libcpp
{

class shared_memory
{
public:
    using mode_t = boost::interprocess::mode_t;
    using region_t = boost::interprocess::mapped_region;

public:
    shared_memory(const char* name, mode_t mod = mode_t::read_write) 
        : _obj{new boost::interprocess::shared_memory_object(boost::interprocess::open_or_create, name, mod)}
        , _mu{new boost::interprocess::named_mutex(boost::interprocess::open_or_create, name)}
    { 
    }
    ~shared_memory() 
    {
    }

    static bool remove(const char* name)
    {
        boost::interprocess::shared_memory_object::remove(name);
    }

    void truncate(const std::size_t sz)
    {
        _obj->truncate(sz);
    }

    std::size_t size()
    {
        boost::interprocess::offset_t sz;
        if (!_obj->get_size(sz))
            return 0;

        return sz;
    }

    region_t map()
    {
        return boost::interprocess::mapped_region(*_obj, _obj->get_mode());
    }

    void lock()
    {
        _mu->lock();
    }

    void unlock()
    {
        _mu->unlock();
    }

private:
    boost::interprocess::shared_memory_object* _obj;
    boost::interprocess::named_mutex* _mu;
};

}

#endif