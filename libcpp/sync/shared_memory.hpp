#ifndef SHARED_MEMORY_HPP
#define SHARED_MEMORY_HPP

#include <assert.h>
#include <fcntl.h>
#include <string.h>

#include <exception>
#include <stdexcept>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

namespace libcpp
{

class shared_memory
{
  public:
#if defined(_WIN32)
    enum flag : int
    {
        create = 0,
        read_write = PAGE_READWRITE,
    };

    enum access : int
    {
        read = FILE_MAP_READ,
        write = FILE_MAP_WRITE,
        all = FILE_MAP_ALL_ACCESS,
    };

    using fd_t = HANDLE;
    static constexpr fd_t invalid_fd = nullptr;
#else
    enum flag : int
    {
        create = O_CREAT,
        read_write = O_RDWR,
    };

    enum access : int
    {
        read = PROT_READ,
        write = PROT_WRITE,
        all = PROT_READ | PROT_WRITE
    };

    using fd_t = int;
    static constexpr fd_t invalid_fd = -1;
#endif

  public:
    shared_memory (const char *name,
                   const std::size_t sz = 0,
                   const int op = flag::create | flag::read_write,
                   const int arg = 0666) :
        name_{name}, sz_{sz}, fd_{invalid_fd}
    {
#if defined(_WIN32)
        fd_ = CreateFileMappingA (INVALID_HANDLE_VALUE, NULL, (DWORD) op,
                                  static_cast<DWORD> ((sz >> 32) & 0xFFFFFFFF),
                                  static_cast<DWORD> (sz & 0xFFFFFFFF), name);
#else
        fd_ = shm_open (name, op, arg);
        if (fd_ != -1)
            ftruncate (fd_, sz_);
#endif

        if (!is_fd_valid ())
            throw std::runtime_error ("shared memory open failed");
    }
    ~shared_memory ()
    {
        unmap ();
        if (is_fd_valid ()) {
#if defined(_WIN32)
            CloseHandle (fd_);
            fd_ = nullptr;
#else
            close (fd_);
            fd_ = -1;
#endif
        }
        sz_ = 0;
        ptr_ = nullptr;
    }

    static int remove (const char *name)
    {
#if defined(_WIN32)
        return 0; // auto recycle
#else
        return shm_unlink (name);
#endif
    }

    inline bool is_fd_valid ()
    {
#if defined(_WIN32)
        return fd_ != nullptr;
#else
        return fd_ != -1;
#endif
    }
    inline void *addr () { return ptr_; }
    inline std::size_t size () { return sz_; }

    void *map (std::size_t offset = 0,
               const int prot = access::all,
               void *addr = nullptr)
    {
        if (!is_fd_valid ())
            return nullptr;

#if defined(_WIN32)
        ptr_ = MapViewOfFileEx (
          fd_, prot, static_cast<DWORD> ((offset >> 32) & 0xFFFFFFFF),
          static_cast<DWORD> (offset & 0xFFFFFFFF), sz_, addr);

        return ptr_;
#else
        ptr_ = mmap (addr, size (), prot, MAP_SHARED, fd_, offset);
        if (ptr_ == MAP_FAILED)
            return nullptr;

        return ptr_;
#endif
    }

    bool unmap ()
    {
        if (ptr_ == nullptr)
            return true;

#if defined(_WIN32)
        if (!UnmapViewOfFile (ptr_))
            return false;
#else
        if (munmap (ptr_, size ()) < 0)
            return false;
#endif

        ptr_ = nullptr;
        return true;
    }

  private:
    const std::string name_ = "";
    std::size_t sz_ = 0;
    fd_t fd_;
    void *ptr_ = nullptr;
};

} // namespace libcpp

#endif