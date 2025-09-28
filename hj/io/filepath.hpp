#ifndef FILEPATH_HPP
#define FILEPATH_HPP

#include <string>
#include <list>
#include <fstream>

#include <boost/filesystem.hpp>

namespace hj
{

class filepath
{
  public:
    static std::string pwd()
    {
        return boost::filesystem::current_path().string();
    }

    static std::string parent(const std::string &filepath)
    {
        return boost::filesystem::path(filepath).parent_path().string();
    }

    static std::string absolute(const std::string &path)
    {
        return boost::filesystem::absolute(boost::filesystem::path(path))
            .string();
    }

    static std::string relative(const std::string &path,
                                const std::string &base = pwd())
    {
        return boost::filesystem::relative(boost::filesystem::path(path),
                                           boost::filesystem::path(base))
            .string();
    }

    template <typename... Types>
    static std::string join(Types &&...args)
    {
        std::vector<std::string> v{std::forward<Types>(args)...};
        auto                     path = boost::filesystem::path("");
        for(auto itr = v.begin(); itr != v.end(); itr++)
        {
            path /= boost::filesystem::path(*itr);
        }

        return path.string();
    }

    static std::string file_name(const std::string &file,
                                 const bool         with_ext = true)
    {
        if(boost::filesystem::is_directory(file))
        {
            return "";
        }

        return with_ext ? boost::filesystem::path(file).filename().string()
                        : boost::filesystem::path(file).stem().string();
    }

    static std::string dir_name(const std::string &path)
    {
        return boost::filesystem::path(path).parent_path().filename().string();
    }

    static std::string path_name(const std::string &filepath)
    {
        auto path = boost::filesystem::path(filepath);
        return boost::filesystem::is_directory(path)
                   ? filepath
                   : path.parent_path().string();
    }

    static std::string extension(const std::string &filepath)
    {
        return boost::filesystem::path(filepath).extension().string();
    }

    static std::string replace_extension(const std::string &filepath,
                                         const std::string &ext)
    {
        return boost::filesystem::path(filepath)
            .replace_extension(ext)
            .string();
    }

    static bool is_dir(const std::string &path)
    {
        return boost::filesystem::is_directory(boost::filesystem::path(path));
    }

    static bool is_symlink(const std::string &file)
    {
        return boost::filesystem::is_symlink(boost::filesystem::path(file));
    }

    static bool is_exist(const std::string &file)
    {
        return boost::filesystem::exists(boost::filesystem::path(file));
    }

    static std::time_t last_mod_time(const std::string &filepath)
    {
        return boost::filesystem::last_write_time(
            boost::filesystem::path(filepath));
    }

    static long long size(const std::string &file)
    {
        if(!is_exist(file) || is_dir(file))
            return -1; // Return -1 for directories or non-existent files

        return boost::filesystem::file_size(boost::filesystem::path(file));
    }

    static void walk(const std::string                       &path,
                     std::function<bool(const std::string &)> fn,
                     bool                                     recursive = false)
    {
        if(recursive)
        {
            auto itr = boost::filesystem::recursive_directory_iterator(
                boost::filesystem::path(path));
            for(; itr != boost::filesystem::recursive_directory_iterator();
                itr++)
            {
                if(!fn(itr->path().string()))
                {
                    return;
                }
            }
            return;
        }

        auto itr = boost::filesystem::directory_iterator(
            boost::filesystem::path(path));
        for(; itr != boost::filesystem::directory_iterator(); itr++)
        {
            if(!fn(itr->path().string()))
            {
                return;
            }
        }
    }

    static std::list<std::string> list(const std::string &path)
    {
        std::list<std::string> li{};
        auto                   itr = boost::filesystem::directory_iterator(
            boost::filesystem::path(path));
        for(; itr != boost::filesystem::directory_iterator(); itr++)
        {
            li.emplace_back(itr->path().string());
        }

        return li;
    }

    static std::list<std::string>
    list(const std::string                                            &path,
         std::function<bool(const std::string &, const std::string &)> greater)
    {
        std::list<std::string> li{};
        auto                   itr = boost::filesystem::directory_iterator(
            boost::filesystem::path(path));
        for(; itr != boost::filesystem::directory_iterator(); itr++)
        {
            auto item = itr->path().string();
            if(li.empty())
            {
                li.emplace_back(item);
                continue;
            }

            for(auto litr = li.begin(); litr != li.end(); litr++)
            {
                if(greater(*litr, item))
                {
                    continue;
                }

                li.insert(litr, item);
                break;
            }
        }

        return li;
    }

    static std::list<std::string> find(const std::string &path,
                                       const std::string &file,
                                       bool               recursive = false)
    {
        auto                   fpath = boost::filesystem::path(path);
        std::list<std::string> li{};
        if(!boost::filesystem::exists(fpath))
        {
            return li;
        }

        if(recursive)
        {
            auto itr = boost::filesystem::recursive_directory_iterator(fpath);
            for(; itr != boost::filesystem::recursive_directory_iterator();
                itr++)
            {
                if(itr->path().filename().string() != file)
                {
                    continue;
                }

                li.emplace_back(itr->path().string());
            }
            return li;
        }

        auto itr = boost::filesystem::directory_iterator(fpath);
        for(; itr != boost::filesystem::directory_iterator(); itr++)
        {
            if(itr->path().filename().string() != file)
            {
                continue;
            }

            li.emplace_back(itr->path().string());
            return li;
        }
        return li;
    }

    // static std::list<std::string> find_by_regex(const std::string& path, const std::string& pattern, bool recursive = false)
    // {
    // }

    static bool make_dir(const std::string &path, bool recursive = true)
    {
        return recursive ? boost::filesystem::create_directories(
                               boost::filesystem::path(path))
                         : boost::filesystem::create_directory(
                               boost::filesystem::path(path));
    }

    static bool make_file(const std::string &file)
    {
        if(boost::filesystem::exists(boost::filesystem::path(file)))
            return false;

        std::ofstream fs(file);
        if(!fs.is_open())
            return false;

        fs.close();
        return true;
    }

    static bool copy_dir(const std::string &from, const std::string &to)
    {
        auto f_from = boost::filesystem::path(from);
        auto f_to   = boost::filesystem::path(to);

        if(!boost::filesystem::exists(f_from)
           || !boost::filesystem::is_directory(f_from))
            return false;

        if(!boost::filesystem::exists(f_to))
            boost::filesystem::create_directory(f_to);

        for(boost::filesystem::directory_iterator file(f_from);
            file != boost::filesystem::directory_iterator();
            ++file)
        {
            const auto &entry = file->path();
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

    static bool copy_file(const std::string &from, const std::string &to)
    {
        auto f_from = boost::filesystem::path(from);
        auto f_to   = boost::filesystem::path(to);
        boost::filesystem::copy_file(from, to);

        return boost::filesystem::exists(f_to);
    }

    static unsigned long long remove(const std::string &filepath,
                                     bool               recursive = true)
    {
        auto fpath = boost::filesystem::path(filepath);
        return recursive ? boost::filesystem::remove_all(fpath)
                         : boost::filesystem::remove(fpath);
    }

    static std::string rename(const std::string &from, const std::string &to)
    {
        auto f_from = boost::filesystem::path(from);
        auto f_to   = boost::filesystem::path(to);
        boost::filesystem::rename(f_from, f_to);

        return boost::filesystem::exists(f_to) ? to : from;
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