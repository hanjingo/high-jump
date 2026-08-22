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
#include <ctime>
#include <filesystem>
#include <fstream>
#include <functional>
#include <regex>
#include <string>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>

namespace hj
{
namespace filepath
{

namespace fs = std::filesystem;

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
} // namespace detail

enum class match_target
{
    filename,
    path
};

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
    if(fs::is_directory(p))
        return "";
    return with_ext ? p.filename().string() : p.stem().string();
}

inline std::string dir_name(const std::string &path)
{
    return fs::path(path).parent_path().filename().string();
}

inline std::string path_name(const std::string &filepath)
{
    auto path = fs::path(filepath);
    return fs::is_directory(path) ? filepath : path.parent_path().string();
}

inline std::string extension(const std::string &filepath)
{
    return fs::path(filepath).extension().string();
}

inline std::string replace_extension(const std::string &filepath,
                                     const std::string &ext)
{
    return fs::path(filepath).replace_extension(ext).string();
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

inline bool is_exist(const std::string &file)
{
    return fs::exists(fs::path(file));
}

inline bool is_exist(const std::string &file, std::error_code &ec) noexcept
{
    return fs::exists(fs::path(file), ec);
}

inline std::time_t last_mod_time(const std::string &filepath)
{
    auto p = fs::path(filepath);
    if(!fs::exists(p))
        return 0;

    auto ftime = fs::last_write_time(p);
    auto sctp =
        std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - fs::file_time_type::clock::now()
            + std::chrono::system_clock::now());
    return std::chrono::system_clock::to_time_t(sctp);
}

inline std::time_t last_mod_time(const std::string &filepath,
                                 std::error_code   &ec) noexcept
{
    auto p = fs::path(filepath);
    if(!fs::exists(p, ec) || ec)
        return 0;

    auto ftime = fs::last_write_time(p, ec);
    if(ec)
        return 0;

    auto sctp =
        std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - fs::file_time_type::clock::now()
            + std::chrono::system_clock::now());
    return std::chrono::system_clock::to_time_t(sctp);
}

inline std::uintmax_t size(const std::string &file)
{
    std::error_code ec;
    if(!fs::exists(fs::path(file), ec) || ec || fs::is_directory(file, ec))
        return -1;

    return fs::file_size(file);
}

inline std::uintmax_t size(const std::string &file,
                           std::error_code   &ec) noexcept
{
    if(!fs::exists(fs::path(file), ec) || ec || fs::is_directory(file, ec))
        return static_cast<std::uintmax_t>(-1);

    auto sz = fs::file_size(file, ec);
    if(ec)
        return static_cast<std::uintmax_t>(-1);

    return sz;
}

template <typename Callback>
inline void walk(const std::string &path,
                 Callback         &&callback,
                 bool               recursive       = false,
                 bool               skip_permission = true)
{
    std::error_code ec;
    auto options = skip_permission
                       ? fs::directory_options::skip_permission_denied
                       : fs::directory_options::none;

    if(recursive)
    {
        fs::recursive_directory_iterator it(path, options, ec);
        fs::recursive_directory_iterator end;
        while(it != end)
        {
            if(ec)
            {
                ec.clear();
                it.increment(ec);
                continue;
            }

            if(!callback(it->path().string()))
                break;

            it.increment(ec);
        }
    } else
    {
        fs::directory_iterator it(path, options, ec);
        fs::directory_iterator end;
        while(it != end)
        {
            if(ec)
            {
                ec.clear();
                it.increment(ec);
                continue;
            }

            if(!callback(it->path().string()))
                break;

            it.increment(ec);
        }
    }
}

template <typename Callback>
inline void walk(const std::string &path,
                 Callback         &&callback,
                 bool               recursive,
                 bool               skip_permission,
                 std::error_code   &ec) noexcept
{
    auto options = skip_permission
                       ? fs::directory_options::skip_permission_denied
                       : fs::directory_options::none;

    if(recursive)
    {
        fs::recursive_directory_iterator it(path, options, ec);
        if(ec)
            return;

        fs::recursive_directory_iterator end;
        while(it != end)
        {
            if(ec)
            {
                ec.clear();
                it.increment(ec);
                continue;
            }

            if(!callback(it->path().string()))
                break;

            it.increment(ec);
        }
    } else
    {
        fs::directory_iterator it(path, options, ec);
        if(ec)
            return;

        fs::directory_iterator end;
        while(it != end)
        {
            if(ec)
            {
                ec.clear();
                it.increment(ec);
                continue;
            }

            if(!callback(it->path().string()))
                break;

            it.increment(ec);
        }
    }
}

inline std::vector<std::string> list(const std::string &path)
{
    std::vector<std::string> vec;
    for(auto &entry : fs::directory_iterator(path))
        vec.emplace_back(entry.path().string());
    return vec;
}

inline std::vector<std::string> list(const std::string &path,
                                     std::error_code   &ec) noexcept
{
    std::vector<std::string> vec;
    auto                     it = fs::directory_iterator(path, ec);
    if(ec)
        return vec;
    fs::directory_iterator end;
    while(it != end)
    {
        if(ec)
        {
            ec.clear();
            it.increment(ec);
            continue;
        }
        vec.emplace_back(it->path().string());
        it.increment(ec);
    }
    return vec;
}

inline std::vector<std::string>
list(const std::string                                            &path,
     std::function<bool(const std::string &, const std::string &)> cmp)
{
    std::vector<std::string> vec;
    for(auto &entry : fs::directory_iterator(path))
        vec.emplace_back(entry.path().string());

    std::sort(vec.begin(), vec.end(), cmp);
    return vec;
}

inline std::vector<std::string>
find(const std::string &path, const std::string &file, bool recursive = false)
{
    std::vector<std::string> vec;
    if(!fs::exists(path))
        return vec;
    if(recursive)
    {
        for(auto &entry : fs::recursive_directory_iterator(path))
            if(entry.path().filename().string() == file)
                vec.emplace_back(entry.path().string());
    } else
    {
        for(auto &entry : fs::directory_iterator(path))
            if(entry.path().filename().string() == file)
                vec.emplace_back(entry.path().string());
    }
    return vec;
}

inline std::vector<std::string>
find_by_regex(const std::string &path,
              const std::string &pattern,
              bool               recursive = false,
              match_target       target    = match_target::filename)
{
    std::vector<std::string> vec;
    std::regex               re(pattern);
    if(!fs::exists(path))
        return vec;

    auto match_func = [&](const fs::path &entry_path) {
        if(target == match_target::path)
            return std::regex_match(entry_path.string(), re);
        else
            return std::regex_match(entry_path.filename().string(), re);
    };

    if(recursive)
    {
        for(auto &entry : fs::recursive_directory_iterator(path))
            if(match_func(entry.path()))
                vec.emplace_back(entry.path().string());
    } else
    {
        for(auto &entry : fs::directory_iterator(path))
            if(match_func(entry.path()))
                vec.emplace_back(entry.path().string());
    }
    return vec;
}

inline bool make_dir(const std::string &path, bool recursive = true)
{
    return recursive ? fs::create_directories(path)
                     : fs::create_directory(path);
}

inline bool
make_dir(const std::string &path, bool recursive, std::error_code &ec) noexcept
{
    return recursive ? fs::create_directories(path, ec)
                     : fs::create_directory(path, ec);
}

inline bool create_file(const std::string &file)
{
    if(fs::exists(file))
        return false;

    std::ofstream ofs(file);
    if(!ofs.is_open())
        return false;

    ofs.close();
    return true;
}

inline bool
copy_dir(const std::string &from, const std::string &to, bool overwrite = true)
{
    std::error_code ec;
    auto            f_from = fs::path(from);
    auto            f_to   = fs::path(to);

    if(!fs::exists(f_from, ec) || !fs::is_directory(f_from, ec) || ec)
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
    if(!fs::exists(f_from, ec) || !fs::is_directory(f_from, ec) || ec)
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

inline bool copy_file(const std::string &from, const std::string &to)
{
    fs::copy_file(from, to);
    return fs::exists(to);
}

inline bool copy_file(const std::string &from,
                      const std::string &to,
                      std::error_code   &ec) noexcept
{
    fs::copy_file(from, to, ec);
    if(ec)
        return false;

    return fs::exists(to, ec);
}

inline bool remove(const std::string &filepath)
{
    std::error_code ec;
    auto            fpath = fs::path(filepath);
    bool            res   = fs::remove(fpath, ec);
    return res && !ec;
}

inline bool remove(const std::string &filepath, std::error_code &ec) noexcept
{
    auto fpath = fs::path(filepath);
    return fs::remove(fpath, ec);
}

inline std::uintmax_t remove_all(const std::string &filepath)
{
    std::error_code ec;
    auto            fpath = fs::path(filepath);
    auto            count = fs::remove_all(fpath, ec);
    if(ec)
        return static_cast<std::uintmax_t>(-1);

    return count;
}

inline std::uintmax_t remove_all(const std::string &filepath,
                                 std::error_code   &ec) noexcept
{
    auto fpath = fs::path(filepath);
    return fs::remove_all(fpath, ec);
}

inline bool rename(const std::string &from, const std::string &to)
{
    std::error_code ec;
    auto            f_from = fs::path(from);
    auto            f_to   = fs::path(to);
    fs::rename(f_from, f_to, ec);
    return !ec;
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

} // namespace filepath
} // namespace hj

#endif