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

#ifndef XML_HPP
#define XML_HPP

#include <string>
#include <sstream>
#include <fstream>
#include <pugixml.hpp>

namespace hj
{

class xml
{
  public:
    enum class format_flags
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

    enum class encoding
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

    enum class parse_options
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

    xml()
        : _doc(std::make_shared<pugi::xml_document>())
        , _node(*_doc)
    {
    }

    xml(const xml &rhs)
        : _doc(rhs._doc)
        , _node(rhs._node)
    {
    }
    ~xml() = default;

    inline xml child(const char *name) const
    {
        return xml(_doc, _node.child(name));
    }

    inline xml append_child(const char *name)
    {
        return xml(_doc, _node.append_child(name));
    }

    inline bool remove_child(const char *name)
    {
        return _node.remove_child(name);
    }

    inline std::string child_value(const char *name) const
    {
        return _node.child_value(name);
    }

    inline void set_value(const char *value) { _node.text().set(value); }

    inline std::string name() const { return _node.name(); }

    inline void set_name(const char *name) { _node.set_name(name); }

    inline std::string value() const { return _node.text().get(); }

    inline bool empty() const { return _node.empty(); }

    inline std::string attr(const char *name) const
    {
        return _node.attribute(name).value();
    }

    inline void set_attr(const char *name, const char *value)
    {
        _node.append_attribute(name).set_value(value);
    }

    bool load(const char         *text,
              const parse_options parse = parse_options::by_default)
    {
        pugi::xml_parse_result r =
            _doc->load_string(text, static_cast<unsigned int>(parse));
        _node = _doc->first_child();
        return r;
    }

    bool load(std::ifstream      &in,
              const parse_options parse    = parse_options::by_default,
              const encoding      encoding = encoding::by_auto)
    {
        pugi::xml_parse_result r =
            _doc->load(in,
                       static_cast<unsigned int>(parse),
                       static_cast<pugi::xml_encoding>(encoding));
        _node = *_doc;
        return r;
    }

    bool load_file(const char         *filepath,
                   const parse_options parse    = parse_options::by_default,
                   const encoding      encoding = encoding::by_auto)
    {
        pugi::xml_parse_result r =
            _doc->load_file(filepath,
                            static_cast<unsigned int>(parse),
                            static_cast<pugi::xml_encoding>(encoding));

        _node = _doc->first_child();
        return r;
    }

    bool save(std::ofstream     &out,
              const char        *indent   = "\t",
              const format_flags flags    = format_flags::by_default,
              const encoding     encoding = encoding::by_auto) const
    {
        if(!out.is_open())
            return false;

        _doc->save(out,
                   indent,
                   static_cast<unsigned int>(flags),
                   static_cast<pugi::xml_encoding>(encoding));
        return true;
    }

    bool save_file(const char        *filepath,
                   const char        *indent   = "\t",
                   const format_flags flags    = format_flags::by_default,
                   const encoding     encoding = encoding::by_auto) const
    {
        return _doc->save_file(filepath,
                               indent,
                               static_cast<unsigned int>(flags),
                               static_cast<pugi::xml_encoding>(encoding));
    }

    std::string str() const
    {
        std::ostringstream oss;
        _doc->save(oss);
        return oss.str();
    }

  private:
    xml(std::shared_ptr<pugi::xml_document> doc, pugi::xml_node node)
        : _doc(std::move(doc))
        , _node(node)
    {
    }

  private:
    std::shared_ptr<pugi::xml_document> _doc;
    pugi::xml_node                      _node;
};

} // namespace hj

#endif