#ifndef SHARED_MEMORY_HPP
#define SHARED_MEMORY_HPP

#include <stdexcept>
#include <exception>
#include <string>
#include <string.h>
#include <assert.h>
#include <fcntl.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

namespace hj
{

class shared_memory
{
  public:
#if defined(_WIN32)
    enum flag : int
    {
        create     = 0,
        read_write = PAGE_READWRITE,
    };

    enum access : int
    {
        read  = FILE_MAP_READ,
        write = FILE_MAP_WRITE,
        all   = FILE_MAP_ALL_ACCESS,
    };

    using fd_t                       = HANDLE;
    static constexpr fd_t invalid_fd = nullptr;
#else
    enum flag : int
    {
        create     = O_CREAT,
        read_write = O_RDWR,
    };

    enum access : int
    {
        read  = PROT_READ,
        write = PROT_WRITE,
        all   = PROT_READ | PROT_WRITE
    };

    using fd_t                       = int;
    static constexpr fd_t invalid_fd = -1;
#endif

  public:
    shared_memory(const char       *name,
                  const std::size_t sz  = 0,
                  const int         op  = flag::create | flag::read_write,
                  const int         arg = 0666)
        : _name{name}
        , _sz{sz}
        , _fd{invalid_fd}
    {
#if defined(_WIN32)
        _fd = CreateFileMappingA(INVALID_HANDLE_VALUE,
                                 NULL,
                                 (DWORD) op,
                                 static_cast<DWORD>((sz >> 32) & 0xFFFFFFFF),
                                 static_cast<DWORD>(sz & 0xFFFFFFFF),
                                 name);
#else
        _fd = shm_open(name, op, arg);
        if(_fd != -1)
            ftruncate(_fd, _sz);
#endif

        if(!is_fd_valid())
            throw std::runtime_error("shared memory open failed");
    }
    ~shared_memory()
    {
        unmap();
        if(is_fd_valid())
        {
#if defined(_WIN32)
            CloseHandle(_fd);
            _fd = nullptr;
#else
            close(_fd);
            _fd = -1;
#endif
        }
        _sz  = 0;
        _ptr = nullptr;
    }

    static int remove(const char *name)
    {
#if defined(_WIN32)
        return 0; // auto recycle
#else
        return shm_unlink(name);
#endif
    }

    inline bool is_fd_valid()
    {
#if defined(_WIN32)
        return _fd != nullptr;
#else
        return _fd != -1;
#endif
    }
    inline void       *addr() { return _ptr; }
    inline std::size_t size() { return _sz; }

    void *map(std::size_t offset = 0,
              const int   prot   = access::all,
              void       *addr   = nullptr)
    {
        if(!is_fd_valid())
            return nullptr;

#if defined(_WIN32)
        _ptr = MapViewOfFileEx(_fd,
                               prot,
                               static_cast<DWORD>((offset >> 32) & 0xFFFFFFFF),
                               static_cast<DWORD>(offset & 0xFFFFFFFF),
                               _sz,
                               addr);

        return _ptr;
#else
        _ptr = mmap(addr, size(), prot, MAP_SHARED, _fd, offset);
        if(_ptr == MAP_FAILED)
            return nullptr;

        return _ptr;
#endif
    }

    bool unmap()
    {
        if(_ptr == nullptr)
            return true;

#if defined(_WIN32)
        if(!UnmapViewOfFile(_ptr))
            return false;
#else
        if(munmap(_ptr, size()) < 0)
            return false;
#endif

        _ptr = nullptr;
        return true;
    }

  private:
    const std::string _name = "";
    std::size_t       _sz   = 0;
    fd_t              _fd;
    void             *_ptr = nullptr;
};

}

#endif