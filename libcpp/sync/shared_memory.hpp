#ifndef SHARED_MEMORY_HPP
#define SHARED_MEMORY_HPP

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>

namespace libcpp
{

class shared_memory
{
public:
    shared_memory(const char* name, bool remove_if_exist = false)
    {
        if (remove_if_exist)
            boost::interprocess::message_queue::remove(name);

        _obj = boost::interprocess::shared_memory_object(
            boost::interprocess::open_or_create, name, boost::interprocess::read_write);
        _region = boost::interprocess::mapped_region(_obj, boost::interprocess::read_write);
    }
    ~shared_memory() {}

    inline void truncate(const std::size_t sz)
    {
        _obj.truncate(sz);
    }

    inline std::size_t size()
    {
        boost::interprocess::offset_t sz;
        _obj.get_size(sz);
        return sz;
    }

    inline std::string name()
    {
        return _obj.get_name();
    }

    inline int* addr()
    {
        return static_cast<int*>(_region.get_address());
    }

    inline bool remove()
    {
        return boost::interprocess::shared_memory_object::remove(name());
    }

private:
    boost::interprocess:shared_memory_object _obj;
    boost::interprocess::mapped_region       _region;
};

}

#endif