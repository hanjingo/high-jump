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
#include <string>
#include <string_view>
#include <utility>

namespace hj
{

class yaml
{
  public:
    template <bool IsConst>
    class iterator_wrapper
    {
      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = yaml;
        using difference_type   = std::ptrdiff_t;
        using pointer           = std::unique_ptr<yaml>;
        using reference         = yaml;

        using BaseIter =
            std::conditional_t<IsConst, YAML::const_iterator, YAML::iterator>;

        iterator_wrapper() = default;
        explicit iterator_wrapper(BaseIter iter)
            : iter_(std::move(iter))
        {
        }

        reference operator*() const { return yaml(*iter_); }

        pointer operator->() const
        {
            return std::unique_ptr<yaml>(new yaml(*iter_));
        }

        iterator_wrapper &operator++()
        {
            ++iter_;
            return *this;
        }

        iterator_wrapper operator++(int)
        {
            iterator_wrapper tmp = *this;
            ++iter_;
            return tmp;
        }

        bool operator==(const iterator_wrapper &rhs) const
        {
            return iter_ == rhs.iter_;
        }
        bool operator!=(const iterator_wrapper &rhs) const
        {
            return iter_ != rhs.iter_;
        }

        yaml key() const { return yaml(iter_->first); }
        yaml value() const { return yaml(iter_->second); }

      private:
        BaseIter iter_;
    };

    using iterator       = iterator_wrapper<false>;
    using const_iterator = iterator_wrapper<true>;

  public:
    yaml()
        : node_(YAML::NodeType::Undefined)
    {
    }

    yaml(const yaml &rhs)                = default;
    yaml(yaml &&rhs) noexcept            = default;
    yaml &operator=(const yaml &rhs)     = default;
    yaml &operator=(yaml &&rhs) noexcept = default;

    ~yaml() = default;

    yaml clone() const { return yaml(YAML::Clone(node_)); }

    template <typename T>
    yaml &operator=(const T &rhs)
    {
        node_ = rhs;
        return *this;
    }

    static yaml load_from_string(std::string_view text) noexcept
    {
        try
        {
            return yaml(YAML::Load(text.data()));
        }
        catch(...)
        {
            return yaml();
        }
    }

    static yaml load_from_file(const std::filesystem::path &file_path) noexcept
    {
        std::error_code ec;
        if(!std::filesystem::exists(file_path, ec)
           || !std::filesystem::is_regular_file(file_path, ec))
            return yaml();

        try
        {
            return yaml(YAML::LoadFile(file_path.string()));
        }
        catch(...)
        {
            return yaml();
        }
    }

    static yaml load_from_stream(std::istream &in) noexcept
    {
        try
        {
            return yaml(YAML::Load(in));
        }
        catch(...)
        {
            return yaml();
        }
    }

    static yaml load(const char *text) noexcept
    {
        return load_from_string(text ? text : "");
    }

    explicit operator bool() const
    {
        return node_.IsDefined() && !node_.IsNull();
    }
    bool operator!() const { return !node_.IsDefined() || node_.IsNull(); }

    bool is_null() const { return node_.IsNull(); }
    bool is_defined() const { return node_.IsDefined(); }
    bool is_scalar() const { return node_.IsScalar(); }
    bool is_sequence() const { return node_.IsSequence(); }
    bool is_map() const { return node_.IsMap(); }

    template <typename Key>
    yaml operator[](Key &&key) const
    {
        return yaml(node_[std::forward<Key>(key)]);
    }

    template <typename Key>
    yaml operator[](Key &&key)
    {
        return yaml(node_[std::forward<Key>(key)]);
    }

    std::string tag() const { return node_.Tag(); }
    void set_tag(std::string_view tag) { node_.SetTag(std::string(tag)); }

    std::string scalar() const { return node_.Scalar(); }
    std::string str() const { return YAML::Dump(node_); }

    template <typename T>
    std::optional<T> as_optional() const noexcept
    {
        try
        {
            if(!node_.IsDefined())
                return std::nullopt;
            return node_.as<T>();
        }
        catch(...)
        {
            return std::nullopt;
        }
    }

    template <typename T>
    T as() const
    {
        return node_.as<T>();
    }

    template <typename T>
    T value_or(T &&default_val) const noexcept
    {
        auto opt = as_optional<std::decay_t<T>>();
        return opt.has_value() ? *opt : std::forward<T>(default_val);
    }

    const_iterator begin() const { return const_iterator(node_.begin()); }
    iterator       begin() { return iterator(node_.begin()); }
    const_iterator end() const { return const_iterator(node_.end()); }
    iterator       end() { return iterator(node_.end()); }

    void push_back(const yaml &rhs) { node_.push_back(rhs.node_); }

    template <typename T>
    void push_back(const T &rhs)
    {
        node_.push_back(rhs);
    }

    template <typename Key, typename Value>
    void force_insert(const Key &key, const Value &value)
    {
        node_.force_insert(key, value);
    }

    bool dump(std::ostream &os) const
    {
        if(!os.good())
            return false;

        os << node_;
        return os.good();
    }

    bool dump(char *buf, size_t &size) const
    {
        if(!buf || size == 0)
            return false;

        std::string yaml_str = str();
        if(yaml_str.size() + 1 > size)
            return false;

        std::memcpy(buf, yaml_str.c_str(), yaml_str.size());
        buf[yaml_str.size()] = '\0';
        size                 = yaml_str.size();
        return true;
    }

  private:
    explicit yaml(YAML::Node node)
        : node_(std::move(node))
    {
    }

  private:
    YAML::Node node_;
};

} // namespace hj

#endif // YAML_HPP