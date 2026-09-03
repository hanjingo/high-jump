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

#include <stdint.h>

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
#define C_STYLE_IMPORT DLL_IMPORT
#else
#define C_STYLE_EXPORT DLL_EXPORT
#define C_STYLE_IMPORT DLL_IMPORT
#endif

typedef int dll_mode_t;

static const dll_mode_t DLL_MODE_DEFAULT       = 0;
static const dll_mode_t DLL_MODE_RTLD_NOW      = 1 << 0;
static const dll_mode_t DLL_MODE_RTLD_GLOBAL   = 1 << 1;
static const dll_mode_t DLL_MODE_RTLD_LOCAL    = 1 << 2;
static const dll_mode_t DLL_MODE_RTLD_NOLOAD   = 1 << 3;
static const dll_mode_t DLL_MODE_RTLD_DEEPBIND = 1 << 4;

static const dll_mode_t DLL_MODE_SEARCH_ALTERED_PATH =
    1 << 8; // Windows Only: LOAD_WITH_ALTERED_SEARCH_PATH
static const dll_mode_t DLL_MODE_SEARCH_SYSTEM32 =
    1 << 9; // Windows Only: LOAD_LIBRARY_SEARCH_SYSTEM32

// define sdk common api
typedef struct sdk_context
{
    uint64_t sz;
    void    *user_data;
    void (*cb)(void *);
} sdk_context;

typedef void (*sdk_callback)(void *);
typedef void (*sdk_api)(sdk_context *);

// dll operations
inline const char *dll_error()
{
#if defined(_WIN32)
    static thread_local char err_buf[256];
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL,
                   GetLastError(),
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   err_buf,
                   sizeof(err_buf),
                   NULL);
    return err_buf;
#else
    return dlerror();
#endif
}

inline void dll_clear_error()
{
#if defined(_WIN32)
    SetLastError(0);
#else
    dlerror();
#endif
}

inline void *dll_open(const char *filename, dll_mode_t mode)
{
    dll_clear_error();

#if defined(_WIN32)
    if(!filename)
    {
        SetLastError(
            static_cast<DWORD>(strlen("Invalid argument: filename is null")));
        return NULL;
    }

    wchar_t  buf[256];
    wchar_t *wpath = buf;
    MultiByteToWideChar(CP_UTF8, 0, filename, -1, wpath, 256);
    if(mode & DLL_MODE_RTLD_NOLOAD)
    {
        HMODULE hmod = NULL;
        if(GetModuleHandleExW(0, wpath, &hmod))
            return (void *) (hmod);

        return NULL;
    }

    DWORD dwFlags = 0;
    if(mode & DLL_MODE_SEARCH_ALTERED_PATH)
        dwFlags |= LOAD_WITH_ALTERED_SEARCH_PATH;
    if(mode & DLL_MODE_SEARCH_SYSTEM32)
        dwFlags |= LOAD_LIBRARY_SEARCH_SYSTEM32;

    return (void *) (LoadLibraryExW(wpath, NULL, dwFlags));

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

    return dlopen(filename, posix_flags);
#endif
}

inline void *dll_get(void *handler, const char *symbol)
{
    dll_clear_error();

#if defined(_WIN32)
    if(!handler || !symbol)
    {
        SetLastError(
            (DWORD) (strlen("Invalid argument: handle or symbol is null")));
        return NULL;
    }

    void *sym = (void *) (GetProcAddress((HMODULE) (handler), symbol));
    if(!sym)
    {
        SetLastError((DWORD) (strlen("Failed to get symbol")));
        return NULL;
    }

    return sym;
#else
    dlerror();
    void *sym = dlsym(handler, symbol);
    return sym;
#endif
}

inline bool dll_close(void *handle)
{
    dll_clear_error();

#if defined(_WIN32)
    if(!handle)
    {
        SetLastError((DWORD) (strlen("Invalid argument: handle is null")));
        return false;
    }

    BOOL ret = FreeLibrary((HMODULE) (handle));
    return ret != 0;
#else
    int ret = dlclose(handle);
    return ret == 0;
#endif
}

#endif
