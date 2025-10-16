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

#ifndef PDF_HPP
#define PDF_HPP

#include <hpdf.h>
#include <string>
#include <vector>
#include <memory>
#include <ostream>
#include <stdexcept>

namespace hj
{

class pdf
{
  public:
    class exception : public std::runtime_error
    {
      public:
        explicit exception(const std::string &message)
            : std::runtime_error("PDF Error: " + message)
        {
        }
    };

    enum class page_orientation
    {
        portrait,
        landscape
    };

    enum class font_name
    {
        helvetica,
        helvetica_bold,
        helvetica_oblique,
        helvetica_bold_oblique,
        times_roman,
        times_bold,
        times_italic,
        times_bold_italic,
        courier,
        courier_bold,
        courier_oblique,
        courier_bold_oblique
    };

    enum class text_align
    {
        left,
        center,
        right,
        justify
    };

    struct color
    {
        float r, g, b;

        color(float red = 0.0f, float green = 0.0f, float blue = 0.0f)
            : r(red)
            , g(green)
            , b(blue)
        {
        }

        static color black() { return color(0.0f, 0.0f, 0.0f); }
        static color white() { return color(1.0f, 1.0f, 1.0f); }
        static color red() { return color(1.0f, 0.0f, 0.0f); }
        static color green() { return color(0.0f, 1.0f, 0.0f); }
        static color blue() { return color(0.0f, 0.0f, 1.0f); }
    };

    struct point
    {
        float x, y;
        point(float x_pos = 0.0f, float y_pos = 0.0f)
            : x(x_pos)
            , y(y_pos)
        {
        }
    };

    struct rectangle
    {
        float x, y, width, height;
        rectangle(float x_pos = 0.0f,
                  float y_pos = 0.0f,
                  float w     = 0.0f,
                  float h     = 0.0f)
            : x(x_pos)
            , y(y_pos)
            , width(w)
            , height(h)
        {
        }
    };

    class page
    {
      public:
        enum class size
        {
            a4,
            a3,
            a5,
            letter,
            legal
        };

      public:
        page(HPDF_Doc doc, HPDF_Page page)
            : _doc(doc)
            , _page(page)
        {
            if(!_page)
                throw exception("Invalid page handle");
        }
        ~page()
        {
            // No need to free page explicitly; HPDF_Free(_doc) will handle it.
            _page = nullptr;
        }

        bool is_same(const page &other) const
        {
            return this->_page == other._page;
        }

        bool set_size(size             sz,
                      page_orientation orientation = page_orientation::portrait)
        {
            HPDF_PageSizes hpdf_size;

            switch(sz)
            {
                case size::a4:
                    hpdf_size = HPDF_PAGE_SIZE_A4;
                    break;
                case size::a3:
                    hpdf_size = HPDF_PAGE_SIZE_A3;
                    break;
                case size::a5:
                    hpdf_size = HPDF_PAGE_SIZE_A5;
                    break;
                case size::letter:
                    hpdf_size = HPDF_PAGE_SIZE_LETTER;
                    break;
                case size::legal:
                    hpdf_size = HPDF_PAGE_SIZE_LEGAL;
                    break;
                default:
                    hpdf_size = HPDF_PAGE_SIZE_A4;
                    break;
            }

            HPDF_PageDirection hpdf_direction =
                (orientation == page_orientation::portrait)
                    ? HPDF_PAGE_PORTRAIT
                    : HPDF_PAGE_LANDSCAPE;

            return HPDF_Page_SetSize(_page, hpdf_size, hpdf_direction)
                   == HPDF_OK;
        }

        bool set_size(float width, float height)
        {
            return HPDF_Page_SetWidth(_page, width) == HPDF_OK
                   && HPDF_Page_SetHeight(_page, height) == HPDF_OK;
        }

        void get_size(float &width, float &height) const
        {
            width  = HPDF_Page_GetWidth(_page);
            height = HPDF_Page_GetHeight(_page);
        }

        float get_width() const { return HPDF_Page_GetWidth(_page); }

        float get_height() const { return HPDF_Page_GetHeight(_page); }

        bool begin_text() { return HPDF_Page_BeginText(_page) == HPDF_OK; }

        bool end_text() { return HPDF_Page_EndText(_page) == HPDF_OK; }

        bool set_font(font_name font, float size)
        {
            const char *font_name;

            switch(font)
            {
                case font_name::helvetica:
                    font_name = HPDF_Font_GetFontName(
                        HPDF_GetFont(_doc, "Helvetica", NULL));
                    break;
                case font_name::helvetica_bold:
                    font_name = HPDF_Font_GetFontName(
                        HPDF_GetFont(_doc, "Helvetica-Bold", NULL));
                    break;
                case font_name::helvetica_oblique:
                    font_name = HPDF_Font_GetFontName(
                        HPDF_GetFont(_doc, "Helvetica-Oblique", NULL));
                    break;
                case font_name::helvetica_bold_oblique:
                    font_name = HPDF_Font_GetFontName(
                        HPDF_GetFont(_doc, "Helvetica-BoldOblique", NULL));
                    break;
                case font_name::times_roman:
                    font_name = HPDF_Font_GetFontName(
                        HPDF_GetFont(_doc, "Times-Roman", NULL));
                    break;
                case font_name::times_bold:
                    font_name = HPDF_Font_GetFontName(
                        HPDF_GetFont(_doc, "Times-Bold", NULL));
                    break;
                case font_name::times_italic:
                    font_name = HPDF_Font_GetFontName(
                        HPDF_GetFont(_doc, "Times-Italic", NULL));
                    break;
                case font_name::times_bold_italic:
                    font_name = HPDF_Font_GetFontName(
                        HPDF_GetFont(_doc, "Times-BoldItalic", NULL));
                    break;
                case font_name::courier:
                    font_name = HPDF_Font_GetFontName(
                        HPDF_GetFont(_doc, "Courier", NULL));
                    break;
                case font_name::courier_bold:
                    font_name = HPDF_Font_GetFontName(
                        HPDF_GetFont(_doc, "Courier-Bold", NULL));
                    break;
                case font_name::courier_oblique:
                    font_name = HPDF_Font_GetFontName(
                        HPDF_GetFont(_doc, "Courier-Oblique", NULL));
                    break;
                case font_name::courier_bold_oblique:
                    font_name = HPDF_Font_GetFontName(
                        HPDF_GetFont(_doc, "Courier-BoldOblique", NULL));
                    break;
                default:
                    font_name = "Helvetica";
                    break;
            }

            HPDF_Font hfont = HPDF_GetFont(_doc, font_name, NULL);
            return HPDF_Page_SetFontAndSize(_page, hfont, size) == HPDF_OK;
        }

        bool set_font(const char *font_name, float size)
        {
            HPDF_Font hfont = HPDF_GetFont(_doc, font_name, NULL);
            return HPDF_Page_SetFontAndSize(_page, hfont, size) == HPDF_OK;
        }

        bool set_text_color(const color &color)
        {
            return HPDF_Page_SetRGBFill(_page, color.r, color.g, color.b)
                   == HPDF_OK;
        }

        bool set_stroke_color(const color &color)
        {
            return HPDF_Page_SetRGBStroke(_page, color.r, color.g, color.b)
                   == HPDF_OK;
        }

        bool set_line_width(float width)
        {
            return HPDF_Page_SetLineWidth(_page, width) == HPDF_OK;
        }

        bool move_to(const point &point)
        {
            return HPDF_Page_MoveTo(_page, point.x, point.y) == HPDF_OK;
        }

        bool line_to(const point &point)
        {
            return HPDF_Page_LineTo(_page, point.x, point.y) == HPDF_OK;
        }

        bool close_path() { return HPDF_Page_ClosePath(_page) == HPDF_OK; }

        bool stroke() { return HPDF_Page_Stroke(_page) == HPDF_OK; }

        bool fill() { return HPDF_Page_Fill(_page) == HPDF_OK; }

        bool set_text_position(const point &point)
        {
            return HPDF_Page_MoveTextPos(_page, point.x, point.y) == HPDF_OK;
        }

        bool show_text(const std::string &text)
        {
            return HPDF_Page_ShowText(_page, text.c_str()) == HPDF_OK;
        }

        bool show_text_at(const std::string &text, const point &point)
        {
            return HPDF_Page_TextOut(_page, point.x, point.y, text.c_str())
                   == HPDF_OK;
        }

        bool draw_rectangle(const rectangle &rect, bool fill = false)
        {
            HPDF_Page_Rectangle(_page, rect.x, rect.y, rect.width, rect.height);
            if(fill)
                return HPDF_Page_Fill(_page) == HPDF_OK;
            else
                return HPDF_Page_Stroke(_page) == HPDF_OK;
        }

        bool draw_circle(const point &center, float radius, bool fill = false)
        {
            HPDF_Page_Circle(_page, center.x, center.y, radius);
            if(fill)
                return HPDF_Page_Fill(_page) == HPDF_OK;
            else
                return HPDF_Page_Stroke(_page) == HPDF_OK;
        }

        bool draw_ellipse(const point &center,
                          float        x_radius,
                          float        y_radius,
                          bool         fill = false)
        {
            HPDF_Page_Ellipse(_page, center.x, center.y, x_radius, y_radius);
            if(fill)
                return HPDF_Page_Fill(_page) == HPDF_OK;
            else
                return HPDF_Page_Stroke(_page) == HPDF_OK;
        }

        bool draw_line(const point &start, const point &end, bool fill = false)
        {
            HPDF_Page_MoveTo(_page, start.x, start.y);
            HPDF_Page_LineTo(_page, end.x, end.y);
            if(fill)
                return HPDF_Page_Fill(_page) == HPDF_OK;
            else
                return HPDF_Page_Stroke(_page) == HPDF_OK;
        }

        bool draw_text(const std::string &text, const point &position)
        {
            return HPDF_Page_TextOut(_page,
                                     position.x,
                                     position.y,
                                     text.c_str())
                   == HPDF_OK;
        }

        void set_text_alignment(text_align align, float page_width = 0.0f)
        {
            if(page_width == 0.0f)
                page_width = get_width();

            switch(align)
            {
                case text_align::left:
                    HPDF_Page_SetTextLeading(_page, 0);
                    break;
                case text_align::center:
                    HPDF_Page_SetTextLeading(_page, 1);
                    break;
                case text_align::right:
                    HPDF_Page_SetTextLeading(_page, 2);
                    break;
                case text_align::justify:
                    HPDF_Page_SetTextLeading(_page, 3);
                    break;
                default:
                    break;
            }
        }

        bool draw_image(const std::string &filename, const rectangle &rect)
        {
            HPDF_Image image =
                HPDF_LoadPngImageFromFile(_doc, filename.c_str());
            if(!image)
                image = HPDF_LoadJpegImageFromFile(_doc, filename.c_str());

            if(!image)
                return false;

            HPDF_Page_DrawImage(_page,
                                image,
                                rect.x,
                                rect.y,
                                rect.width,
                                rect.height);
            return true;
        }

        bool draw_image(const std::string &filename,
                        const point       &pos,
                        const float        width,
                        const float        height)
        {
            HPDF_Image image =
                HPDF_LoadPngImageFromFile(_doc, filename.c_str());
            if(!image)
                image = HPDF_LoadJpegImageFromFile(_doc, filename.c_str());

            if(image)
                HPDF_Page_DrawImage(_page, image, pos.x, pos.y, width, height);
            else
                return false;

            return true;
        }

        float get_text_width(const std::string &text) const
        {
            return HPDF_Page_TextWidth(_page, text.c_str());
        }

        bool insert_image(const std::string &filename,
                          const rectangle   &rectangle)
        {
            HPDF_Image image =
                HPDF_LoadPngImageFromFile(_doc, filename.c_str());
            if(!image)
                image = HPDF_LoadJpegImageFromFile(_doc, filename.c_str());

            if(image)
                HPDF_Page_DrawImage(_page,
                                    image,
                                    rectangle.x,
                                    rectangle.y,
                                    rectangle.width,
                                    rectangle.height);
            else
                return false;

            return true;
        }

        bool insert_image(const std::string &filename,
                          const point       &pos,
                          const float        width,
                          const float        height)
        {
            HPDF_Image image =
                HPDF_LoadPngImageFromFile(_doc, filename.c_str());
            if(!image)
                image = HPDF_LoadJpegImageFromFile(_doc, filename.c_str());

            if(image)
                HPDF_Page_DrawImage(_page, image, pos.x, pos.y, width, height);
            else
                return false;

            return true;
        }

      private:
        HPDF_Page _page;
        HPDF_Doc  _doc;
    };

    class document
    {
      public:
        using error_callback_t = std::function<void(HPDF_STATUS, HPDF_STATUS)>;

        enum permission : HPDF_UINT
        {
            none     = 0,
            print    = HPDF_ENABLE_PRINT,
            edit_all = HPDF_ENABLE_EDIT_ALL,
            copy     = HPDF_ENABLE_COPY,
            edit     = HPDF_ENABLE_EDIT,
            all = HPDF_ENABLE_PRINT | HPDF_ENABLE_EDIT_ALL | HPDF_ENABLE_COPY
                  | HPDF_ENABLE_EDIT
        };

      public:
        document(error_callback_t cb = nullptr)
            : _doc(nullptr)
            , _perm(permission::all)
            , _error_cb(cb)
        {
            _doc = HPDF_New(_error_handler, this);
            if(!_doc)
                throw exception("Failed to create PDF document");

            HPDF_SetCompressionMode(_doc, HPDF_COMP_ALL);
        }
        document(document &&other) noexcept
            : _doc(other._doc)
            , _pages(std::move(other._pages))
        {
            other._doc = nullptr;
        }
        document(const document &) = delete;

        ~document()
        {
            if(_doc)
                HPDF_Free(_doc);
        }

        document &operator=(const document &) = delete;
        document &operator=(document &&other) noexcept
        {
            if(this == &other)
                return *this;


            if(_doc)
                HPDF_Free(_doc);

            _doc       = other._doc;
            _pages     = std::move(other._pages);
            other._doc = nullptr;
            return *this;
        }

        page &add_page()
        {
            HPDF_Page origin = HPDF_AddPage(_doc);
            if(!origin)
                throw exception("Failed to add page");

            auto pdf_page = std::make_unique<hj::pdf::page>(_doc, origin);
            hj::pdf::page &page_ref = *pdf_page;
            _pages.push_back(std::move(pdf_page));

            return page_ref;
        }

        page &get_page(size_t index)
        {
            if(index >= _pages.size())
                throw exception("Page index out of range");

            return *_pages[index];
        }

        size_t get_page_count() const { return _pages.size(); }

        void set_title(const char *title)
        {
            HPDF_SetInfoAttr(_doc, HPDF_INFO_TITLE, title);
        }

        void set_author(const char *author)
        {
            HPDF_SetInfoAttr(_doc, HPDF_INFO_AUTHOR, author);
        }

        void set_subject(const char *subject)
        {
            HPDF_SetInfoAttr(_doc, HPDF_INFO_SUBJECT, subject);
        }

        void set_keywords(const char *keywords)
        {
            HPDF_SetInfoAttr(_doc, HPDF_INFO_KEYWORDS, keywords);
        }

        void set_creator(const char *creator)
        {
            HPDF_SetInfoAttr(_doc, HPDF_INFO_CREATOR, creator);
        }

        void set_producer(const char *producer)
        {
            HPDF_SetInfoAttr(_doc, HPDF_INFO_PRODUCER, producer);
        }

        void set_password(const char *owner_passwd,
                          const char *user_passwd = "")
        {
            HPDF_SetPassword(_doc, owner_passwd, user_passwd);
        }

        // NOTE: you should set_password before setting permissions!!!
        void set_permission(permission perm)
        {
            _perm = perm;
            HPDF_SetPermission(_doc, _perm);
        }

        void add_permission(permission perm)
        {
            _perm |= perm;
            HPDF_SetPermission(_doc, _perm);
        }

        void remove_permission(permission perm)
        {
            _perm &= ~perm;
            HPDF_SetPermission(_doc, _perm);
        }

        bool has_permission(permission perm) const
        {
            return (_perm & perm) != 0;
        }

        permission get_permission() const
        {
            return static_cast<permission>(_perm);
        }

        void clear_all_permissions() { set_permission(permission::none); }

        bool save(const std::string &filename)
        {
            return HPDF_SaveToFile(_doc, filename.c_str()) == HPDF_OK;
        }

        bool save(std::vector<unsigned char> &out)
        {
            if(!_doc)
                return false;

            if(HPDF_SaveToStream(_doc) != HPDF_OK)
                return false;

            HPDF_UINT32 size = HPDF_GetStreamSize(_doc);
            if(size == 0)
                return false;

            out.resize(size);
            HPDF_UINT32 actual_size = size;
            if(HPDF_ReadFromStream(_doc, out.data(), &actual_size) != HPDF_OK)
                return false;

            out.resize(actual_size);
            HPDF_ResetStream(_doc);
            return true;
        }

        bool save(std::ostream &out)
        {
            if(!_doc)
                return false;

            if(HPDF_SaveToStream(_doc) != HPDF_OK)
                return false;

            HPDF_UINT32 size = HPDF_GetStreamSize(_doc);
            if(size == 0)
                return false;

            HPDF_UINT32                actual_size = size;
            std::vector<unsigned char> buffer(size);
            if(HPDF_ReadFromStream(_doc, buffer.data(), &actual_size)
               != HPDF_OK)
                return false;

            out.write(reinterpret_cast<const char *>(buffer.data()),
                      actual_size);
            HPDF_ResetStream(_doc);
            return out.good();
        }

      private:
        static void _error_handler(HPDF_STATUS error_no,
                                   HPDF_STATUS detail_no,
                                   void       *user_data)
        {
            auto *self = static_cast<document *>(user_data);
            if(self && self->_error_cb)
                self->_error_cb(error_no, detail_no);
            else
                throw exception("HPDF Error: " + std::to_string(error_no)
                                + ", Detail: " + std::to_string(detail_no));
        }

      private:
        HPDF_Doc                           _doc;
        std::vector<std::unique_ptr<page>> _pages;
        HPDF_UINT                          _perm;
        error_callback_t                   _error_cb;
    };
};

} // namespace hj

#endif // PDF_HPP