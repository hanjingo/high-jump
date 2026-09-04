/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef DLL_H
#define DLL_H

#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <wchar.h>
#else
#include <dlfcn.h>
#endif

#if defined(_WIN32)
#define DLL_EXT ".dll"
#define DLL_PREFIX ""
#elif defined(__APPLE__)
#define DLL_EXT ".dylib"
#define DLL_PREFIX "lib"
#elif defined(__linux__) || defined(__ANDROID__)
#define DLL_EXT ".so"
#define DLL_PREFIX "lib"
#else
#define DLL_EXT ""
#define DLL_PREFIX ""
#pragma message("WARNING: UNKNOWN DYNAMIC LINK LIBRARY FILE EXTENSION.")
#endif

#if defined(_MSC_VER)
#define DLL_EXPORT __declspec(dllexport)
#define DLL_IMPORT __declspec(dllimport)
#elif defined(__GNUC__) || defined(__clang__)
#define DLL_EXPORT __attribute__((visibility("default")))
#define DLL_IMPORT
#else
#define DLL_EXPORT
#define DLL_IMPORT
#endif

#ifdef __cplusplus
#define C_STYLE_EXPORT extern "C" DLL_EXPORT
#define C_STYLE_IMPORT extern "C" DLL_IMPORT
#else
#define C_STYLE_EXPORT DLL_EXPORT
#define C_STYLE_IMPORT DLL_IMPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef int dll_mode_t;

/*
 * ====================================================================================================
 *                                   FLAG COMPATIBILITY MATRIX
 * ====================================================================================================
 *  Flag Flag                       | Windows (PE)                   | POSIX (ELF / Mach-O)
 * ---------------------------------+--------------------------------+---------------------------------
 *  DLL_MODE_DEFAULT                | Default LoadLibraryExW (CWD)   | RTLD_LAZY | RTLD_LOCAL
 *  DLL_MODE_RTLD_NOW               | Ignored (Windows resolves all) | RTLD_NOW
 *  DLL_MODE_RTLD_GLOBAL            | Ignored (No ELF symbol space)  | RTLD_GLOBAL
 *  DLL_MODE_RTLD_LOCAL             | Ignored (Default PE behavior)  | RTLD_LOCAL
 *  DLL_MODE_RTLD_NOLOAD            | Emulated via GetModuleHandleExW| RTLD_NOLOAD
 *  DLL_MODE_RTLD_DEEPBIND          | Ignored                        | RTLD_DEEPBIND (Glibc/Linux only)
 * ---------------------------------+--------------------------------+---------------------------------
 *  DLL_MODE_SEARCH_ALTERED_PATH    | LOAD_WITH_ALTERED_SEARCH_PATH  | Ignored
 *  DLL_MODE_SEARCH_SYSTEM32        | LOAD_LIBRARY_SEARCH_SYSTEM32   | Ignored
 *  DLL_MODE_SEARCH_APP_DIR         | LOAD_LIBRARY_SEARCH_APP_DIR    | Ignored
 *  DLL_MODE_SEARCH_USER_DIRS       | LOAD_LIBRARY_SEARCH_USER_DIRS  | Ignored
 *  DLL_MODE_SEARCH_DLL_LOAD_DIR    | LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR| Ignored
 *  DLL_MODE_SAFE                   | Secure default flags combined  | Ignored (POSIX uses RPATH/RUNPATH)
 * ====================================================================================================
 */

// Portable Flags (Supported or safely ignored across platforms)
#define DLL_MODE_DEFAULT 0
#define DLL_MODE_RTLD_NOW (1 << 0)    // POSIX: RTLD_NOW      | Windows: Ignored
#define DLL_MODE_RTLD_GLOBAL (1 << 1) // POSIX: RTLD_GLOBAL   | Windows: Ignored
#define DLL_MODE_RTLD_LOCAL (1 << 2)  // POSIX: RTLD_LOCAL    | Windows: Ignored
#define DLL_MODE_RTLD_NOLOAD                                                   \
    (1 << 3) // POSIX: RTLD_NOLOAD   | Windows: Emulated (GetModuleHandleExW)

// POSIX-Specific Flags
#define DLL_MODE_RTLD_DEEPBIND (1 << 4) // Linux/Glibc Only: RTLD_DEEPBIND

// Windows-Specific Search Path Security Flags
#define DLL_MODE_SEARCH_ALTERED_PATH                                           \
    (1 << 8) // Windows Only: LOAD_WITH_ALTERED_SEARCH_PATH
#define DLL_MODE_SEARCH_SYSTEM32                                               \
    (1 << 9) // Windows Only: LOAD_LIBRARY_SEARCH_SYSTEM32
#define DLL_MODE_SEARCH_APP_DIR                                                \
    (1 << 10) // Windows Only: LOAD_LIBRARY_SEARCH_APPLICATION_DIR
#define DLL_MODE_SEARCH_USER_DIRS                                              \
    (1 << 11) // Windows Only: LOAD_LIBRARY_SEARCH_USER_DIRS
#define DLL_MODE_SEARCH_DLL_LOAD_DIR                                           \
    (1 << 12) // Windows Only: LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR

// Windows-Specific Compound Secure Default
#define DLL_MODE_SAFE                                                          \
    (DLL_MODE_SEARCH_SYSTEM32 | DLL_MODE_SEARCH_APP_DIR                        \
     | DLL_MODE_SEARCH_USER_DIRS)

// All flags combined for validation purposes
#define _DLL_MODE_ALL_MASK                                                     \
    (DLL_MODE_RTLD_NOW | DLL_MODE_RTLD_GLOBAL | DLL_MODE_RTLD_LOCAL            \
     | DLL_MODE_RTLD_NOLOAD | DLL_MODE_RTLD_DEEPBIND                           \
     | DLL_MODE_SEARCH_ALTERED_PATH | DLL_MODE_SEARCH_SYSTEM32                 \
     | DLL_MODE_SEARCH_APP_DIR | DLL_MODE_SEARCH_USER_DIRS                     \
     | DLL_MODE_SEARCH_DLL_LOAD_DIR)

// thread safe error buffer for dll operations
static inline char *_dll_get_err_buf(void)
{
#if defined(_MSC_VER)
    static __declspec(thread) char err_buf[512];
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    static _Thread_local char err_buf[512];
#elif defined(__GNUC__) || defined(__clang__)
    static __thread char err_buf[512];
#else
#error                                                                         \
    "Thread-local storage (TLS) support is required for dll_h in multi-threaded environments."
#endif
    return err_buf;
}

static inline void _dll_set_err_buf(const char *msg)
{
    char *buf = _dll_get_err_buf();
    if(msg)
    {
#if defined(_MSC_VER)
        strncpy_s(buf, 512, msg, _TRUNCATE);
#else
        strncpy(buf, msg, 511);
        buf[511] = '\0';
#endif
    } else
    {
        buf[0] = '\0';
    }
}

// ---------------------------------------------------------------------------
// Common API for Dynamic Link Library (DLL) Operations
// ---------------------------------------------------------------------------
static inline void dll_clear_error(void)
{
    char *buf = _dll_get_err_buf();
    buf[0]    = '\0';

#if defined(_WIN32)
    SetLastError(0);
#else
    dlerror();
#endif
}

/**
 * @brief Retrieves and consumes the error message for the last DLL operation.
 */
static inline const char *dll_pop_error(void)
{
    char *buf = _dll_get_err_buf();
    if(buf[0] != '\0')
    {
        static char temp_buf[512];
#if defined(_MSC_VER)
        strncpy_s(temp_buf, 512, buf, _TRUNCATE);
#else
        strncpy(temp_buf, buf, 511);
        temp_buf[511] = '\0';
#endif
        buf[0] = '\0';
        return temp_buf;
    }

#if defined(_WIN32)
    DWORD err_code = GetLastError();
    if(err_code == 0)
        return "";

    DWORD len = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM
                                   | FORMAT_MESSAGE_IGNORE_INSERTS,
                               NULL,
                               err_code,
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                               buf,
                               512,
                               NULL);

    SetLastError(0);
    if(len == 0)
        return "Unknown system error";

    while(len > 0 && (buf[len - 1] == '\r' || buf[len - 1] == '\n'))
        buf[--len] = '\0';

    return buf;

#else
    const char *err = dlerror();
    return err ? err : "";

#endif
}

static inline void *dll_open(const char *filename, dll_mode_t mode)
{
    dll_clear_error();

    if(!filename || filename[0] == '\0')
    {
#if defined(_WIN32)
        SetLastError(ERROR_INVALID_PARAMETER);
#endif
        _dll_set_err_buf("Invalid argument: filename is NULL or empty");
        return NULL;
    }

    if((mode & ~_DLL_MODE_ALL_MASK) != 0)
    {
#if defined(_WIN32)
        SetLastError(ERROR_INVALID_PARAMETER);
#endif
        _dll_set_err_buf("Invalid argument: unrecognized dll_mode_t flags");
        return NULL;
    }

#if defined(_WIN32)
    int wlen = MultiByteToWideChar(CP_UTF8,
                                   MB_ERR_INVALID_CHARS,
                                   filename,
                                   -1,
                                   NULL,
                                   0);
    if(wlen <= 0)
    {
        SetLastError(ERROR_NO_UNICODE_TRANSLATION);
        _dll_set_err_buf("Invalid UTF-8 sequence in filename");
        return NULL;
    }

    wchar_t  stack_buf[256];
    wchar_t *wpath = stack_buf;
    if((size_t) wlen > sizeof(stack_buf) / sizeof(wchar_t))
    {
        wpath = (wchar_t *) malloc((size_t) wlen * sizeof(wchar_t));
        if(!wpath)
        {
            SetLastError(ERROR_OUTOFMEMORY);
            _dll_set_err_buf("Out of memory during path conversion");
            return NULL;
        }
    }

    if(MultiByteToWideChar(CP_UTF8, 0, filename, -1, wpath, wlen) == 0)
    {
        if(wpath != stack_buf)
            free(wpath);

        _dll_set_err_buf("Failed to convert filename to UTF-16");
        return NULL;
    }

    DWORD    full_len  = GetFullPathNameW(wpath, 0, NULL, NULL);
    wchar_t *full_path = NULL;
    if(full_len > 0)
    {
        full_path = (wchar_t *) malloc(full_len * sizeof(wchar_t));
        if(full_path)
        {
            if(GetFullPathNameW(wpath, full_len, full_path, NULL) == 0)
            {
                free(full_path);
                full_path = NULL;
            }
        }
    }

    wchar_t *final_path    = full_path ? full_path : wpath;
    wchar_t *long_path_buf = NULL;

    if(final_path && wcsncmp(final_path, L"\\\\?\\", 4) != 0)
    {
        size_t path_len = wcslen(final_path);
        if(path_len >= 248)
        {
            if(wcsncmp(final_path, L"\\\\", 2) == 0)
            {
                size_t alloc_len = path_len + 8;
                long_path_buf = (wchar_t *) malloc(alloc_len * sizeof(wchar_t));
                if(long_path_buf)
                {
                    swprintf(long_path_buf,
                             alloc_len,
                             L"\\\\?\\UNC\\%s",
                             final_path + 2);
                    final_path = long_path_buf;
                }
            } else
            {
                size_t alloc_len = path_len + 5;
                long_path_buf = (wchar_t *) malloc(alloc_len * sizeof(wchar_t));
                if(long_path_buf)
                {
                    swprintf(long_path_buf,
                             alloc_len,
                             L"\\\\?\\%s",
                             final_path);
                    final_path = long_path_buf;
                }
            }
        }
    }

    if(mode & DLL_MODE_RTLD_NOLOAD)
    {
        HMODULE hmod = NULL;
        BOOL    ok   = FALSE;
        int     has_path_separator =
            (strchr(filename, '/') != NULL || strchr(filename, '\\') != NULL);

        if(has_path_separator)
            ok = GetModuleHandleExW(0, final_path, &hmod);
        else
            ok = GetModuleHandleExW(0, wpath, &hmod);

        if(long_path_buf)
            free(long_path_buf);
        if(full_path)
            free(full_path);
        if(wpath != stack_buf)
            free(wpath);

        if(!ok)
        {
            SetLastError(ERROR_MOD_NOT_FOUND);
            _dll_set_err_buf("Module not currently loaded (RTLD_NOLOAD)");
            return NULL;
        }

        return (void *) hmod;
    }

    DWORD dwFlags = 0;
    if(mode & DLL_MODE_SEARCH_ALTERED_PATH)
        dwFlags |= LOAD_WITH_ALTERED_SEARCH_PATH;
    if(mode & DLL_MODE_SEARCH_SYSTEM32)
        dwFlags |= LOAD_LIBRARY_SEARCH_SYSTEM32;
    if(mode & DLL_MODE_SEARCH_APP_DIR)
        dwFlags |= LOAD_LIBRARY_SEARCH_APPLICATION_DIR;
    if(mode & DLL_MODE_SEARCH_USER_DIRS)
        dwFlags |= LOAD_LIBRARY_SEARCH_USER_DIRS;
    if(mode & DLL_MODE_SEARCH_DLL_LOAD_DIR)
        dwFlags |= LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR;

    HMODULE hmod = LoadLibraryExW(final_path, NULL, dwFlags);

    if(long_path_buf)
        free(long_path_buf);
    if(full_path)
        free(full_path);
    if(wpath != stack_buf)
        free(wpath);

    return (void *) hmod;

#else
    int posix_flags = 0;
    if(mode & DLL_MODE_RTLD_NOW)
        posix_flags |= RTLD_NOW;
    else
        posix_flags |= RTLD_LAZY;

    if(mode & DLL_MODE_RTLD_GLOBAL)
        posix_flags |= RTLD_GLOBAL;
    if(mode & DLL_MODE_RTLD_LOCAL)
        posix_flags |= RTLD_LOCAL;
    if(mode & DLL_MODE_RTLD_NOLOAD)
        posix_flags |= RTLD_NOLOAD;
#if defined(RTLD_DEEPBIND)
    if(mode & DLL_MODE_RTLD_DEEPBIND)
        posix_flags |= RTLD_DEEPBIND;
#endif

    void *handle = dlopen(filename, posix_flags);
    if(!handle && (mode & DLL_MODE_RTLD_NOLOAD))
    {
        const char *err = dlerror();
        if(!err || err[0] == '\0')
            _dll_set_err_buf("Module not currently loaded (RTLD_NOLOAD)");
    }
    return handle;
#endif
}

static inline void *dll_get(void *handler, const char *symbol)
{
    dll_clear_error();

    if(!handler || !symbol || symbol[0] == '\0')
    {
#if defined(_WIN32)
        SetLastError(ERROR_INVALID_PARAMETER);
#endif
        _dll_set_err_buf("Invalid argument: handle or symbol is NULL/empty");
        return NULL;
    }

#if defined(_WIN32)
    void *sym = (void *) GetProcAddress((HMODULE) handler, symbol);
    if(!sym)
    {
        if(GetLastError() == 0)
            SetLastError(ERROR_PROC_NOT_FOUND);
    }
    return sym;
#else
    void       *sym = dlsym(handler, symbol);
    const char *err = dlerror();
    if(err != NULL)
        return NULL;

    return sym;
#endif
}

/**
 * @brief Closes and unloads the dynamic link library module.
 * 
 * @param handle Dynamic module handle returned by dll_open().
 * @return 0 on success, or -1 on failure (error details via dll_pop_error()).
 * 
 * @note CONTRACT & LIFETIME WARNING:
 *       1. After dll_close(handle) is called, the handle becomes INVALID immediately.
 *       2. Passing a closed handle to dll_get() or calling dll_close() twice on the same handle
 *          is UNDEFINED BEHAVIOR (Use-After-Close / Double-Free).
 *       3. Function pointers fetched via dll_get() from this handle also become EXPIRED
 *          and calling them after close will result in Segmentation Faults / Access Violations.
 */
static inline int dll_close(void *handle)
{
    dll_clear_error();
    if(!handle)
    {
#if defined(_WIN32)
        SetLastError(ERROR_INVALID_PARAMETER);
#endif
        _dll_set_err_buf("Invalid argument: handle is null");
        return -1;
    }

#if defined(_WIN32)
    BOOL ret = FreeLibrary((HMODULE) handle);
    return ret ? 0 : -1;
#else
    int ret = dlclose(handle);
    return ret;
#endif
}

#ifdef __cplusplus
}
#endif


// ---------------------------------------------------------------------------
// C++ Wrappers for Dynamic Link Library (DLL) Operations
// ---------------------------------------------------------------------------
#ifdef __cplusplus

#include <string>
#include <stdexcept>

namespace hj
{

class dll_loader
{
  public:
    dll_loader() noexcept
        : _handle(nullptr)
    {
    }

    explicit dll_loader(const char *path, dll_mode_t mode = DLL_MODE_DEFAULT)
        : _handle(dll_open(path, mode))
    {
        if(!_handle)
            throw std::logic_error("DLL handle is loaded fail");
    }
    ~dll_loader() { close(); }

    dll_loader(const dll_loader &)            = delete;
    dll_loader &operator=(const dll_loader &) = delete;

    dll_loader(dll_loader &&other) noexcept
        : _handle(other._handle)
    {
        other._handle = nullptr;
    }

    dll_loader &operator=(dll_loader &&other) noexcept
    {
        if(this != &other)
        {
            close();
            _handle       = other._handle;
            other._handle = nullptr;
        }
        return *this;
    }

    bool open(const char *filename, dll_mode_t mode = DLL_MODE_DEFAULT)
    {
        close();
        _handle = dll_open(filename, mode);
        return is_loaded();
    }

    void close()
    {
        if(_handle)
        {
            dll_close(_handle);
            _handle = nullptr;
        }
    }

    bool is_loaded() const { return _handle != nullptr; }

    template <class T>
    T symbol(const char *name) const noexcept
    {
        return reinterpret_cast<T>(dll_get(_handle, name));
    }

    const char *pop_error() const { return dll_pop_error(); }

    inline void *get() const noexcept { return _handle; }

  private:
    void *_handle;
};

} // namespace hj

#endif

#endif // DLL_H