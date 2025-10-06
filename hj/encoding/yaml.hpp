/*
 *  This file is part of hj.
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

#ifndef YAML_HPP
#define YAML_HPP

#include <cstring>
#include <string>
#include <sstream>
#include <yaml-cpp/yaml.h>
#include <fstream>

namespace hj
{

class yaml
{
  public:
    using const_iterator = YAML::const_iterator;
    using iterator       = YAML::iterator;

  public:
    yaml()
        : _node()
    {
    }

    yaml(const yaml &rhs)
        : _node(rhs._node)
    {
    }

    inline ~yaml() = default;

    template <typename T>
    yaml &operator=(const T &rhs)
    {
        _node = rhs;
        return *this;
    }

    yaml &operator=(const yaml &rhs)
    {
        _node = rhs._node;
        return *this;
    }

    explicit operator bool() const { return _node.IsDefined(); }

    bool operator!() const { return !_node.IsDefined(); }

    template <typename Key>
    const yaml operator[](const Key &key) const
    {
        return yaml(_node[key]);
    }

    template <typename Key>
    yaml operator[](const Key &key)
    {
        return yaml(_node[key]);
    }

    template <typename Key>
    const yaml operator[](Key &&key) const
    {
        return yaml(_node[key]);
    }

    template <typename Key>
    yaml operator[](Key &&key)
    {
        return yaml(_node[key]);
    }

    inline bool is_null() const { return _node.IsNull(); }
    inline bool is_defined() const { return _node.IsDefined(); }
    inline bool is_scalar() const { return _node.IsScalar(); }
    inline bool is_sequence() const { return _node.IsSequence(); }
    inline bool is_map() const { return _node.IsMap(); }

    inline const std::string &tag() const { return _node.Tag(); }
    inline void set_tag(const std::string &tag) { _node.SetTag(tag); }

    inline const std::string &scalar() const { return _node.Scalar(); }
    inline std::string        str() const { return YAML::Dump(_node); }
    template <typename T>
    inline T as() const
    {
        return _node.as<T>();
    }

    inline const_iterator begin() const { return _node.begin(); }
    inline iterator       begin() { return _node.begin(); }
    inline const_iterator end() const { return _node.end(); }
    inline iterator       end() { return _node.end(); }

    inline void push_back(const yaml &rhs) { _node.push_back(rhs._node); }
    template <typename Key, typename Value>
    inline void force_insert(const Key &key, const Value &value)
    {
        _node.force_insert(key, value);
    }
    template <typename T>
    inline void push_back(const T &rhs)
    {
        _node.push_back(rhs);
    }

    static yaml load(const char *text) noexcept
    {
        try
        {
            std::stringstream ss(text);
            return yaml(YAML::Load(ss));
        }
        catch(...)
        {
            return yaml();
        }
    }

    yaml load(std::ifstream &fin) noexcept
    {
        if(!fin.is_open())
            return yaml();

        try
        {
            _node = YAML::Load(fin);
        }
        catch(const std::exception &e)
        {
            _node = YAML::Node();
        }
        return *this;
    }

    bool dump(char *buf, size_t &size) const
    {
        if(!buf || size == 0)
            return false;

        std::string yaml_str = YAML::Dump(_node);
        if(yaml_str.size() + 1 > size)
            return false; // +1 for null-terminator

        memcpy(buf, yaml_str.c_str(), yaml_str.size());
        buf[yaml_str.size()] = '\0';
        size                 = yaml_str.size();
        return true;
    }

    bool dump(std::ofstream &fout) const
    {
        if(!fout.is_open())
            return false;

        fout << YAML::Dump(_node);
        return true;
    }

  private:
    yaml(const YAML::Node &node)
        : _node(node)
    {
    }
    yaml &operator=(const YAML::Node &rhs)
    {
        _node = rhs;
        return *this;
    }

  private:
    YAML::Node _node;
};

}

#endif // YAML_HPP