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
#ifndef INI_HPP
#define INI_HPP

#include <string>
#include <string_view>
#include <sstream>
#include <streambuf>
#include <system_error>
#include <utility>
#include <stdexcept>
#include <optional>
#include <filesystem>
#include <cerrno>

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace hj
{

enum class ini_errc
{
    success = 0,
    invalid_argument,
    file_not_found,
    parser_error,
    bad_path_error,
    bad_data_error,
    filesystem_error,
    out_of_memory,
    std_exception,
    unknown_error
};

inline std::error_code make_error_code(ini_errc e) noexcept;

namespace detail
{
class ini_category final : public std::error_category
{
  public:
    const char *name() const noexcept override { return "hj::ini"; }

    std::string message(int ev) const override
    {
        switch(static_cast<ini_errc>(ev))
        {
            case ini_errc::success:
                return "Success";
            case ini_errc::invalid_argument:
                return "Invalid argument (nullptr or illegal parameter)";
            case ini_errc::file_not_found:
                return "File does not exist (ENOENT)";
            case ini_errc::parser_error:
                return "INI syntax parser error (check syntax or line number)";
            case ini_errc::bad_path_error:
                return "Property tree bad path error";
            case ini_errc::bad_data_error:
                return "Property tree bad data (type conversion failure)";
            case ini_errc::filesystem_error:
                return "Boost filesystem error";
            case ini_errc::out_of_memory:
                return "Out of memory (std::bad_alloc)";
            case ini_errc::std_exception:
                return "Caught standard library exception";
            case ini_errc::unknown_error:
                return "Unknown non-standard exception";
            default:
                return "Unknown ini error code";
        }
    }
};

inline const std::error_category &ini_category_instance() noexcept
{
    static ini_category inst;
    return inst;
}

/**
 * @brief Lightweight memory stream buffer for zero-copy string_view parsing.
 */
class membuf : public std::streambuf
{
  public:
    membuf(const char *base, size_t size)
    {
        char *p = const_cast<char *>(base);
        setg(p, p, p + size);
    }
};

/**
     * @brief Translate path string to ptree path type using '/' as separator.
     * @note hj::ini Path Syntax Convention:
     *       Users must use 'section/key' format (e.g., "server/port") to access 
     *       nested properties. The forward slash ('/') acts as the hierarchical 
     *       separator based on Boost Property Tree semantics.
     */
static inline boost::property_tree::ptree::path_type
translate(const std::string &s)
{
    return boost::property_tree::ptree::path_type(s, '/');
}

static inline std::error_code exception_to_error_code() noexcept
{
    try
    {
        throw;
    }
    catch(const boost::filesystem::filesystem_error &e)
    {
        if(e.code() == boost::system::errc::no_such_file_or_directory
           || e.code().value() == ENOENT || e.code().value() == 2)
        {
            return make_error_code(ini_errc::file_not_found);
        }
        return make_error_code(ini_errc::filesystem_error);
    }
    catch(const std::filesystem::filesystem_error &e)
    {
        if(e.code() == std::errc::no_such_file_or_directory
           || e.code().value() == 2)
        {
            return make_error_code(ini_errc::file_not_found);
        }
        return make_error_code(ini_errc::filesystem_error);
    }
    catch(const boost::property_tree::ini_parser::ini_parser_error &)
    {
        return make_error_code(ini_errc::parser_error);
    }
    catch(const boost::property_tree::ptree_bad_path &)
    {
        return make_error_code(ini_errc::bad_path_error);
    }
    catch(const boost::property_tree::ptree_bad_data &)
    {
        return make_error_code(ini_errc::bad_data_error);
    }
    catch(const std::bad_alloc &)
    {
        return make_error_code(ini_errc::out_of_memory);
    }
    catch(const std::exception &e)
    {
        std::string msg = e.what();
        if(msg.find("No such file") != std::string::npos)
        {
            return make_error_code(ini_errc::file_not_found);
        }
        return make_error_code(ini_errc::std_exception);
    }
    catch(...)
    {
        return make_error_code(ini_errc::unknown_error);
    }
}

} // namespace detail

inline std::error_code make_error_code(ini_errc e) noexcept
{
    return std::error_code(static_cast<int>(e),
                           detail::ini_category_instance());
}

} // namespace hj

namespace std
{
template <>
struct is_error_code_enum<hj::ini_errc> : std::true_type
{
};
} // namespace std

namespace hj
{

class ini
{
  private:
    boost::property_tree::ptree _tree;

  public:
    ini() = default;
    explicit ini(boost::property_tree::ptree tree)
        : _tree(std::move(tree))
    {
    }

    ini(const ini &rhs)     = default;
    ini(ini &&rhs) noexcept = default;

    ~ini() = default;

    ini &operator=(const ini &rhs)     = default;
    ini &operator=(ini &&rhs) noexcept = default;

    ini &operator=(const boost::property_tree::ptree &rhs)
    {
        _tree = rhs;
        return *this;
    }

    ini &operator=(boost::property_tree::ptree &&rhs) noexcept
    {
        _tree = std::move(rhs);
        return *this;
    }

    bool empty() const noexcept { return _tree.empty(); }

    /**
     * @brief Parse INI configuration from std::string_view with error code output (High Performance).
     * @param text INI text content (supports zero-copy via string_view).
     * @param ec Output error code.
     * @return ini Parsed ini configuration object.
     */
    static std::optional<ini> parse(std::string_view text,
                                    std::error_code &ec) noexcept
    {
        if(text.data() == nullptr && !text.empty())
        {
            ec = make_error_code(ini_errc::invalid_argument);
            return std::nullopt;
        }

        try
        {
            detail::membuf              buf(text.data(), text.size());
            std::istream                ss(&buf);
            boost::property_tree::ptree tree;
            boost::property_tree::ini_parser::read_ini(ss, tree);
            ec = make_error_code(ini_errc::success);
            return ini(std::move(tree));
        }
        catch(...)
        {
            ec = detail::exception_to_error_code();
            return std::nullopt;
        }
    }

    static std::optional<ini> try_parse(std::string_view text) noexcept
    {
        std::error_code ec;
        return parse(text, ec);
    }

    std::error_code read_file(const std::filesystem::path &filepath) noexcept
    {
        try
        {
            std::error_code ec_fs;
            if(!std::filesystem::exists(filepath, ec_fs)
               || !std::filesystem::is_regular_file(filepath, ec_fs))
            {
                return make_error_code(ini_errc::file_not_found);
            }

            boost::property_tree::ptree tree;
            boost::property_tree::ini_parser::read_ini(filepath.string(), tree);
            _tree = std::move(tree);
            return make_error_code(ini_errc::success);
        }
        catch(...)
        {
            return detail::exception_to_error_code();
        }
    }

    std::error_code read_file(const char *filepath) noexcept
    {
        if(!filepath)
            return make_error_code(ini_errc::invalid_argument);

        return read_file(std::filesystem::path(filepath));
    }

    std::error_code
    write_file(const std::filesystem::path &filepath) const noexcept
    {
        try
        {
            boost::property_tree::ini_parser::write_ini(filepath.string(),
                                                        _tree);
            return make_error_code(ini_errc::success);
        }
        catch(...)
        {
            return detail::exception_to_error_code();
        }
    }

    std::error_code write_file(const char *filepath) const noexcept
    {
        if(!filepath)
            return make_error_code(ini_errc::invalid_argument);

        return write_file(std::filesystem::path(filepath));
    }

    /**
     * @brief Serialize configuration to INI format string with error code reporting.
     * @param ec Output error code to distinguish serialization failure from empty content.
     * @return std::optional<std::string> Returns std::nullopt if serialization fails.
     */
    std::optional<std::string> str(std::error_code &ec) const noexcept
    {
        std::ostringstream ss;
        try
        {
            boost::property_tree::write_ini(ss, _tree);
            ec = make_error_code(ini_errc::success);
            return ss.str();
        }
        catch(...)
        {
            ec = detail::exception_to_error_code();
            return std::nullopt;
        }
    }

    /**
     * @brief Serialize configuration to INI format string safely without error code reference.
     * @return std::optional<std::string> Returns std::nullopt if serialization fails.
     */
    [[deprecated("Use str(std::error_code& ec) or try_str() instead.")]]
    std::optional<std::string> str() const noexcept
    {
        std::error_code ec;
        return str(ec);
    }

    /**
     * @brief Try to serialize configuration to INI format string safely without error code reference.
     * @return std::optional<std::string> Returns std::nullopt if serialization fails.
     */
    std::optional<std::string> try_str() const noexcept
    {
        std::error_code ec;
        return str(ec);
    }

    /**
     * @brief Get configuration value with error code reporting.
     * @param path Target property path using 'section/key' syntax.
     * @param ec Output error code to distinguish missing paths from type errors.
     */
    template <typename T>
    std::optional<T> get(const std::string &path,
                         std::error_code   &ec) const noexcept
    {
        try
        {
            auto val = _tree.get<T>(detail::translate(path));
            ec       = make_error_code(ini_errc::success);
            return val;
        }
        catch(const boost::property_tree::ptree_bad_path &)
        {
            ec = make_error_code(ini_errc::bad_path_error);
            return std::nullopt;
        }
        catch(const boost::property_tree::ptree_bad_data &)
        {
            ec = make_error_code(ini_errc::bad_data_error);
            return std::nullopt;
        }
        catch(...)
        {
            ec = detail::exception_to_error_code();
            return std::nullopt;
        }
    }

    /**
     * @brief Get configuration value with a fallback default value.
     * @param path Target property path using 'section/key' syntax.
     * @param default_value Fallback value if path is missing.
     */
    template <typename T>
    T get(const std::string &path, const T &default_value) const noexcept
    {
        std::error_code ec;
        auto            val = get<T>(path, ec);
        if(!val)
            return default_value;

        return *val;
    }

    /**
     * @brief Put configuration value.
     * @param path Target property path using 'section/key' syntax.
     * @param value Value to set.
     */
    template <typename T>
    void put(const std::string &path, const T &value)
    {
        _tree.put(detail::translate(path), value);
    }

    /**
     * @brief Get a nested child configuration section with error code reporting.
     * @param path Section path using 'section/key' syntax.
     * @param ec Output error code to distinguish a missing section from a successful query.
     * @return std::optional<ini> Returns std::nullopt if the child section does not exist.
     */
    std::optional<ini> get_child(const std::string &path,
                                 std::error_code   &ec) const noexcept
    {
        try
        {
            auto child_tree = _tree.get_child(detail::translate(path));
            ec              = make_error_code(ini_errc::success);
            return ini(std::move(child_tree));
        }
        catch(const boost::property_tree::ptree_bad_path &)
        {
            ec = make_error_code(ini_errc::bad_path_error);
            return std::nullopt;
        }
        catch(...)
        {
            ec = detail::exception_to_error_code();
            return std::nullopt;
        }
    }

    /**
     * @brief Try to get a nested child configuration section safely without error code reference.
     * @param path Section path using 'section/key' syntax.
     * @return std::optional<ini> Returns std::nullopt if the child section does not exist.
     */
    std::optional<ini> try_get_child(const std::string &path) const noexcept
    {
        std::error_code ec;
        return get_child(path, ec);
    }
};

} // namespace hj

#endif // INI_HPP