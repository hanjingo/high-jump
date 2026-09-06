/*
 * This file is part of high-jump(hj).
 *
 * Copyright 2025 hanjingo <hehehunanchina@live.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include <stdexcept>
#include <system_error>
#include <string>
#include <type_traits>
#include <vector>
#include <unordered_set>
#include <sstream>
#include <algorithm>
#include <optional>

#include <boost/program_options.hpp>

namespace hj
{

class options_error : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

class options_parse_error : public options_error
{
  public:
    using options_error::options_error;
};

class options_duplicate_error : public options_error
{
  public:
    using options_error::options_error;
};

class options_unknown_option : public options_parse_error
{
  public:
    using options_parse_error::options_parse_error;
};

class options_invalid_value : public options_parse_error
{
  public:
    using options_parse_error::options_parse_error;
};

class options_required_option : public options_parse_error
{
  public:
    using options_parse_error::options_parse_error;
};

/**
 * @brief Command line options parser based on Boost.Program_options.
 * 
 * @note Thread Safety:
 * hj::options is NOT thread-safe. It is designed to be used in a single-threaded 
 * setup phase (typically in the main thread) following the architecture below:
 * 
 *   main thread
 *       ↓
 *   parse() & configure
 *       ↓
 *   construct application configuration / immutable snapshot
 *       ↓
 *   worker threads (read-only / safe)
 * 
 * @note Exception Safety:
 * parse() provides the Strong Exception Guarantee. If parsing fails, the internal 
 * state (including any previous valid parsed state) remains completely untouched.
 */
class options
{
  public:
    options()  = default;
    ~options() = default;

    options(const options &)                = delete;
    options &operator=(const options &)     = delete;
    options(options &&) noexcept            = default;
    options &operator=(options &&) noexcept = default;

    template <typename T>
    void add(const char *key, const T &default_value, const char *memo = "")
    {
        if(_parsed)
            throw options_error(
                "Cannot add options after parse() has been invoked.");

        _check_duplicate_keys(key);
        _add_impl(key,
                  default_value,
                  memo,
                  std::is_same<T, std::vector<std::string>>{});
    }

    template <typename T>
    void add_required(const char *key, const char *memo = "")
    {
        if(_parsed)
            throw options_error(
                "Cannot add options after parse() has been invoked.");

        _check_duplicate_keys(key);
        _desc.add_options()(key,
                            boost::program_options::value<T>()->required(),
                            memo);
    }

    void add_flag(const char *key, const char *memo = "")
    {
        if(_parsed)
            throw options_error(
                "Cannot add options after parse() has been invoked.");

        _check_duplicate_keys(key);
        _desc.add_options()(key, boost::program_options::bool_switch(), memo);
    }

    void add_positional(const char *key, int max_count = 1)
    {
        if(_parsed)
            throw options_error("Cannot add positional options after parse() "
                                "has been invoked.");

        _pos.add(key, max_count);
    }

    void parse(int argc, char *argv[])
    {
        boost::program_options::variables_map new_vm;
        try
        {
            if(_pos.max_total_count() > 0)
            {
                boost::program_options::store(
                    boost::program_options::command_line_parser(argc, argv)
                        .options(_desc)
                        .positional(_pos)
                        .run(),
                    new_vm);
            } else
            {
                boost::program_options::store(
                    boost::program_options::parse_command_line(argc,
                                                               argv,
                                                               _desc),
                    new_vm);
            }
            boost::program_options::notify(new_vm);
        }
        catch(const boost::program_options::unknown_option &e)
        {
            throw options_unknown_option(e.what());
        }
        catch(const boost::program_options::invalid_option_value &e)
        {
            throw options_invalid_value(e.what());
        }
        catch(const boost::program_options::required_option &e)
        {
            throw options_required_option(e.what());
        }
        catch(const boost::program_options::error &e)
        {
            throw options_parse_error(e.what());
        }

        _vm.swap(new_vm);
        _parsed = true;
    }

    template <typename T>
    [[deprecated(
        "Use opts.parse(argc, argv) followed by opts.get<T>(key) instead.")]]
    T parse(int argc, char *argv[], const char *key)
    {
        parse(argc, argv);
        return get<T>(key);
    }

    template <typename T>
    [[deprecated("Use opts.parse(argc, argv) followed by opts.get<T>(key, "
                 "default) instead.")]]
    T parse(int argc, char *argv[], const char *key, const T &default_value)
    {
        parse(argc, argv);
        if(!_vm.count(key) || _vm[key].defaulted())
            return default_value;

        return get<T>(key);
    }

    template <typename T>
    T get(const char *key, const T &default_value = T{}) const
    {
        try
        {
            if(!_vm.count(key))
                return default_value;

            return _vm[key].as<T>();
        }
        catch(...)
        {
            return default_value;
        }
    }

    template <typename T>
    std::optional<T> try_get(const char *key) const
    {
        try
        {
            if(!_vm.count(key))
                return std::nullopt;

            return _vm[key].as<T>();
        }
        catch(...)
        {
            return std::nullopt;
        }
    }

    template <typename T = std::vector<std::string>>
    T get_positional(const char *key) const
    {
        static_assert(std::is_same<T, std::string>::value
                          || std::is_same<T, std::vector<std::string>>::value,
                      "hj::options::get_positional only supports std::string "
                      "or std::vector<std::string>.");
        return get<T>(key);
    }

    template <typename T = std::vector<std::string>>
    [[deprecated("Use opts.parse(argc, argv) followed by "
                 "opts.get_positional<T>(key) instead.")]]
    T parse_positional(int argc, char *argv[], const char *key)
    {
        parse(argc, argv);
        return get_positional<T>(key);
    }

    const boost::program_options::options_description &description() const
    {
        return _desc;
    }

  private:
    void _check_duplicate_keys(const char *key_str)
    {
        std::string       s(key_str);
        std::stringstream ss(s);
        std::string       segment;
        while(std::getline(ss, segment, ','))
        {
            auto start = segment.find_first_not_of(" \t");
            auto end   = segment.find_last_not_of(" \t");
            if(start == std::string::npos)
                continue;
            std::string name = segment.substr(start, end - start + 1);

            if(_registered_keys.find(name) != _registered_keys.end())
            {
                throw options_error("Duplicate option definition: " + name);
            }
            _registered_keys.insert(name);
        }
    }

    template <typename T>
    void _add_impl(const char *key,
                   const T    &default_value,
                   const char *memo,
                   std::false_type)
    {
        _desc.add_options()(
            key,
            boost::program_options::value<T>()->default_value(default_value),
            memo);
    }

    template <typename T>
    void _add_impl(const char *key, const T &, const char *memo, std::true_type)
    {
        _desc.add_options()(key, boost::program_options::value<T>(), memo);
    }

  private:
    boost::program_options::options_description            _desc;
    boost::program_options::positional_options_description _pos;
    boost::program_options::variables_map                  _vm;
    std::unordered_set<std::string>                        _registered_keys;
    bool                                                   _parsed{false};
};

} // namespace hj

#endif // OPTIONS_HPP