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

#ifndef YAML_HPP
#define YAML_HPP

#include <yaml-cpp/yaml.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

namespace hj
{

enum class convert_error
{
    not_defined,
    bad_conversion
};

class yaml
{
  public:
    struct arrow_proxy;

    template <bool IsConst>
    class iterator_wrapper
    {
      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = yaml;
        using difference_type   = std::ptrdiff_t;
        using pointer           = arrow_proxy;
        using reference         = yaml;

        using BaseIter =
            std::conditional_t<IsConst, YAML::const_iterator, YAML::iterator>;

        iterator_wrapper() = default;

        explicit iterator_wrapper(BaseIter iter)
            : _iter(std::move(iter))
        {
        }

        template <bool OtherIsConst,
                  typename = std::enable_if_t<IsConst && !OtherIsConst>>
        iterator_wrapper(const iterator_wrapper<OtherIsConst> &other) noexcept
            : _iter(other.base())
        {
        }

        reference operator*() const;
        pointer   operator->() const;

        iterator_wrapper &operator++()
        {
            ++_iter;
            return *this;
        }

        iterator_wrapper operator++(int)
        {
            iterator_wrapper tmp = *this;
            ++_iter;
            return tmp;
        }

        template <bool OtherIsConst>
        bool operator==(const iterator_wrapper<OtherIsConst> &rhs) const
        {
            return _iter == rhs.base();
        }

        template <bool OtherIsConst>
        bool operator!=(const iterator_wrapper<OtherIsConst> &rhs) const
        {
            return _iter != rhs.base();
        }

        yaml key() const;
        yaml value() const;

        const BaseIter &base() const noexcept { return _iter; }

      private:
        BaseIter _iter;
    };

    using iterator       = iterator_wrapper<false>;
    using const_iterator = iterator_wrapper<true>;

  public:
    yaml()
        : _node(YAML::NodeType::Undefined)
    {
    }

    yaml(const yaml &rhs)                = default;
    yaml(yaml &&rhs) noexcept            = default;
    yaml &operator=(const yaml &rhs)     = default;
    yaml &operator=(yaml &&rhs) noexcept = default;

    ~yaml() = default;

    yaml clone() const { return yaml(YAML::Clone(_node)); }

    template <typename T>
    yaml &operator=(const T &rhs)
    {
        _node = rhs;
        return *this;
    }

    static yaml load_from_string(std::string_view text)
    {
        return yaml(YAML::Load(std::string(text.data(), text.size())));
    }

    static yaml load(const char *text)
    {
        if(!text)
        {
            throw std::invalid_argument("hj::yaml::load: null pointer passed");
        }
        return load_from_string(text);
    }

    static yaml load_from_file(const std::filesystem::path &file_path)
    {
        if(!std::filesystem::exists(file_path))
        {
            throw std::filesystem::filesystem_error(
                "File does not exist",
                file_path,
                std::make_error_code(std::errc::no_such_file_or_directory));
        }
        return yaml(YAML::LoadFile(file_path.string()));
    }

    static yaml load_from_stream(std::istream &in)
    {
        if(!in.good())
        {
            throw std::runtime_error(
                "hj::yaml::load_from_stream: stream is in bad/failed state");
        }
        return yaml(YAML::Load(in));
    }

    static std::optional<yaml>
    try_load_from_string(std::string_view text) noexcept
    {
        try
        {
            return yaml(YAML::Load(std::string(text.data(), text.size())));
        }
        catch(...)
        {
            return std::nullopt;
        }
    }

    static std::optional<yaml> try_load(const char *text) noexcept
    {
        if(!text)
        {
            return std::nullopt;
        }
        return try_load_from_string(text);
    }

    static std::optional<yaml>
    try_load_from_file(const std::filesystem::path &file_path) noexcept
    {
        try
        {
            std::error_code ec;
            if(!std::filesystem::exists(file_path, ec)
               || !std::filesystem::is_regular_file(file_path, ec))
            {
                return std::nullopt;
            }
            return yaml(YAML::LoadFile(file_path.string()));
        }
        catch(...)
        {
            return std::nullopt;
        }
    }

    static std::optional<yaml> try_load_from_stream(std::istream &in) noexcept
    {
        if(!in.good())
        {
            return std::nullopt;
        }
        try
        {
            return yaml(YAML::Load(in));
        }
        catch(...)
        {
            return std::nullopt;
        }
    }

    explicit operator bool() const
    {
        return _node.IsDefined() && !_node.IsNull();
    }
    bool operator!() const { return !_node.IsDefined() || _node.IsNull(); }

    bool is_null() const { return _node.IsNull(); }
    bool is_defined() const { return _node.IsDefined(); }
    bool is_scalar() const { return _node.IsScalar(); }
    bool is_sequence() const { return _node.IsSequence(); }
    bool is_map() const { return _node.IsMap(); }

    template <typename Key>
    yaml operator[](Key &&key) const
    {
        return yaml(_node[std::forward<Key>(key)]);
    }

    template <typename Key>
    yaml operator[](Key &&key)
    {
        return yaml(_node[std::forward<Key>(key)]);
    }

    std::string tag() const { return _node.Tag(); }
    void set_tag(std::string_view tag) { _node.SetTag(std::string(tag)); }

    std::optional<std::string> scalar_optional() const noexcept
    {
        if(!_node.IsDefined() || !_node.IsScalar())
        {
            return std::nullopt;
        }
        return _node.Scalar();
    }

    std::string scalar() const
    {
        if(!_node.IsDefined() || !_node.IsScalar())
        {
            throw YAML::BadConversion(_node.Mark());
        }
        return _node.Scalar();
    }

    std::string str() const { return YAML::Dump(_node); }

    template <typename T>
    T as() const
    {
        return _node.as<T>();
    }

    template <typename T>
    std::optional<T> as_optional() const
    {
        if(!_node.IsDefined() || _node.IsNull())
        {
            return std::nullopt;
        }
        return _node.as<T>();
    }

    template <typename T>
    T value_or(T &&default_val) const
    {
        if(!_node.IsDefined() || _node.IsNull())
        {
            return std::forward<T>(default_val);
        }
        return _node.as<T>();
    }

    template <typename T>
    std::variant<T, convert_error> try_as() const noexcept
    {
        if(!_node.IsDefined())
        {
            return convert_error::not_defined;
        }
        try
        {
            return _node.as<T>();
        }
        catch(...)
        {
            return convert_error::bad_conversion;
        }
    }

    const_iterator begin() const { return const_iterator(_node.begin()); }
    iterator       begin() { return iterator(_node.begin()); }
    const_iterator end() const { return const_iterator(_node.end()); }
    iterator       end() { return iterator(_node.end()); }

    const_iterator cbegin() const { return const_iterator(_node.begin()); }
    const_iterator cend() const { return const_iterator(_node.end()); }

    void push_back(const yaml &rhs) { _node.push_back(rhs._node); }

    template <typename T>
    void push_back(const T &rhs)
    {
        _node.push_back(rhs);
    }

    template <typename Key, typename Value>
    void insert_or_assign(Key &&key, Value &&value)
    {
        _node[std::forward<Key>(key)] = std::forward<Value>(value);
    }

    template <typename Key, typename Value>
    void insert(Key &&key, Value &&value)
    {
        _node.force_insert(std::forward<Key>(key), std::forward<Value>(value));
    }

    template <typename Key, typename Value>
    [[deprecated(
        "Use insert_or_assign() instead to avoid leaky backend abstraction.")]]
    void force_insert(const Key &key, const Value &value)
    {
        _node.force_insert(key, value);
    }

    bool dump(std::ostream &os) const
    {
        if(!os.good())
            return false;

        os << _node;
        return os.good();
    }

    bool dump(char *buf, size_t &size) const
    {
        std::string yaml_str = str();
        size_t      needed   = yaml_str.size();

        if(!buf || size <= needed)
        {
            size = needed;
            return false;
        }

        std::memcpy(buf, yaml_str.c_str(), needed);
        buf[needed] = '\0';
        size        = needed;
        return true;
    }

  private:
    explicit yaml(YAML::Node node)
        : _node(std::move(node))
    {
    }

  private:
    YAML::Node _node;
};

struct yaml::arrow_proxy
{
    yaml        node;
    const yaml *operator->() const noexcept { return &node; }
};

template <bool IsConst>
inline typename yaml::iterator_wrapper<IsConst>::reference
yaml::iterator_wrapper<IsConst>::operator*() const
{
    return yaml(*_iter);
}

template <bool IsConst>
inline typename yaml::iterator_wrapper<IsConst>::pointer
yaml::iterator_wrapper<IsConst>::operator->() const
{
    return arrow_proxy{yaml(*_iter)};
}

template <bool IsConst>
inline yaml yaml::iterator_wrapper<IsConst>::key() const
{
    return yaml(_iter->first);
}

template <bool IsConst>
inline yaml yaml::iterator_wrapper<IsConst>::value() const
{
    return yaml(_iter->second);
}

} // namespace hj

#endif // YAML_HPP