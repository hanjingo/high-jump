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

#ifndef XML_HPP
#define XML_HPP

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>

namespace hj::xml
{

template <typename Enum>
constexpr std::underlying_type_t<Enum> to_underlying(Enum e) noexcept
{
    return static_cast<std::underlying_type_t<Enum>>(e);
}

namespace detail
{
template <typename T>
struct value_converter;

template <typename T>
struct integer_converter
{
    static std::optional<T> convert(std::string_view sv) noexcept
    {
        if(sv.empty())
        {
            return std::nullopt;
        }
        T    val{};
        auto res = std::from_chars(sv.data(), sv.data() + sv.size(), val);
        if(res.ec == std::errc{} && res.ptr == sv.data() + sv.size())
        {
            return val;
        }
        return std::nullopt;
    }
};

template <>
struct value_converter<int> : integer_converter<int>
{
};
template <>
struct value_converter<long> : integer_converter<long>
{
};
template <>
struct value_converter<long long> : integer_converter<long long>
{
};
template <>
struct value_converter<unsigned int> : integer_converter<unsigned int>
{
};
template <>
struct value_converter<unsigned long> : integer_converter<unsigned long>
{
};
template <>
struct value_converter<unsigned long long>
    : integer_converter<unsigned long long>
{
};

template <>
struct value_converter<float>
{
    static std::optional<float> convert(std::string_view sv) noexcept
    {
        if(sv.empty())
            return std::nullopt;
        float val{};
        auto  res = std::from_chars(sv.data(), sv.data() + sv.size(), val);
        if(res.ec == std::errc{} && res.ptr == sv.data() + sv.size())
            return val;
        return std::nullopt;
    }
};

template <>
struct value_converter<double>
{
    static std::optional<double> convert(std::string_view sv) noexcept
    {
        if(sv.empty())
            return std::nullopt;
        double val{};
        auto   res = std::from_chars(sv.data(), sv.data() + sv.size(), val);
        if(res.ec == std::errc{} && res.ptr == sv.data() + sv.size())
            return val;
        return std::nullopt;
    }
};

template <>
struct value_converter<bool>
{
    static std::optional<bool> convert(std::string_view sv) noexcept
    {
        if(sv == "true" || sv == "1" || sv == "yes" || sv == "on")
            return true;
        if(sv == "false" || sv == "0" || sv == "no" || sv == "off")
            return false;
        return std::nullopt;
    }
};

template <>
struct value_converter<std::string_view>
{
    static std::optional<std::string_view> convert(std::string_view sv) noexcept
    {
        return sv;
    }
};

template <>
struct value_converter<std::string>
{
    static std::optional<std::string> convert(std::string_view sv) noexcept
    {
        return std::string(sv);
    }
};
} // namespace detail

enum class format_flags : unsigned int
{
    indent                 = 1U << 0,
    write_bom              = 1U << 1,
    raw                    = 1U << 2,
    no_declaration         = 1U << 3,
    no_escapes             = 1U << 4,
    save_file_text         = 1U << 5,
    indent_attributes      = 1U << 6,
    no_empty_element_tags  = 1U << 7,
    skip_control_chars     = 1U << 8,
    attribute_single_quote = 1U << 9,
    by_default             = indent
};

enum class encoding : unsigned int
{
    by_auto  = 0,
    utf8     = 1,
    utf16_le = 2,
    utf16_be = 3,
    utf16    = 4,
    utf32_le = 5,
    utf32_be = 6,
    utf32    = 7,
    wchar_   = 8,
    latin1   = 9
};

enum class parse_options : unsigned int
{
    minimal          = 0x0000,
    pi               = 0x0001,
    comments         = 0x0002,
    cdata            = 0x0004,
    ws_pcdata        = 0x0008,
    escapes          = 0x0010,
    eol              = 0x0020,
    wconv_attribute  = 0x0040,
    wnorm_attribute  = 0x0080,
    declaration      = 0x0100,
    doctype          = 0x0200,
    ws_pcdata_single = 0x0400,
    trim_pcdata      = 0x0800,
    fragment         = 0x1000,
    embed_pcdata     = 0x2000,
    merge_pcdata     = 0x4000,

    by_default = escapes | wnorm_attribute | declaration | doctype | pi
                 | comments | cdata,
    full       = by_default | ws_pcdata
};

enum class node_type
{
    null        = 0,
    document    = 1,
    element     = 2,
    pcdata      = 3,
    cdata       = 4,
    comment     = 5,
    pi          = 6,
    declaration = 7,
    doctype     = 8
};

#define HJ_XML_ENABLE_BITMASK_OPERATORS(Enum)                                  \
    constexpr inline Enum operator|(Enum lhs, Enum rhs) noexcept               \
    {                                                                          \
        return static_cast<Enum>(to_underlying(lhs) | to_underlying(rhs));     \
    }                                                                          \
    constexpr inline Enum operator&(Enum lhs, Enum rhs) noexcept               \
    {                                                                          \
        return static_cast<Enum>(to_underlying(lhs) & to_underlying(rhs));     \
    }                                                                          \
    constexpr inline Enum operator^(Enum lhs, Enum rhs) noexcept               \
    {                                                                          \
        return static_cast<Enum>(to_underlying(lhs) ^ to_underlying(rhs));     \
    }                                                                          \
    constexpr inline Enum operator~(Enum val) noexcept                         \
    {                                                                          \
        return static_cast<Enum>(~to_underlying(val));                         \
    }                                                                          \
    constexpr inline Enum &operator|=(Enum &lhs, Enum rhs) noexcept            \
    {                                                                          \
        lhs = lhs | rhs;                                                       \
        return lhs;                                                            \
    }                                                                          \
    constexpr inline Enum &operator&=(Enum &lhs, Enum rhs) noexcept            \
    {                                                                          \
        lhs = lhs & rhs;                                                       \
        return lhs;                                                            \
    }                                                                          \
    constexpr inline Enum &operator^=(Enum &lhs, Enum rhs) noexcept            \
    {                                                                          \
        lhs = lhs ^ rhs;                                                       \
        return lhs;                                                            \
    }

HJ_XML_ENABLE_BITMASK_OPERATORS(format_flags)
HJ_XML_ENABLE_BITMASK_OPERATORS(parse_options)
#undef HJ_XML_ENABLE_BITMASK_OPERATORS

enum class parse_status
{
    ok                   = 0,
    file_not_found       = 1,
    io_error             = 2,
    out_of_memory        = 3,
    internal_error       = 4,
    unrecognized_tag     = 5,
    bad_pi               = 6,
    bad_comment          = 7,
    bad_cdata            = 8,
    bad_doctype          = 9,
    bad_pcdata           = 10,
    bad_start_element    = 11,
    bad_attribute        = 12,
    bad_end_element      = 13,
    end_element_mismatch = 14,
    append_invalid_root  = 15,
    no_document_element  = 16
};

struct parse_result
{
    parse_status _status{parse_status::ok};
    const char  *_description{"No error"};
    std::size_t  _offset{0};

    constexpr explicit operator bool() const noexcept
    {
        return _status == parse_status::ok;
    }

    [[nodiscard]] constexpr parse_status status() const noexcept
    {
        return _status;
    }
    [[nodiscard]] const char *description() const noexcept
    {
        return _description;
    }
    [[nodiscard]] std::size_t offset() const noexcept { return _offset; }
};

class text
{
  public:
    text() noexcept = default;
    explicit text(void *opaque_handle) noexcept
        : _handle(opaque_handle)
    {
    }

    explicit           operator bool() const noexcept;
    [[nodiscard]] bool empty() const noexcept;

    [[nodiscard]] std::string_view get() const noexcept;
    bool                           set(const char *val);

    template <typename T>
    [[nodiscard]] std::optional<T> as() const noexcept
    {
        if(empty())
            return std::nullopt;
        return detail::value_converter<T>::convert(get());
    }

    [[nodiscard]] int as_int(int default_val = 0) const noexcept;
    [[nodiscard]] unsigned int
                         as_uint(unsigned int default_val = 0) const noexcept;
    [[nodiscard]] double as_double(double default_val = 0.0) const noexcept;
    [[nodiscard]] float  as_float(float default_val = 0.0f) const noexcept;
    [[nodiscard]] bool   as_bool(bool default_val = false) const noexcept;

    void *internal_handle() const noexcept { return _handle; }

  private:
    void *_handle{nullptr};
};

class attribute
{
  public:
    attribute() noexcept = default;
    explicit attribute(void *opaque_handle) noexcept
        : _handle(opaque_handle)
    {
    }

    explicit           operator bool() const noexcept;
    [[nodiscard]] bool empty() const noexcept;

    [[nodiscard]] std::string_view name() const noexcept;
    [[nodiscard]] std::string_view value() const noexcept;

    template <typename T>
    [[nodiscard]] std::optional<T> as() const noexcept
    {
        if(empty())
            return std::nullopt;
        return detail::value_converter<T>::convert(value());
    }

    [[nodiscard]] int as_int(int default_val = 0) const noexcept;
    [[nodiscard]] unsigned int
                         as_uint(unsigned int default_val = 0) const noexcept;
    [[nodiscard]] double as_double(double default_val = 0.0) const noexcept;
    [[nodiscard]] bool   as_bool(bool default_val = false) const noexcept;

    bool set_name(const char *name);
    bool set_value(const char *val);

    [[nodiscard]] attribute next_attribute() const noexcept;
    [[nodiscard]] attribute previous_attribute() const noexcept;

    void *internal_handle() const noexcept { return _handle; }

  private:
    void *_handle{nullptr};
};

class attribute_iterator
{
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type        = attribute;
    using difference_type   = std::ptrdiff_t;
    using pointer           = attribute *;
    using reference         = attribute;

    attribute_iterator() noexcept = default;
    explicit attribute_iterator(void *handle) noexcept
        : _handle(handle)
    {
    }

    reference operator*() const noexcept { return attribute(_handle); }
    attribute_iterator &operator++() noexcept;
    attribute_iterator  operator++(int) noexcept
    {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    bool operator==(const attribute_iterator &rhs) const noexcept
    {
        return _handle == rhs._handle;
    }
    bool operator!=(const attribute_iterator &rhs) const noexcept
    {
        return _handle != rhs._handle;
    }

  private:
    void *_handle{nullptr};
};

struct attribute_range
{
    attribute_iterator _begin;
    attribute_iterator _end;

    [[nodiscard]] attribute_iterator begin() const noexcept { return _begin; }
    [[nodiscard]] attribute_iterator end() const noexcept { return _end; }
};

class node;

class node_iterator
{
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type        = node;
    using difference_type   = std::ptrdiff_t;
    using pointer           = node *;
    using reference         = node;

    node_iterator() noexcept = default;
    node_iterator(void *handle, const char *filter_name = nullptr) noexcept
        : _handle(handle)
        , _filter_name(filter_name)
    {
    }

    reference      operator*() const noexcept;
    node_iterator &operator++() noexcept;
    node_iterator  operator++(int) noexcept
    {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    bool operator==(const node_iterator &rhs) const noexcept
    {
        return _handle == rhs._handle;
    }
    bool operator!=(const node_iterator &rhs) const noexcept
    {
        return _handle != rhs._handle;
    }

  private:
    void       *_handle{nullptr};
    const char *_filter_name{nullptr};
};

struct node_range
{
    node_iterator _begin;
    node_iterator _end;

    [[nodiscard]] node_iterator begin() const noexcept { return _begin; }
    [[nodiscard]] node_iterator end() const noexcept { return _end; }
};

class node
{
  public:
    node() noexcept = default;
    explicit node(void *opaque_handle) noexcept
        : _handle(opaque_handle)
    {
    }

    explicit           operator bool() const noexcept;
    [[nodiscard]] bool empty() const noexcept;

    [[nodiscard]] node_type type() const noexcept;

    [[nodiscard]] std::string_view name() const noexcept;
    bool                           set_name(const char *name);

    [[nodiscard]] std::string_view value() const noexcept;
    bool                           set_value(const char *val);

    template <typename T>
    [[nodiscard]] std::optional<T> value_as() const noexcept
    {
        return text().as<T>();
    }

    [[nodiscard]] hj::xml::text text() const noexcept;
    bool                        set_text(const char *val);

    [[nodiscard]] node parent() const noexcept;
    [[nodiscard]] node first_child() const noexcept;
    [[nodiscard]] node last_child() const noexcept;

    [[nodiscard]] node next_sibling() const noexcept;
    [[nodiscard]] node next_sibling(const char *name) const noexcept;

    [[nodiscard]] node previous_sibling() const noexcept;
    [[nodiscard]] node previous_sibling(const char *name) const noexcept;

    [[nodiscard]] node child(const char *name) const noexcept;
    node               append_child(const char *name);
    bool               remove_child(const char *name);
    bool               remove_child(const node &child_node);

    [[nodiscard]] std::string_view child_value(const char *name) const noexcept;

    [[nodiscard]] attribute first_attribute() const noexcept;
    [[nodiscard]] attribute last_attribute() const noexcept;
    [[nodiscard]] attribute attribute_node(const char *name) const noexcept;

    [[nodiscard]] std::string_view attr(const char *name) const noexcept;

    template <typename T>
    [[nodiscard]] std::optional<T> attr_as(const char *name) const noexcept
    {
        auto a = attribute_node(name);
        if(!a)
            return std::nullopt;
        return a.as<T>();
    }

    bool set_attr(const char *name, const char *val);
    bool remove_attribute(const char *name);
    bool remove_attribute(const attribute &attr);

    [[nodiscard]] node_range children() const noexcept;
    [[nodiscard]] node_range children(const char *filter_name) const noexcept;
    [[nodiscard]] attribute_range attributes() const noexcept;

    void *internal_handle() const noexcept { return _handle; }

  private:
    void *_handle{nullptr};
};

inline node node_iterator::operator*() const noexcept
{
    return node(_handle);
}

class document
{
  public:
    document();
    ~document();

    document(const document &)            = delete;
    document &operator=(const document &) = delete;

    document(document &&rhs) noexcept;
    document &operator=(document &&rhs) noexcept;

    explicit           operator bool() const noexcept;
    [[nodiscard]] bool empty() const noexcept;

    [[nodiscard]] node root() const noexcept;
    [[nodiscard]] node document_element() const noexcept { return root(); }

    parse_result
    load_string(const char         *text,
                const parse_options parse = parse_options::by_default);
    parse_result load(const char         *text,
                      const parse_options parse = parse_options::by_default);
    parse_result load(std::string_view    text,
                      const parse_options parse = parse_options::by_default,
                      const encoding      enc   = encoding::by_auto);
    parse_result load(std::istream       &in,
                      const parse_options parse = parse_options::by_default,
                      const encoding      enc   = encoding::by_auto);
    parse_result
    load_file(const char         *filepath,
              const parse_options parse = parse_options::by_default,
              const encoding      enc   = encoding::by_auto);

    bool save(std::ostream      &out,
              const char        *indent = "\t",
              const format_flags flags  = format_flags::by_default,
              const encoding     enc    = encoding::by_auto) const;
    bool save_file(const char        *filepath,
                   const char        *indent = "\t",
                   const format_flags flags  = format_flags::by_default,
                   const encoding     enc    = encoding::by_auto) const;

    [[nodiscard]] std::string
    str(const char        *indent = "\t",
        const format_flags flags  = format_flags::by_default) const;

  private:
    struct impl;
    std::unique_ptr<impl> _pimpl;
};

} // namespace hj::xml


#include <pugixml.hpp>

namespace hj::xml
{

namespace detail
{

inline unsigned int to_pugi_format_flags(format_flags flags) noexcept
{
    unsigned int res = 0;
    auto         val = to_underlying(flags);
    if(val & to_underlying(format_flags::indent))
        res |= pugi::format_indent;
    if(val & to_underlying(format_flags::write_bom))
        res |= pugi::format_write_bom;
    if(val & to_underlying(format_flags::raw))
        res |= pugi::format_raw;
    if(val & to_underlying(format_flags::no_declaration))
        res |= pugi::format_no_declaration;
    if(val & to_underlying(format_flags::no_escapes))
        res |= pugi::format_no_escapes;
    if(val & to_underlying(format_flags::save_file_text))
        res |= pugi::format_save_file_text;
    if(val & to_underlying(format_flags::indent_attributes))
        res |= pugi::format_indent_attributes;
    if(val & to_underlying(format_flags::no_empty_element_tags))
        res |= pugi::format_no_empty_element_tags;
    if(val & to_underlying(format_flags::skip_control_chars))
        res |= pugi::format_skip_control_chars;
    if(val & to_underlying(format_flags::attribute_single_quote))
        res |= pugi::format_attribute_single_quote;
    return res;
}

inline unsigned int to_pugi_parse_options(parse_options opts) noexcept
{
    unsigned int res = 0;
    auto         val = to_underlying(opts);
    if(val & to_underlying(parse_options::pi))
        res |= pugi::parse_pi;
    if(val & to_underlying(parse_options::comments))
        res |= pugi::parse_comments;
    if(val & to_underlying(parse_options::cdata))
        res |= pugi::parse_cdata;
    if(val & to_underlying(parse_options::ws_pcdata))
        res |= pugi::parse_ws_pcdata;
    if(val & to_underlying(parse_options::escapes))
        res |= pugi::parse_escapes;
    if(val & to_underlying(parse_options::eol))
        res |= pugi::parse_eol;
    if(val & to_underlying(parse_options::wconv_attribute))
        res |= pugi::parse_wconv_attribute;
    if(val & to_underlying(parse_options::wnorm_attribute))
        res |= pugi::parse_wnorm_attribute;
    if(val & to_underlying(parse_options::declaration))
        res |= pugi::parse_declaration;
    if(val & to_underlying(parse_options::doctype))
        res |= pugi::parse_doctype;
    if(val & to_underlying(parse_options::ws_pcdata_single))
        res |= pugi::parse_ws_pcdata_single;
    if(val & to_underlying(parse_options::trim_pcdata))
        res |= pugi::parse_trim_pcdata;
    if(val & to_underlying(parse_options::fragment))
        res |= pugi::parse_fragment;
    if(val & to_underlying(parse_options::embed_pcdata))
        res |= pugi::parse_embed_pcdata;
    if(val & to_underlying(parse_options::merge_pcdata))
        res |= pugi::parse_merge_pcdata;
    return res;
}

inline pugi::xml_encoding to_pugi_encoding(encoding enc) noexcept
{
    switch(enc)
    {
        case encoding::utf8:
            return pugi::encoding_utf8;
        case encoding::utf16_le:
            return pugi::encoding_utf16_le;
        case encoding::utf16_be:
            return pugi::encoding_utf16_be;
        case encoding::utf16:
            return pugi::encoding_utf16;
        case encoding::utf32_le:
            return pugi::encoding_utf32_le;
        case encoding::utf32_be:
            return pugi::encoding_utf32_be;
        case encoding::utf32:
            return pugi::encoding_utf32;
        case encoding::wchar_:
            return pugi::encoding_wchar;
        case encoding::latin1:
            return pugi::encoding_latin1;
        default:
            return pugi::encoding_auto;
    }
}

inline parse_result
make_parse_result(const pugi::xml_parse_result &res) noexcept
{
    return parse_result{static_cast<parse_status>(res.status),
                        res.description(),
                        static_cast<std::size_t>(res.offset)};
}

inline pugi::xml_node unwrap(void *h) noexcept
{
    pugi::xml_node n;
    std::memcpy(&n, &h, sizeof(void *));
    return n;
}

inline void *wrap(pugi::xml_node n) noexcept
{
    void *h = nullptr;
    std::memcpy(&h, &n, sizeof(pugi::xml_node));
    return h;
}

inline pugi::xml_attribute unwrap_attr(void *h) noexcept
{
    pugi::xml_attribute a;
    std::memcpy(&a, &h, sizeof(void *));
    return a;
}

inline void *wrap_attr(pugi::xml_attribute a) noexcept
{
    void *h = nullptr;
    std::memcpy(&h, &a, sizeof(pugi::xml_attribute));
    return h;
}

} // namespace detail

inline text::operator bool() const noexcept
{
    return !detail::unwrap_attr(_handle).empty();
}
inline bool text::empty() const noexcept
{
    return detail::unwrap_attr(_handle).empty();
}
inline std::string_view text::get() const noexcept
{
    pugi::xml_text t;
    std::memcpy(&t, &_handle, sizeof(void *));
    return t.get();
}
inline bool text::set(const char *val)
{
    pugi::xml_text t;
    std::memcpy(&t, &_handle, sizeof(void *));
    return t.set(val);
}
inline int text::as_int(int default_val) const noexcept
{
    pugi::xml_text t;
    std::memcpy(&t, &_handle, sizeof(void *));
    return t.as_int(default_val);
}
inline unsigned int text::as_uint(unsigned int default_val) const noexcept
{
    pugi::xml_text t;
    std::memcpy(&t, &_handle, sizeof(void *));
    return t.as_uint(default_val);
}
inline double text::as_double(double default_val) const noexcept
{
    pugi::xml_text t;
    std::memcpy(&t, &_handle, sizeof(void *));
    return t.as_double(default_val);
}
inline float text::as_float(float default_val) const noexcept
{
    pugi::xml_text t;
    std::memcpy(&t, &_handle, sizeof(void *));
    return t.as_float(default_val);
}
inline bool text::as_bool(bool default_val) const noexcept
{
    pugi::xml_text t;
    std::memcpy(&t, &_handle, sizeof(void *));
    return t.as_bool(default_val);
}

inline attribute::operator bool() const noexcept
{
    return !detail::unwrap_attr(_handle).empty();
}
inline bool attribute::empty() const noexcept
{
    return detail::unwrap_attr(_handle).empty();
}
inline std::string_view attribute::name() const noexcept
{
    return detail::unwrap_attr(_handle).name();
}
inline std::string_view attribute::value() const noexcept
{
    return detail::unwrap_attr(_handle).value();
}
inline int attribute::as_int(int default_val) const noexcept
{
    return detail::unwrap_attr(_handle).as_int(default_val);
}
inline unsigned int attribute::as_uint(unsigned int default_val) const noexcept
{
    return detail::unwrap_attr(_handle).as_uint(default_val);
}
inline double attribute::as_double(double default_val) const noexcept
{
    return detail::unwrap_attr(_handle).as_double(default_val);
}
inline bool attribute::as_bool(bool default_val) const noexcept
{
    return detail::unwrap_attr(_handle).as_bool(default_val);
}
inline bool attribute::set_name(const char *name)
{
    auto a = detail::unwrap_attr(_handle);
    return a.set_name(name);
}
inline bool attribute::set_value(const char *val)
{
    auto a = detail::unwrap_attr(_handle);
    return a.set_value(val);
}
inline attribute attribute::next_attribute() const noexcept
{
    return attribute(
        detail::wrap_attr(detail::unwrap_attr(_handle).next_attribute()));
}
inline attribute attribute::previous_attribute() const noexcept
{
    return attribute(
        detail::wrap_attr(detail::unwrap_attr(_handle).previous_attribute()));
}

inline attribute_iterator &attribute_iterator::operator++() noexcept
{
    _handle = detail::wrap_attr(detail::unwrap_attr(_handle).next_attribute());
    return *this;
}

inline node::operator bool() const noexcept
{
    return !detail::unwrap(_handle).empty();
}
inline bool node::empty() const noexcept
{
    return detail::unwrap(_handle).empty();
}
inline node_type node::type() const noexcept
{
    return static_cast<node_type>(detail::unwrap(_handle).type());
}
inline std::string_view node::name() const noexcept
{
    return detail::unwrap(_handle).name();
}
inline bool node::set_name(const char *name)
{
    auto n = detail::unwrap(_handle);
    return n.set_name(name);
}
inline std::string_view node::value() const noexcept
{
    return detail::unwrap(_handle).text().get();
}
inline bool node::set_value(const char *val)
{
    auto n = detail::unwrap(_handle);
    return n.text().set(val);
}

inline hj::xml::text node::text() const noexcept
{
    auto  t = detail::unwrap(_handle).text();
    void *h = nullptr;
    std::memcpy(&h, &t, sizeof(pugi::xml_text));
    return hj::xml::text(h);
}

inline bool node::set_text(const char *val)
{
    auto n = detail::unwrap(_handle);
    return n.text().set(val);
}
inline node node::parent() const noexcept
{
    return node(detail::wrap(detail::unwrap(_handle).parent()));
}
inline node node::first_child() const noexcept
{
    return node(detail::wrap(detail::unwrap(_handle).first_child()));
}
inline node node::last_child() const noexcept
{
    return node(detail::wrap(detail::unwrap(_handle).last_child()));
}
inline node node::next_sibling() const noexcept
{
    return node(detail::wrap(detail::unwrap(_handle).next_sibling()));
}
inline node node::next_sibling(const char *name) const noexcept
{
    return node(detail::wrap(detail::unwrap(_handle).next_sibling(name)));
}
inline node node::previous_sibling() const noexcept
{
    return node(detail::wrap(detail::unwrap(_handle).previous_sibling()));
}
inline node node::previous_sibling(const char *name) const noexcept
{
    return node(detail::wrap(detail::unwrap(_handle).previous_sibling(name)));
}
inline node node::child(const char *name) const noexcept
{
    return node(detail::wrap(detail::unwrap(_handle).child(name)));
}
inline node node::append_child(const char *name)
{
    return node(detail::wrap(detail::unwrap(_handle).append_child(name)));
}
inline bool node::remove_child(const char *name)
{
    return detail::unwrap(_handle).remove_child(name);
}
inline bool node::remove_child(const node &child_node)
{
    return detail::unwrap(_handle).remove_child(
        detail::unwrap(child_node._handle));
}
inline std::string_view node::child_value(const char *name) const noexcept
{
    return detail::unwrap(_handle).child_value(name);
}
inline attribute node::first_attribute() const noexcept
{
    return attribute(
        detail::wrap_attr(detail::unwrap(_handle).first_attribute()));
}
inline attribute node::last_attribute() const noexcept
{
    return attribute(
        detail::wrap_attr(detail::unwrap(_handle).last_attribute()));
}
inline attribute node::attribute_node(const char *name) const noexcept
{
    return attribute(
        detail::wrap_attr(detail::unwrap(_handle).attribute(name)));
}
inline std::string_view node::attr(const char *name) const noexcept
{
    return detail::unwrap(_handle).attribute(name).value();
}

inline bool node::set_attr(const char *name, const char *val)
{
    auto n = detail::unwrap(_handle);
    auto a = n.attribute(name);
    if(a)
        return a.set_value(val);
    auto new_attr = n.append_attribute(name);
    if(!new_attr)
        return false;
    return new_attr.set_value(val);
}

inline bool node::remove_attribute(const char *name)
{
    return detail::unwrap(_handle).remove_attribute(name);
}
inline bool node::remove_attribute(const attribute &attr)
{
    return detail::unwrap(_handle).remove_attribute(
        detail::unwrap_attr(attr.internal_handle()));
}

inline node_range node::children() const noexcept
{
    auto n = detail::unwrap(_handle);
    return node_range{node_iterator(detail::wrap(n.first_child())),
                      node_iterator()};
}

inline node_range node::children(const char *filter_name) const noexcept
{
    auto           n     = detail::unwrap(_handle);
    pugi::xml_node start = filter_name ? n.child(filter_name) : n.first_child();
    return node_range{node_iterator(detail::wrap(start), filter_name),
                      node_iterator()};
}

inline attribute_range node::attributes() const noexcept
{
    auto n = detail::unwrap(_handle);
    return attribute_range{
        attribute_iterator(detail::wrap_attr(n.first_attribute())),
        attribute_iterator()};
}

inline node_iterator &node_iterator::operator++() noexcept
{
    auto n = detail::unwrap(_handle);
    if(_filter_name)
    {
        _handle = detail::wrap(n.next_sibling(_filter_name));
    } else
    {
        _handle = detail::wrap(n.next_sibling());
    }
    return *this;
}

struct document::impl
{
    pugi::xml_document doc;
};

inline document::document()
    : _pimpl(std::make_unique<impl>())
{
}
inline document::~document() = default;

inline document::document(document &&rhs) noexcept            = default;
inline document &document::operator=(document &&rhs) noexcept = default;

inline document::operator bool() const noexcept
{
    return _pimpl && !_pimpl->doc.empty();
}
inline bool document::empty() const noexcept
{
    return !_pimpl || _pimpl->doc.empty();
}

inline node document::root() const noexcept
{
    if(!_pimpl)
        return node();
    return node(detail::wrap(_pimpl->doc.document_element()));
}

inline parse_result document::load_string(const char         *text,
                                          const parse_options parse)
{
    if(!_pimpl)
        _pimpl = std::make_unique<impl>();
    return detail::make_parse_result(
        _pimpl->doc.load_string(text, detail::to_pugi_parse_options(parse)));
}

inline parse_result document::load(const char *text, const parse_options parse)
{
    return load_string(text, parse);
}

inline parse_result document::load(std::string_view    text,
                                   const parse_options parse,
                                   const encoding      enc)
{
    if(!_pimpl)
        _pimpl = std::make_unique<impl>();
    return detail::make_parse_result(
        _pimpl->doc.load_buffer(text.data(),
                                text.size(),
                                detail::to_pugi_parse_options(parse),
                                detail::to_pugi_encoding(enc)));
}

inline parse_result
document::load(std::istream &in, const parse_options parse, const encoding enc)
{
    if(!_pimpl)
        _pimpl = std::make_unique<impl>();
    return detail::make_parse_result(
        _pimpl->doc.load(in,
                         detail::to_pugi_parse_options(parse),
                         detail::to_pugi_encoding(enc)));
}

inline parse_result document::load_file(const char         *filepath,
                                        const parse_options parse,
                                        const encoding      enc)
{
    if(!_pimpl)
        _pimpl = std::make_unique<impl>();
    return detail::make_parse_result(
        _pimpl->doc.load_file(filepath,
                              detail::to_pugi_parse_options(parse),
                              detail::to_pugi_encoding(enc)));
}

inline bool document::save(std::ostream      &out,
                           const char        *indent,
                           const format_flags flags,
                           const encoding     enc) const
{
    if(!_pimpl)
        return false;
    _pimpl->doc.save(out,
                     indent,
                     detail::to_pugi_format_flags(flags),
                     detail::to_pugi_encoding(enc));
    return out.good();
}

inline bool document::save_file(const char        *filepath,
                                const char        *indent,
                                const format_flags flags,
                                const encoding     enc) const
{
    if(!_pimpl)
        return false;
    return _pimpl->doc.save_file(filepath,
                                 indent,
                                 detail::to_pugi_format_flags(flags),
                                 detail::to_pugi_encoding(enc));
}

inline std::string document::str(const char        *indent,
                                 const format_flags flags) const
{
    if(!_pimpl)
        return "";
    struct string_writer : pugi::xml_writer
    {
        std::string result;
        void        write(const void *data, size_t size) override
        {
            result.append(static_cast<const char *>(data), size);
        }
    } writer;

    _pimpl->doc.save(writer, indent, detail::to_pugi_format_flags(flags));
    return writer.result;
}

} // namespace hj::xml

#endif