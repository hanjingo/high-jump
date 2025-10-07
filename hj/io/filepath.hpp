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

#include <string>
#include <list>
#include <vector>
#include <fstream>
#include <regex>
#include <boost/filesystem.hpp>

namespace hj
{

class filepath
{
  public:
    static std::string pwd() noexcept
    {
        try
        {
            return boost::filesystem::current_path().string();
        }
        catch(...)
        {
            return "";
        }
    }

    static std::string parent(const std::string &filepath) noexcept
    {
        try
        {
            return boost::filesystem::path(filepath).parent_path().string();
        }
        catch(...)
        {
            return "";
        }
    }

    static std::string absolute(const std::string &path) noexcept
    {
        try
        {
            return boost::filesystem::absolute(boost::filesystem::path(path))
                .string();
        }
        catch(...)
        {
            return "";
        }
    }

    static std::string relative(const std::string &path,
                                const std::string &base = pwd()) noexcept
    {
        try
        {
            return boost::filesystem::relative(boost::filesystem::path(path),
                                               boost::filesystem::path(base))
                .string();
        }
        catch(...)
        {
            return "";
        }
    }

    template <typename... Types>
    static std::string join(Types &&...args) noexcept
    {
        std::vector<std::string> v{std::forward<Types>(args)...};
        auto                     path = boost::filesystem::path("");
        for(const auto &s : v)
            path /= boost::filesystem::path(s);
        return path.string();
    }

    static std::string file_name(const std::string &file,
                                 bool               with_ext = true) noexcept
    {
        try
        {
            if(boost::filesystem::is_directory(file))
                return "";
            return with_ext ? boost::filesystem::path(file).filename().string()
                            : boost::filesystem::path(file).stem().string();
        }
        catch(...)
        {
            return "";
        }
    }

    static std::string dir_name(const std::string &path) noexcept
    {
        try
        {
            return boost::filesystem::path(path)
                .parent_path()
                .filename()
                .string();
        }
        catch(...)
        {
            return "";
        }
    }

    static std::string path_name(const std::string &filepath) noexcept
    {
        try
        {
            auto path = boost::filesystem::path(filepath);
            return boost::filesystem::is_directory(path)
                       ? filepath
                       : path.parent_path().string();
        }
        catch(...)
        {
            return "";
        }
    }

    static std::string extension(const std::string &filepath) noexcept
    {
        try
        {
            return boost::filesystem::path(filepath).extension().string();
        }
        catch(...)
        {
            return "";
        }
    }

    static std::string replace_extension(const std::string &filepath,
                                         const std::string &ext) noexcept
    {
        try
        {
            return boost::filesystem::path(filepath)
                .replace_extension(ext)
                .string();
        }
        catch(...)
        {
            return filepath;
        }
    }

    static bool is_dir(const std::string &path) noexcept
    {
        try
        {
            return boost::filesystem::is_directory(
                boost::filesystem::path(path));
        }
        catch(...)
        {
            return false;
        }
    }

    static bool is_symlink(const std::string &file) noexcept
    {
        try
        {
            return boost::filesystem::is_symlink(boost::filesystem::path(file));
        }
        catch(...)
        {
            return false;
        }
    }

    static bool is_exist(const std::string &file) noexcept
    {
        try
        {
            return boost::filesystem::exists(boost::filesystem::path(file));
        }
        catch(...)
        {
            return false;
        }
    }

    static std::time_t last_mod_time(const std::string &filepath) noexcept
    {
        try
        {
            return boost::filesystem::last_write_time(
                boost::filesystem::path(filepath));
        }
        catch(...)
        {
            return 0;
        }
    }

    static long long size(const std::string &file) noexcept
    {
        try
        {
            if(!is_exist(file) || is_dir(file))
                return -1;
            return boost::filesystem::file_size(boost::filesystem::path(file));
        }
        catch(...)
        {
            return -1;
        }
    }

    static void walk(const std::string                       &path,
                     std::function<bool(const std::string &)> fn,
                     bool recursive = false) noexcept
    {
        try
        {
            if(recursive)
            {
                for(auto &entry :
                    boost::filesystem::recursive_directory_iterator(path))
                    if(!fn(entry.path().string()))
                        return;
            } else
            {
                for(auto &entry : boost::filesystem::directory_iterator(path))
                    if(!fn(entry.path().string()))
                        return;
            }
        }
        catch(...)
        {
        }
    }

    static std::list<std::string> list(const std::string &path) noexcept
    {
        std::list<std::string> li;
        try
        {
            for(auto &entry : boost::filesystem::directory_iterator(path))
                li.emplace_back(entry.path().string());
        }
        catch(...)
        {
        }
        return li;
    }

    static std::list<std::string>
    list(const std::string &path,
         std::function<bool(const std::string &, const std::string &)>
             greater) noexcept
    {
        std::list<std::string> li;
        try
        {
            for(auto &entry : boost::filesystem::directory_iterator(path))
            {
                auto item = entry.path().string();
                if(li.empty())
                {
                    li.emplace_back(item);
                    continue;
                }
                for(auto litr = li.begin(); litr != li.end(); litr++)
                {
                    if(greater(*litr, item))
                        continue;
                    li.insert(litr, item);
                    break;
                }
            }
        }
        catch(...)
        {
        }
        return li;
    }

    static std::list<std::string> find(const std::string &path,
                                       const std::string &file,
                                       bool recursive = false) noexcept
    {
        std::list<std::string> li;
        try
        {
            if(!boost::filesystem::exists(path))
                return li;
            if(recursive)
            {
                for(auto &entry :
                    boost::filesystem::recursive_directory_iterator(path))
                    if(entry.path().filename().string() == file)
                        li.emplace_back(entry.path().string());
            } else
            {
                for(auto &entry : boost::filesystem::directory_iterator(path))
                    if(entry.path().filename().string() == file)
                        li.emplace_back(entry.path().string());
            }
        }
        catch(...)
        {
        }
        return li;
    }

    static std::list<std::string> find_by_regex(const std::string &path,
                                                const std::string &pattern,
                                                bool recursive = false) noexcept
    {
        std::list<std::string> li;
        try
        {
            std::regex re(pattern);
            if(recursive)
            {
                for(auto &entry :
                    boost::filesystem::recursive_directory_iterator(path))
                    if(std::regex_match(entry.path().filename().string(), re))
                        li.emplace_back(entry.path().string());
            } else
            {
                for(auto &entry : boost::filesystem::directory_iterator(path))
                    if(std::regex_match(entry.path().filename().string(), re))
                        li.emplace_back(entry.path().string());
            }
        }
        catch(...)
        {
        }
        return li;
    }

    static bool make_dir(const std::string &path,
                         bool               recursive = true) noexcept
    {
        try
        {
            return recursive ? boost::filesystem::create_directories(path)
                             : boost::filesystem::create_directory(path);
        }
        catch(...)
        {
            return false;
        }
    }

    static bool make_file(const std::string &file) noexcept
    {
        try
        {
            if(boost::filesystem::exists(file))
                return false;
            std::ofstream fs(file);
            if(!fs.is_open())
                return false;
            fs.close();
            return true;
        }
        catch(...)
        {
            return false;
        }
    }

    static bool copy_dir(const std::string &from,
                         const std::string &to) noexcept
    {
        try
        {
            auto f_from = boost::filesystem::path(from);
            auto f_to   = boost::filesystem::path(to);

            if(!boost::filesystem::exists(f_from)
               || !boost::filesystem::is_directory(f_from))
                return false;

            if(!boost::filesystem::exists(f_to))
                boost::filesystem::create_directory(f_to);

            for(auto &file : boost::filesystem::directory_iterator(f_from))
            {
                const auto &entry = file.path();
                auto        dest  = f_to / entry.filename();
                if(boost::filesystem::is_directory(entry))
                {
                    if(!copy_dir(entry.string(), dest.string()))
                        return false;
                } else
                {
                    boost::filesystem::copy_file(entry, dest);
                }
            }
            return true;
        }
        catch(...)
        {
            return false;
        }
    }

    static bool copy_file(const std::string &from,
                          const std::string &to) noexcept
    {
        try
        {
            boost::filesystem::copy_file(from, to);
            return boost::filesystem::exists(to);
        }
        catch(...)
        {
            return false;
        }
    }

    static unsigned long long remove(const std::string &filepath,
                                     bool recursive = true) noexcept
    {
        try
        {
            auto fpath = boost::filesystem::path(filepath);
            return recursive ? boost::filesystem::remove_all(fpath)
                             : boost::filesystem::remove(fpath);
        }
        catch(...)
        {
            return 0;
        }
    }

    static std::string rename(const std::string &from,
                              const std::string &to) noexcept
    {
        try
        {
            auto f_from = boost::filesystem::path(from);
            auto f_to   = boost::filesystem::path(to);
            boost::filesystem::rename(f_from, f_to);
            return boost::filesystem::exists(f_to) ? to : from;
        }
        catch(...)
        {
            return from;
        }
    }

  private:
    filepath()                            = default;
    ~filepath()                           = default;
    filepath(const filepath &)            = delete;
    filepath &operator=(const filepath &) = delete;
    filepath(filepath &&)                 = delete;
    filepath &operator=(filepath &&)      = delete;
};

}

#endif