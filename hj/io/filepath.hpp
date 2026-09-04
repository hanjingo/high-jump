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

#ifndef FILEPATH_HPP
#define FILEPATH_HPP

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <optional>
#include <regex>
#include <string>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utime.h>
#endif

namespace hj
{
namespace filepath
{

namespace fs = std::filesystem;

enum class match_target
{
    filename,
    path
};

struct walk_options
{
    bool        recursive              = false;
    std::size_t max_depth              = 0;
    bool        skip_permission_denied = true;
    bool        continue_on_error      = true;

    walk_options(bool        rec,
                 std::size_t depth     = 0,
                 bool        skip_perm = true,
                 bool        cont_err  = true)
        : recursive(rec)
        , max_depth(depth)
        , skip_permission_denied(skip_perm)
        , continue_on_error(cont_err)
    {
    }
    walk_options() = default;

    void reset()
    {
        recursive              = false;
        max_depth              = 0;
        skip_permission_denied = true;
        continue_on_error      = true;
    }
};

struct mkdir_options
{
    bool recursive     = true;
    bool succ_if_exist = false;

    mkdir_options(bool rec, bool ok)
        : recursive(rec)
        , succ_if_exist(ok)
    {
    }
    mkdir_options() = default;

    void reset()
    {
        recursive     = true;
        succ_if_exist = false;
    }
};

struct find_options
{
    bool         recursive              = false;
    std::size_t  max_depth              = 0;
    std::size_t  max_result             = 0;
    match_target target                 = match_target::filename;
    bool         skip_permission_denied = true;
    bool         continue_on_error      = true;

    find_options(bool         rec,
                 std::size_t  depth,
                 std::size_t  max_res,
                 match_target t,
                 bool         skip_perm = true,
                 bool         cont_err  = true)
        : recursive(rec)
        , max_depth(depth)
        , max_result(max_res)
        , target(t)
        , skip_permission_denied(skip_perm)
        , continue_on_error(cont_err)
    {
    }
    find_options() = default;

    void reset()
    {
        recursive              = false;
        max_depth              = 0;
        max_result             = 0;
        target                 = match_target::filename;
        skip_permission_denied = true;
        continue_on_error      = true;
    }
};

struct list_options
{
    bool skip_permission_denied = true;
    bool continue_on_error      = true;

    list_options(bool skip_perm = true, bool cont_err = true)
        : skip_permission_denied(skip_perm)
        , continue_on_error(cont_err)
    {
    }

    void reset()
    {
        skip_permission_denied = true;
        continue_on_error      = true;
    }
};

inline bool default_walk_error_handler(const std::string &,
                                       const std::error_code &)
{
    return true;
}

namespace detail
{
template <typename T>
inline void join_helper(fs::path &p, T &&arg)
{
    p /= std::forward<T>(arg);
}

template <typename T, typename... Args>
inline void join_helper(fs::path &p, T &&first, Args &&...args)
{
    p /= std::forward<T>(first);
    join_helper(p, std::forward<Args>(args)...);
}

template <typename Iterator, typename Callback, typename ErrorHandler>
inline void walk_impl(const std::string &path,
                      Callback         &&callback,
                      walk_options       options,
                      std::error_code   &ec,
                      ErrorHandler     &&err_handler)
{
    ec.clear();
    auto     fs_options = options.skip_permission_denied
                              ? fs::directory_options::skip_permission_denied
                              : fs::directory_options::none;
    Iterator it(path, fs_options, ec);
    Iterator end;
    if(ec)
    {
        const bool handled = err_handler(path, ec);

        // Once the error handler has been invoked, the handler owns the
        // decision to stop/continue. A handled termination is not a
        // pending error and therefore must not escape through the
        // non-error-code overload as an exception.
        if(!handled || !options.continue_on_error)
        {
            ec.clear();
            return;
        }

        ec.clear();
    }

    while(it != end)
    {
        if(ec)
        {
            std::string current_path = path;
            try
            {
                current_path = it->path().string();
            }
            catch(const fs::filesystem_error &)
            {
                ec = std::make_error_code(std::errc::io_error);
                break;
            }

            const bool handled = err_handler(current_path, ec);
            if(!handled || !options.continue_on_error)
            {
                // The error has been delivered to the handler. Clear it so
                // that termination caused by the handler is not re-thrown
                // by the throwing walk() overload.
                ec.clear();
                break;
            }
            ec.clear();
            it.increment(ec);
            continue;
        }

        std::size_t current_depth = 0;
        if constexpr(std::is_same_v<Iterator, fs::recursive_directory_iterator>)
        {
            current_depth = it.depth();
            if(options.max_depth > 0 && current_depth >= options.max_depth)
            {
                std::error_code temp_ec;
                if(it->is_directory(temp_ec))
                    it.disable_recursion_pending();
            }
        }

        if(!callback(it->path().string(), current_depth))
            break;

        it.increment(ec);
    }
}
} // namespace detail

inline std::string pwd()
{
    return fs::current_path().string();
}

inline std::string pwd(std::error_code &ec) noexcept
{
    auto p = fs::current_path(ec);
    if(ec)
        return "";

    return p.string();
}

inline std::string parent(const std::string &filepath)
{
    return fs::path(filepath).parent_path().string();
}

inline std::string absolute(const std::string &path)
{
    return fs::absolute(fs::path(path)).string();
}

inline std::string absolute(const std::string &path,
                            std::error_code   &ec) noexcept
{
    auto p = fs::absolute(fs::path(path), ec);
    if(ec)
        return "";

    return p.string();
}

inline std::string normalize(const std::string &path)
{
    return fs::path(path).lexically_normal().string();
}

inline std::string canonical(const std::string &path)
{
    return fs::canonical(fs::path(path)).string();
}

inline std::string canonical(const std::string &path,
                             std::error_code   &ec) noexcept
{
    auto p = fs::canonical(fs::path(path), ec);
    if(ec)
        return "";

    return p.string();
}

inline std::string weakly_canonical(const std::string &path)
{
    return fs::weakly_canonical(fs::path(path)).string();
}

inline std::string weakly_canonical(const std::string &path,
                                    std::error_code   &ec) noexcept
{
    auto p = fs::weakly_canonical(fs::path(path), ec);
    if(ec)
        return "";

    return p.string();
}

inline std::string relative(const std::string &path, const std::string &base)
{
    return fs::relative(fs::path(path), fs::path(base)).string();
}

inline std::string relative(const std::string &path,
                            const std::string &base,
                            std::error_code   &ec) noexcept
{
    auto p = fs::relative(fs::path(path), fs::path(base), ec);
    if(ec)
        return "";

    return p.string();
}

template <typename T,
          typename... Args,
          typename = typename std::enable_if<
              !std::is_same<typename std::decay<T>::type,
                            std::vector<std::string>>::value>::type>
inline std::string join(T &&first, Args &&...args)
{
    fs::path p(std::forward<T>(first));
    detail::join_helper(p, std::forward<Args>(args)...);
    return p.string();
}

template <typename Container>
inline auto join(const Container &c) ->
    typename std::enable_if<!std::is_constructible<fs::path, Container>::value,
                            std::string>::type
{
    fs::path p;
    for(const auto &s : c)
        p /= fs::path(s);

    return p.string();
}

inline std::string file_name(const std::string &file, bool with_ext = true)
{
    fs::path p(file);
    return with_ext ? p.filename().string() : p.stem().string();
}

inline std::string dir_name(const std::string &path)
{
    return fs::path(path).parent_path().filename().string();
}

inline std::string path_name(const std::string &filepath)
{
    return fs::path(filepath).parent_path().string();
}

inline std::string extension(const std::string &filepathname)
{
    return fs::path(filepathname).extension().string();
}

inline std::string replace_extension(const std::string &filepathname,
                                     const std::string &ext)
{
    return fs::path(filepathname).replace_extension(ext).string();
}

inline bool is_dir(const std::string &path)
{
    return fs::is_directory(fs::path(path));
}

inline bool is_dir(const std::string &path, std::error_code &ec) noexcept
{
    return fs::is_directory(fs::path(path), ec);
}

inline bool is_symlink(const std::string &file)
{
    return fs::is_symlink(fs::path(file));
}

inline bool is_symlink(const std::string &file, std::error_code &ec) noexcept
{
    return fs::is_symlink(fs::path(file), ec);
}

inline bool exists(const std::string &file)
{
    return fs::exists(fs::path(file));
}

inline bool exists(const std::string &file, std::error_code &ec) noexcept
{
    return fs::exists(fs::path(file), ec);
}

inline std::time_t last_mod_time(const std::string &filepath)
{
    std::error_code ec;
    auto            p     = fs::path(filepath);
    auto            ftime = fs::last_write_time(p, ec);
    if(ec)
        return 0;

    auto sctp =
        std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - fs::file_time_type::clock::now()
            + std::chrono::system_clock::now());
    return std::chrono::system_clock::to_time_t(sctp);
}

inline std::time_t last_mod_time(const std::string &filepath,
                                 std::error_code   &ec) noexcept
{
    auto p     = fs::path(filepath);
    auto ftime = fs::last_write_time(p, ec);
    if(ec)
        return 0;

    auto sctp =
        std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - fs::file_time_type::clock::now()
            + std::chrono::system_clock::now());
    return std::chrono::system_clock::to_time_t(sctp);
}

inline std::uintmax_t size(const std::string &file,
                           std::error_code   &ec) noexcept
{
    auto p = fs::path(file);
    if(fs::is_directory(p, ec))
    {
        if(!ec)
            ec = std::make_error_code(std::errc::is_a_directory);

        return 0;
    }
    if(ec)
        return 0;

    auto sz = fs::file_size(p, ec);
    if(ec)
        return 0;

    return sz;
}

inline std::optional<std::uintmax_t> size(const std::string &file)
{
    std::error_code ec;
    auto            sz = hj::filepath::size(file, ec);
    if(ec)
        return std::nullopt;

    return sz;
}

template <typename Callback,
          typename ErrorHandler = decltype(&default_walk_error_handler)>
inline void walk(const std::string &path,
                 Callback         &&callback,
                 walk_options       options     = {},
                 ErrorHandler     &&err_handler = default_walk_error_handler)
{
    std::error_code ec;
    walk(path,
         std::forward<Callback>(callback),
         options,
         ec,
         std::forward<ErrorHandler>(err_handler));
    if(ec && !options.continue_on_error)
        throw std::filesystem::filesystem_error("walk failed", path, ec);
}

template <typename Callback,
          typename ErrorHandler = decltype(&default_walk_error_handler)>
inline void
walk(const std::string &path,
     Callback         &&callback,
     walk_options       options,
     std::error_code   &ec,
     ErrorHandler     &&err_handler = default_walk_error_handler) noexcept
{
    if(options.recursive)
    {
        detail::walk_impl<fs::recursive_directory_iterator>(
            path,
            std::forward<Callback>(callback),
            options,
            ec,
            std::forward<ErrorHandler>(err_handler));
    } else
    {
        detail::walk_impl<fs::directory_iterator>(
            path,
            std::forward<Callback>(callback),
            options,
            ec,
            std::forward<ErrorHandler>(err_handler));
    }
}

inline std::vector<std::string>
list(const std::string &path, list_options opt, std::error_code &ec) noexcept
{
    std::vector<std::string> vec;
    ec.clear();
    std::error_code first_ec;
    auto fs_options = opt.skip_permission_denied
                          ? fs::directory_options::skip_permission_denied
                          : fs::directory_options::none;
    fs::directory_iterator it(path, fs_options, ec);
    fs::directory_iterator end;
    if(ec)
        return vec;

    while(it != end)
    {
        if(ec)
        {
            bool is_perm = (ec == std::errc::permission_denied
                            || ec.value() == EACCES || ec.value() == EPERM);
            if(opt.skip_permission_denied && is_perm)
            {
                ec.clear();
                it.increment(ec);
                continue;
            }

            if(!first_ec)
                first_ec = ec;

            if(!opt.continue_on_error)
                break;

            ec.clear();
            it.increment(ec);
            continue;
        }

        vec.emplace_back(it->path().string());
        it.increment(ec);
    }

    if(first_ec)
        ec = first_ec;

    return vec;
}

inline std::vector<std::string> list(const std::string &path,
                                     std::error_code   &ec) noexcept
{
    return list(path, list_options{}, ec);
}

inline std::vector<std::string> list(const std::string &path)
{
    std::error_code ec;
    auto            vec = list(path, list_options{}, ec);
    if(ec)
    {
        throw std::filesystem::filesystem_error("list failed", path, ec);
    }
    return vec;
}

template <typename Compare>
inline std::vector<std::string> list(const std::string &path, Compare &&cmp)
{
    std::vector<std::string> vec = list(path);
    std::sort(vec.begin(), vec.end(), std::forward<Compare>(cmp));
    return vec;
}

template <typename Compare>
inline std::vector<std::string>
list(const std::string &path, std::error_code &ec, Compare &&cmp) noexcept
{
    std::vector<std::string> vec = list(path, ec);
    if(ec)
        return vec;

    std::sort(vec.begin(), vec.end(), std::forward<Compare>(cmp));
    return vec;
}

inline std::vector<std::string> find(const std::string &path,
                                     const std::string &file,
                                     find_options       opt,
                                     std::error_code   &ec) noexcept
{
    std::vector<std::string> vec;
    ec.clear();
    if(!fs::exists(path, ec) || ec)
    {
        if(!ec)
            ec = std::make_error_code(std::errc::no_such_file_or_directory);

        return vec;
    }

    auto callback = [&](const std::string &current_path, std::size_t depth) {
        if(opt.max_depth > 0 && depth >= opt.max_depth)
            return true;

        fs::path curr(current_path);
        if(curr.filename().string() == file)
        {
            vec.emplace_back(current_path);
            if(opt.max_result > 0 && vec.size() >= opt.max_result)
                return false;
        }
        return true;
    };

    walk_options options(opt.recursive);
    options.max_depth              = opt.max_depth;
    options.skip_permission_denied = opt.skip_permission_denied;
    options.continue_on_error      = opt.continue_on_error;
    auto err_handler = [&](const std::string &, const std::error_code &err) {
        if(opt.skip_permission_denied
           && (err == std::errc::permission_denied || err.value() == EACCES
               || err.value() == EPERM))
            return true;

        if(opt.continue_on_error)
        {
            if(!ec)
                ec = err;

            return true;
        }
        if(!ec)
            ec = err;
        return false;
    };

    if(opt.recursive)
    {
        detail::walk_impl<fs::recursive_directory_iterator>(path,
                                                            callback,
                                                            options,
                                                            ec,
                                                            err_handler);
    } else
    {
        detail::walk_impl<fs::directory_iterator>(path,
                                                  callback,
                                                  options,
                                                  ec,
                                                  err_handler);
    }

    return vec;
}

inline std::vector<std::string>
find(const std::string &path, const std::string &file, find_options opt)
{
    std::error_code ec;
    auto            vec = hj::filepath::find(path, file, opt, ec);
    if(ec)
    {
        throw std::filesystem::filesystem_error("find failed", path, ec);
    }
    return vec;
}

inline std::vector<std::string> find_by_regex(const std::string &path,
                                              const std::string &pattern,
                                              find_options       opt,
                                              std::error_code   &ec) noexcept
{
    std::vector<std::string> vec;
    ec.clear();
    std::regex re;
    try
    {
        re.assign(pattern);
    }
    catch(const std::regex_error &)
    {
        ec = std::make_error_code(std::errc::invalid_argument);
        return vec;
    }

    if(!fs::exists(path, ec) || ec)
    {
        if(!ec)
            ec = std::make_error_code(std::errc::no_such_file_or_directory);

        return vec;
    }

    auto match_func = [&](const fs::path &entry_path) {
        if(opt.target == match_target::path)
            return std::regex_match(entry_path.string(), re);
        else
            return std::regex_match(entry_path.filename().string(), re);
    };

    auto callback = [&](const std::string &current_path, std::size_t depth) {
        if(opt.max_depth > 0 && depth >= opt.max_depth)
            return true;

        fs::path curr(current_path);
        if(match_func(curr))
        {
            vec.emplace_back(current_path);
            if(opt.max_result > 0 && vec.size() >= opt.max_result)
                return false;
        }
        return true;
    };

    walk_options options(opt.recursive);
    options.max_depth              = opt.max_depth;
    options.skip_permission_denied = opt.skip_permission_denied;
    options.continue_on_error      = opt.continue_on_error;
    auto err_handler = [&](const std::string &, const std::error_code &err) {
        if(opt.skip_permission_denied
           && (err == std::errc::permission_denied || err.value() == EACCES
               || err.value() == EPERM))
            return true;

        if(opt.continue_on_error)
        {
            if(!ec)
                ec = err;

            return true;
        }
        if(!ec)
            ec = err;

        return false;
    };

    if(opt.recursive)
    {
        detail::walk_impl<fs::recursive_directory_iterator>(path,
                                                            callback,
                                                            options,
                                                            ec,
                                                            err_handler);
    } else
    {
        detail::walk_impl<fs::directory_iterator>(path,
                                                  callback,
                                                  options,
                                                  ec,
                                                  err_handler);
    }

    return vec;
}

inline std::vector<std::string> find_by_regex(const std::string &path,
                                              const std::string &pattern,
                                              find_options       opt)
{
    std::error_code ec;
    auto            vec = hj::filepath::find_by_regex(path, pattern, opt, ec);
    if(ec)
    {
        throw std::filesystem::filesystem_error("find_by_regex failed",
                                                path,
                                                ec);
    }
    return vec;
}

inline bool
mkdir(const std::string &path, mkdir_options opt, std::error_code &ec) noexcept
{
    ec.clear();
    fs::path p(path);
    bool     created = opt.recursive ? fs::create_directories(p, ec)
                                     : fs::create_directory(p, ec);
    if(!ec && !created)
    {
        std::error_code temp_ec;
        if(fs::is_directory(p, temp_ec))
        {
            if(opt.succ_if_exist)
            {
                ec.clear();
                return true;
            } else
            {
                ec = std::make_error_code(std::errc::file_exists);
                return false;
            }
        } else
        {
            ec = std::make_error_code(std::errc::file_exists);
            return false;
        }
    }

    if(ec)
    {
        std::error_code temp_ec;
        if(fs::is_directory(p, temp_ec))
        {
            if(opt.succ_if_exist)
            {
                ec.clear();
                return true;
            } else
            {
                if(ec.value() == 0)
                    ec = std::make_error_code(std::errc::file_exists);
                return false;
            }
        }
        return false;
    }

    return true;
}

inline bool mkdir(const std::string &path, mkdir_options opt = {})
{
    std::error_code ec;
    bool            res = hj::filepath::mkdir(path, opt, ec);
    if(ec && !opt.succ_if_exist)
        throw std::filesystem::filesystem_error("mkdir failed", path, ec);

    return res;
}

inline bool touch(const std::string &file, std::error_code &ec) noexcept
{
    ec.clear();
    fs::path p(file);

#if defined(_WIN32) || defined(_WIN64)
    HANDLE hFile =
        CreateFileW(p.wstring().c_str(),
                    GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    nullptr,
                    CREATE_NEW,
                    FILE_ATTRIBUTE_NORMAL,
                    nullptr);
    if(hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
        return true;
    }

    DWORD err = GetLastError();
    if(err == ERROR_FILE_EXISTS || err == ERROR_ALREADY_EXISTS)
    {
        hFile =
            CreateFileW(p.wstring().c_str(),
                        FILE_WRITE_ATTRIBUTES,
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        nullptr,
                        OPEN_EXISTING,
                        FILE_FLAG_BACKUP_SEMANTICS,
                        nullptr);
        if(hFile != INVALID_HANDLE_VALUE)
        {
            SYSTEMTIME st;
            GetSystemTime(&st);
            FILETIME ft;
            SystemTimeToFileTime(&st, &ft);
            BOOL ok = SetFileTime(hFile, nullptr, nullptr, &ft);
            CloseHandle(hFile);
            if(ok)
                return true;
        }
    }
    ec.assign(static_cast<int>(GetLastError()), std::system_category());
    return false;
#else
    int fd = open(p.string().c_str(), O_WRONLY | O_CREAT | O_EXCL, 0666);
    if(fd >= 0)
    {
        close(fd);
        return true;
    }

    if(errno == EEXIST)
    {
        if(utimes(p.string().c_str(), nullptr) == 0)
            return true;
    }
    ec.assign(errno, std::system_category());
    return false;
#endif
}

inline bool touch(const std::string &file)
{
    std::error_code ec;
    bool            res = touch(file, ec);
    if(ec)
    {
        throw std::filesystem::filesystem_error("touch failed", file, ec);
    }
    return res;
}

inline bool
copy_dir(const std::string &from, const std::string &to, bool overwrite = false)
{
    std::error_code ec;
    auto            f_from = fs::path(from);
    auto            f_to   = fs::path(to);

    if(!fs::is_directory(f_from, ec) || ec)
        return false;

    auto options =
        fs::copy_options::recursive | fs::copy_options::copy_symlinks;
    if(overwrite)
        options |= fs::copy_options::overwrite_existing;
    else
        options |= fs::copy_options::skip_existing;

    fs::copy(f_from, f_to, options, ec);
    return !ec;
}

inline bool copy_dir(const std::string &from,
                     const std::string &to,
                     bool               overwrite,
                     std::error_code   &ec) noexcept
{
    auto f_from = fs::path(from);
    auto f_to   = fs::path(to);
    if(!fs::is_directory(f_from, ec) || ec)
        return false;

    auto options =
        fs::copy_options::recursive | fs::copy_options::copy_symlinks;
    if(overwrite)
        options |= fs::copy_options::overwrite_existing;
    else
        options |= fs::copy_options::skip_existing;

    fs::copy(f_from, f_to, options, ec);
    return !ec;
}

inline bool copy_file(const std::string &from,
                      const std::string &to,
                      std::error_code   &ec) noexcept
{
    ec.clear();
    fs::copy_file(from, to, ec);
    return !ec;
}

inline bool copy_file(const std::string &from, const std::string &to)
{
    std::error_code ec;
    bool            res = copy_file(from, to, ec);
    if(ec)
    {
        throw std::filesystem::filesystem_error("copy_file failed", from, ec);
    }
    return res;
}

inline bool remove(const std::string &filepath, std::error_code &ec) noexcept
{
    auto fpath = fs::path(filepath);
    return fs::remove(fpath, ec);
}

inline bool remove(const std::string &filepath)
{
    std::error_code ec;
    bool            res = remove(filepath, ec);
    if(ec)
    {
        throw std::filesystem::filesystem_error("remove failed", filepath, ec);
    }
    return res;
}

inline std::uintmax_t remove_all(const std::string &filepath,
                                 std::error_code   &ec) noexcept
{
    auto fpath = fs::path(filepath);
    return fs::remove_all(fpath, ec);
}

inline std::optional<std::uintmax_t> remove_all(const std::string &filepath)
{
    std::error_code ec;
    auto            count = hj::filepath::remove_all(filepath, ec);
    if(ec)
    {
        throw std::filesystem::filesystem_error("remove_all failed",
                                                filepath,
                                                ec);
    }
    return count;
}

inline bool rename(const std::string &from,
                   const std::string &to,
                   std::error_code   &ec) noexcept
{
    auto f_from = fs::path(from);
    auto f_to   = fs::path(to);

    fs::rename(f_from, f_to, ec);
    return !ec;
}

inline bool rename(const std::string &from, const std::string &to)
{
    std::error_code ec;
    bool            res = rename(from, to, ec);
    if(ec)
    {
        throw std::filesystem::filesystem_error("rename failed", from, ec);
    }
    return res;
}

} // namespace filepath
} // namespace hj

#endif