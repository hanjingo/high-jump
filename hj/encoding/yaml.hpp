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
    using iterator = YAML::iterator;

public:
    yaml() : _node() {}
    yaml(const yaml& rhs) : _node(rhs._node) {}
    inline ~yaml() = default;

    template <typename T>
    yaml& operator=(const T& rhs) { _node = rhs; return *this; }
    yaml& operator=(const yaml& rhs) { _node = rhs._node; return *this; }

    explicit operator bool() const { return _node.IsDefined(); }
    bool operator!() const { return !_node.IsDefined(); }
    template <typename Key>
    const yaml operator[](const Key& key) const { return yaml(_node[key]); }
    template <typename Key>
    yaml operator[](const Key& key) { return yaml(_node[key]); }

    inline bool is_null() const { return _node.IsNull(); }
    inline bool is_defined() const { return _node.IsDefined(); }
    inline bool is_scalar() const { return _node.IsScalar(); }
    inline bool is_sequence() const { return _node.IsSequence(); }
    inline bool is_map() const { return _node.IsMap(); }

    inline const std::string& tag() const { return _node.Tag(); }
    inline void set_tag(const std::string& tag) { _node.SetTag(tag); }

    inline const std::string& scalar() const { return _node.Scalar(); }
    inline std::string str() const { return YAML::Dump(_node); }
    template<typename T>
    inline T as() const { return _node.as<T>(); }

    inline const_iterator begin() const { return _node.begin(); }
    inline iterator begin() { return _node.begin(); }
    inline const_iterator end() const { return _node.end(); }
    inline iterator end() { return _node.end(); }

    inline void push_back(const yaml& rhs) { _node.push_back(rhs._node); }
    template <typename Key, typename Value>
    inline void force_insert(const Key& key, const Value& value) { _node.force_insert(key, value); }
    template <typename T>
    inline void push_back(const T& rhs) { _node.push_back(rhs); }

    static yaml load(const char* text) 
    {
        std::stringstream ss(text);
        return yaml(YAML::Load(ss));
    }

    yaml load(std::ifstream& fin) 
    {
        if (!fin.is_open()) 
            return yaml();

        _node = YAML::Load(fin);
        return *this;
    }

    bool dump(char* buf, size_t& size) const
    {
        if (!buf || size == 0) 
            return false;

        std::string yaml_str = YAML::Dump(_node);
        size = yaml_str.size();
        if (size > 0)
        {
            memcpy(buf, yaml_str.c_str(), size);
            buf[size] = '\0';  // Null-terminate the string
        }
        return true;
    }

    bool dump(std::ofstream& fout) const
    {
        if (!fout.is_open()) 
            return false;

        fout << YAML::Dump(_node);
        return true;
    }

private:
    yaml(const YAML::Node& node) : _node(node) {}
    yaml& operator=(const YAML::Node& rhs) { _node = rhs; return *this; }

private:
    YAML::Node _node;
};

}

#endif // YAML_HPP