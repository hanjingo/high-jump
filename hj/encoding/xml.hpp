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

#include <string>
#include <string_view>
#include <fstream>
#include <iosfwd>
#include <pugixml.hpp>

namespace hj::xml
{

enum class format_flags : unsigned int
{
    indent                 = pugi::format_indent,
    write_bom              = pugi::format_write_bom,
    raw                    = pugi::format_raw,
    no_declaration         = pugi::format_no_declaration,
    no_escapes             = pugi::format_no_escapes,
    save_file_text         = pugi::format_save_file_text,
    indent_attributes      = pugi::format_indent_attributes,
    no_empty_element_tags  = pugi::format_no_empty_element_tags,
    skip_control_chars     = pugi::format_skip_control_chars,
    attribute_single_quote = pugi::format_attribute_single_quote,
    by_default             = pugi::format_default
};

enum class encoding : unsigned int
{
    by_auto  = pugi::encoding_auto,
    utf8     = pugi::encoding_utf8,
    utf16_le = pugi::encoding_utf16_le,
    utf16_be = pugi::encoding_utf16_be,
    utf16    = pugi::encoding_utf16,
    utf32_le = pugi::encoding_utf32_le,
    utf32_be = pugi::encoding_utf32_be,
    utf32    = pugi::encoding_utf32,
    wchar_   = pugi::encoding_wchar,
    latin1   = pugi::encoding_latin1
};

enum class parse_options : unsigned int
{
    minimal          = pugi::parse_minimal,
    pi               = pugi::parse_pi,
    comments         = pugi::parse_comments,
    cdata            = pugi::parse_cdata,
    ws_pcdata        = pugi::parse_ws_pcdata,
    escapes          = pugi::parse_escapes,
    eol              = pugi::parse_eol,
    wconv_attribute  = pugi::parse_wconv_attribute,
    wnorm_attribute  = pugi::parse_wnorm_attribute,
    declaration      = pugi::parse_declaration,
    doctype          = pugi::parse_doctype,
    ws_pcdata_single = pugi::parse_ws_pcdata_single,
    trim_pcdata      = pugi::parse_trim_pcdata,
    fragment         = pugi::parse_fragment,
    embed_pcdata     = pugi::parse_embed_pcdata,
    merge_pcdata     = pugi::parse_merge_pcdata,

    by_default = pugi::parse_default,
    full       = pugi::parse_full
};

struct parse_result
{
    pugi::xml_parse_result internal_result;

    constexpr explicit operator bool() const noexcept
    {
        return internal_result.status == pugi::status_ok;
    }

    [[nodiscard]] const char *description() const noexcept
    {
        return internal_result.description();
    }

    [[nodiscard]] std::size_t offset() const noexcept
    {
        return static_cast<std::size_t>(internal_result.offset);
    }
};

class node
{
  public:
    node() noexcept = default;
    node(pugi::xml_node n) noexcept
        : _node(n)
    {
    }

    explicit           operator bool() const noexcept { return !_node.empty(); }
    [[nodiscard]] bool empty() const noexcept { return _node.empty(); }

    [[nodiscard]] node child(const char *name) const noexcept
    {
        return node(_node.child(name));
    }

    node append_child(const char *name)
    {
        return node(_node.append_child(name));
    }

    bool remove_child(const char *name) { return _node.remove_child(name); }

    bool remove_child(const node &child_node)
    {
        return _node.remove_child(child_node._node);
    }

    [[nodiscard]] std::string_view child_value(const char *name) const noexcept
    {
        return _node.child_value(name);
    }

    void set_value(const char *val) { _node.text().set(val); }

    [[nodiscard]] std::string_view name() const noexcept
    {
        return _node.name();
    }

    void set_name(const char *name) { _node.set_name(name); }

    [[nodiscard]] std::string_view value() const noexcept
    {
        return _node.text().get();
    }

    [[nodiscard]] std::string_view attr(const char *name) const noexcept
    {
        return _node.attribute(name).value();
    }

    void set_attr(const char *name, const char *val)
    {
        auto attribute = _node.attribute(name);
        if(attribute)
        {
            attribute.set_value(val);
        } else
        {
            _node.append_attribute(name).set_value(val);
        }
    }

    [[nodiscard]] pugi::xml_node raw_node() const noexcept { return _node; }

  protected:
    pugi::xml_node _node;
};

class document : public node
{
  public:
    document() { _node = _doc; }

    ~document() = default;

    document(const document &)            = delete;
    document &operator=(const document &) = delete;

    document(document &&rhs) noexcept
        : node()
        , _doc(std::move(rhs._doc))
    {
        _node     = _doc;
        rhs._node = pugi::xml_node();
    }

    document &operator=(document &&rhs) noexcept
    {
        if(this != &rhs)
        {
            _doc      = std::move(rhs._doc);
            _node     = _doc;
            rhs._node = pugi::xml_node();
        }
        return *this;
    }

    parse_result
    load_string(const char         *text,
                const parse_options parse = parse_options::by_default)
    {
        auto res = _doc.load_string(text, static_cast<unsigned int>(parse));
        _node    = _doc;
        return parse_result{res};
    }

    parse_result load(const char         *text,
                      const parse_options parse = parse_options::by_default)
    {
        return load_string(text, parse);
    }

    parse_result load(std::string_view    text,
                      const parse_options parse = parse_options::by_default)
    {
        auto res = _doc.load_buffer(text.data(),
                                    text.size(),
                                    static_cast<unsigned int>(parse));
        _node    = _doc;
        return parse_result{res};
    }

    parse_result load(std::istream       &in,
                      const parse_options parse = parse_options::by_default,
                      const encoding      enc   = encoding::by_auto)
    {
        auto res = _doc.load(in,
                             static_cast<unsigned int>(parse),
                             static_cast<pugi::xml_encoding>(enc));
        _node    = _doc;
        return parse_result{res};
    }

    parse_result
    load_file(const char         *filepath,
              const parse_options parse = parse_options::by_default,
              const encoding      enc   = encoding::by_auto)
    {
        auto res = _doc.load_file(filepath,
                                  static_cast<unsigned int>(parse),
                                  static_cast<pugi::xml_encoding>(enc));
        _node    = _doc;
        return parse_result{res};
    }

    bool save(std::ostream      &out,
              const char        *indent = "\t",
              const format_flags flags  = format_flags::by_default,
              const encoding     enc    = encoding::by_auto) const
    {
        _doc.save(out,
                  indent,
                  static_cast<unsigned int>(flags),
                  static_cast<pugi::xml_encoding>(enc));
        return out.good();
    }

    bool save_file(const char        *filepath,
                   const char        *indent = "\t",
                   const format_flags flags  = format_flags::by_default,
                   const encoding     enc    = encoding::by_auto) const
    {
        return _doc.save_file(filepath,
                              indent,
                              static_cast<unsigned int>(flags),
                              static_cast<pugi::xml_encoding>(enc));
    }

    [[nodiscard]] std::string
    str(const char        *indent = "\t",
        const format_flags flags  = format_flags::by_default) const
    {
        struct string_writer : pugi::xml_writer
        {
            std::string result;
            void        write(const void *data, size_t size) override
            {
                result.append(static_cast<const char *>(data), size);
            }
        } writer;

        _doc.save(writer, indent, static_cast<unsigned int>(flags));
        return std::move(writer.result);
    }

    [[nodiscard]] node root() const noexcept
    {
        return node(_doc.document_element());
    }

    [[nodiscard]] node document_element() const noexcept { return root(); }

  private:
    pugi::xml_document _doc;
};

} // namespace hj::xml

#endif