#ifndef SHARED_MEMORY_HPP
#define SHARED_MEMORY_HPP

#include <exception>
#include <string>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

#if defined(_WIN32)

// TODO

#elif __linux__
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace libcpp
{

class shared_memory
{
public:
    // param description
    // flag:
    //   + O_CREAT
    //   + O_RDWR
    //   + O_EXCL
    shared_memory(const char* name, const std::size_t sz = 0, const int flag = O_CREAT | O_RDWR, const int mod = 0666)
        : _name{name}, _sz{sz}, _fd{shm_open(name, flag, mod)}
    {
        truncate(sz);

        if (_fd < 0)
            throw std::runtime_error("shm_open failed");
    }
    ~shared_memory() 
    {
        unmap();
        if (_fd >= 0)
            close(_fd);
        _fd = -1;
        _sz = 0;
        _ptr = nullptr;
    }

    static int remove(const char* name)
    {
        return shm_unlink(name);
    }

    inline void* addr() { return _ptr; }
    inline std::size_t size() { return _sz; }

    void truncate(const std::size_t sz)
    {
        if (_fd > -1)
            ftruncate(_fd, sz);

        _sz = sz;
    }

    // param description
    // offset: suggest n*4096
    // prot: 
    //   + PROT_EXEC
    //   + PROT_READ
    //   + PROT_WRITE
    //   + PROT_NONE
    // flags:
    //   + MAP_FIXED
    //   + MAP_SHARED
    //   + MAP_PRIVATE
    //   + MAP_DENYWRITE 
    //   + MAP_EXECUTABLE 
    //   + MAP_NORESERVE 
    //   + MAP_LOCKED 
    //   + MAP_GROWSDOWN 
    //   + MAP_ANONYMOUS 
    //   + MAP_ANON (not used)
    //   + MAP_FILE 
    //   + MAP_32BIT 
    //   + MAP_POPULATE 
    //   + MAP_NONBLOCK 
    void* map(std::size_t offset = 0, 
              const int prot = PROT_READ | PROT_WRITE, 
              const int flag = MAP_SHARED, 
              void* addr = nullptr)
    {
        _ptr = mmap(addr, size(), prot, flag, _fd, offset);
        if (_ptr == MAP_FAILED)
            return nullptr;

        return _ptr;
    }

    bool unmap()
    {
        if (_ptr != nullptr)
            if (munmap(_ptr, size()) < 0)
                return false;

        _ptr = nullptr;
        return true;
    }

private:
    const std::string _name = "";
    std::size_t       _sz = 0;
    int               _fd = -1;
    void*             _ptr = nullptr;
};

}

#else
#pragma warning unknown OS, some function will be disabled
#endif

#endif