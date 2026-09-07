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

#ifndef PDF_HPP
#define PDF_HPP

#include <hpdf.h>
#include <filesystem>
#include <functional>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

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
        float r{0.0f}, g{0.0f}, b{0.0f};

        constexpr color() noexcept = default;
        constexpr color(float red, float green, float blue) noexcept
            : r(red)
            , g(green)
            , b(blue)
        {
        }

        static constexpr color black() noexcept
        {
            return color(0.0f, 0.0f, 0.0f);
        }
        static constexpr color white() noexcept
        {
            return color(1.0f, 1.0f, 1.0f);
        }
        static constexpr color red() noexcept
        {
            return color(1.0f, 0.0f, 0.0f);
        }
        static constexpr color green() noexcept
        {
            return color(0.0f, 1.0f, 0.0f);
        }
        static constexpr color blue() noexcept
        {
            return color(0.0f, 0.0f, 1.0f);
        }
    };

    struct point
    {
        float x{0.0f}, y{0.0f};
        constexpr point() noexcept = default;
        constexpr point(float x_pos, float y_pos) noexcept
            : x(x_pos)
            , y(y_pos)
        {
        }
    };

    struct rectangle
    {
        float x{0.0f}, y{0.0f}, width{0.0f}, height{0.0f};
        constexpr rectangle() noexcept = default;
        constexpr rectangle(float x_pos, float y_pos, float w, float h) noexcept
            : x(x_pos)
            , y(y_pos)
            , width(w)
            , height(h)
        {
        }
    };

    // Lightweight handle class referencing HPDF_Page object
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
        page(HPDF_Doc doc, HPDF_Page page_handle)
            : _doc(doc)
            , _page(page_handle)
        {
            if(!_page)
                throw exception("Invalid page handle provided");
        }

        ~page()                           = default;
        page(const page &)                = default;
        page &operator=(const page &)     = default;
        page(page &&) noexcept            = default;
        page &operator=(page &&) noexcept = default;

        [[nodiscard]] bool is_same(const page &other) const noexcept
        {
            return this->_page == other._page;
        }

        [[nodiscard]] bool set_size(
            size             sz,
            page_orientation orientation = page_orientation::portrait) noexcept
        {
            HPDF_PageSizes hpdf_size;
            switch(sz)
            {
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
                case size::a4:
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

        [[nodiscard]] bool set_size(float width, float height) noexcept
        {
            return HPDF_Page_SetWidth(_page, width) == HPDF_OK
                   && HPDF_Page_SetHeight(_page, height) == HPDF_OK;
        }

        void get_size(float &width, float &height) const noexcept
        {
            width  = HPDF_Page_GetWidth(_page);
            height = HPDF_Page_GetHeight(_page);
        }

        [[nodiscard]] float get_width() const noexcept
        {
            return HPDF_Page_GetWidth(_page);
        }
        [[nodiscard]] float get_height() const noexcept
        {
            return HPDF_Page_GetHeight(_page);
        }

        [[nodiscard]] bool begin_text() noexcept
        {
            return HPDF_Page_BeginText(_page) == HPDF_OK;
        }
        [[nodiscard]] bool end_text() noexcept
        {
            return HPDF_Page_EndText(_page) == HPDF_OK;
        }

        [[nodiscard]] bool set_font(font_name font, float size) noexcept
        {
            const char *name = nullptr;
            switch(font)
            {
                case font_name::helvetica_bold:
                    name = "Helvetica-Bold";
                    break;
                case font_name::helvetica_oblique:
                    name = "Helvetica-Oblique";
                    break;
                case font_name::helvetica_bold_oblique:
                    name = "Helvetica-BoldOblique";
                    break;
                case font_name::times_roman:
                    name = "Times-Roman";
                    break;
                case font_name::times_bold:
                    name = "Times-Bold";
                    break;
                case font_name::times_italic:
                    name = "Times-Italic";
                    break;
                case font_name::times_bold_italic:
                    name = "Times-BoldItalic";
                    break;
                case font_name::courier:
                    name = "Courier";
                    break;
                case font_name::courier_bold:
                    name = "Courier-Bold";
                    break;
                case font_name::courier_oblique:
                    name = "Courier-Oblique";
                    break;
                case font_name::courier_bold_oblique:
                    name = "Courier-BoldOblique";
                    break;
                case font_name::helvetica:
                default:
                    name = "Helvetica";
                    break;
            }

            HPDF_Font hfont = HPDF_GetFont(_doc, name, nullptr);
            return hfont ? (HPDF_Page_SetFontAndSize(_page, hfont, size)
                            == HPDF_OK)
                         : false;
        }

        [[nodiscard]] bool set_font(const char *font_name_str,
                                    float       size) noexcept
        {
            HPDF_Font hfont = HPDF_GetFont(_doc, font_name_str, nullptr);
            return hfont ? (HPDF_Page_SetFontAndSize(_page, hfont, size)
                            == HPDF_OK)
                         : false;
        }

        [[nodiscard]] bool set_text_color(const color &c) noexcept
        {
            return HPDF_Page_SetRGBFill(_page, c.r, c.g, c.b) == HPDF_OK;
        }

        [[nodiscard]] bool set_stroke_color(const color &c) noexcept
        {
            return HPDF_Page_SetRGBStroke(_page, c.r, c.g, c.b) == HPDF_OK;
        }

        [[nodiscard]] bool set_line_width(float width) noexcept
        {
            return HPDF_Page_SetLineWidth(_page, width) == HPDF_OK;
        }

        [[nodiscard]] bool move_to(const point &pt) noexcept
        {
            return HPDF_Page_MoveTo(_page, pt.x, pt.y) == HPDF_OK;
        }

        [[nodiscard]] bool line_to(const point &pt) noexcept
        {
            return HPDF_Page_LineTo(_page, pt.x, pt.y) == HPDF_OK;
        }

        [[nodiscard]] bool close_path() noexcept
        {
            return HPDF_Page_ClosePath(_page) == HPDF_OK;
        }
        [[nodiscard]] bool stroke() noexcept
        {
            return HPDF_Page_Stroke(_page) == HPDF_OK;
        }
        [[nodiscard]] bool fill() noexcept
        {
            return HPDF_Page_Fill(_page) == HPDF_OK;
        }

        [[nodiscard]] bool set_text_position(const point &pt) noexcept
        {
            return HPDF_Page_MoveTextPos(_page, pt.x, pt.y) == HPDF_OK;
        }

        [[nodiscard]] bool show_text(std::string_view text) noexcept
        {
            return HPDF_Page_ShowText(_page, std::string(text).c_str())
                   == HPDF_OK;
        }

        [[nodiscard]] bool show_text_at(std::string_view text,
                                        const point     &pt) noexcept
        {
            return HPDF_Page_TextOut(_page,
                                     pt.x,
                                     pt.y,
                                     std::string(text).c_str())
                   == HPDF_OK;
        }

        [[nodiscard]] bool draw_rectangle(const rectangle &rect,
                                          bool fill_rect = false) noexcept
        {
            if(HPDF_Page_Rectangle(_page,
                                   rect.x,
                                   rect.y,
                                   rect.width,
                                   rect.height)
               != HPDF_OK)
                return false;
            return fill_rect ? (HPDF_Page_Fill(_page) == HPDF_OK)
                             : (HPDF_Page_Stroke(_page) == HPDF_OK);
        }

        [[nodiscard]] bool draw_circle(const point &center,
                                       float        radius,
                                       bool         fill_shape = false) noexcept
        {
            if(HPDF_Page_Circle(_page, center.x, center.y, radius) != HPDF_OK)
                return false;
            return fill_shape ? (HPDF_Page_Fill(_page) == HPDF_OK)
                              : (HPDF_Page_Stroke(_page) == HPDF_OK);
        }

        [[nodiscard]] bool draw_ellipse(const point &center,
                                        float        x_radius,
                                        float        y_radius,
                                        bool fill_shape = false) noexcept
        {
            if(HPDF_Page_Ellipse(_page, center.x, center.y, x_radius, y_radius)
               != HPDF_OK)
                return false;
            return fill_shape ? (HPDF_Page_Fill(_page) == HPDF_OK)
                              : (HPDF_Page_Stroke(_page) == HPDF_OK);
        }

        [[nodiscard]] bool draw_line(const point &start,
                                     const point &end) noexcept
        {
            if(HPDF_Page_MoveTo(_page, start.x, start.y) != HPDF_OK)
                return false;
            if(HPDF_Page_LineTo(_page, end.x, end.y) != HPDF_OK)
                return false;
            return HPDF_Page_Stroke(_page) == HPDF_OK;
        }

        [[nodiscard]] bool draw_text(std::string_view text,
                                     const point     &position) noexcept
        {
            return show_text_at(text, position);
        }

        [[nodiscard]] bool draw_text_aligned(std::string_view text,
                                             const rectangle &rect,
                                             text_align       align) noexcept
        {
            HPDF_TextAlignment hpdf_align;
            switch(align)
            {
                case text_align::center:
                    hpdf_align = HPDF_TALIGN_CENTER;
                    break;
                case text_align::right:
                    hpdf_align = HPDF_TALIGN_RIGHT;
                    break;
                case text_align::justify:
                    hpdf_align = HPDF_TALIGN_JUSTIFY;
                    break;
                case text_align::left:
                default:
                    hpdf_align = HPDF_TALIGN_LEFT;
                    break;
            }

            return HPDF_Page_TextRect(_page,
                                      rect.x,
                                      rect.y + rect.height,
                                      rect.x + rect.width,
                                      rect.y,
                                      std::string(text).c_str(),
                                      hpdf_align,
                                      nullptr)
                   == HPDF_OK;
        }

        [[nodiscard]] bool draw_image(const std::filesystem::path &image_path,
                                      const rectangle &rect) noexcept
        {
            std::string path_str = image_path.string();
            HPDF_Image  image =
                HPDF_LoadPngImageFromFile(_doc, path_str.c_str());
            if(!image)
                image = HPDF_LoadJpegImageFromFile(_doc, path_str.c_str());

            if(!image)
                return false;

            return HPDF_Page_DrawImage(_page,
                                       image,
                                       rect.x,
                                       rect.y,
                                       rect.width,
                                       rect.height)
                   == HPDF_OK;
        }

        [[nodiscard]] bool draw_image(const std::filesystem::path &image_path,
                                      const point                 &pos,
                                      float                        width,
                                      float height) noexcept
        {
            return draw_image(image_path,
                              rectangle(pos.x, pos.y, width, height));
        }

        [[nodiscard]] float get_text_width(std::string_view text) const noexcept
        {
            return HPDF_Page_TextWidth(_page, std::string(text).c_str());
        }

      private:
        HPDF_Doc  _doc{nullptr};
        HPDF_Page _page{nullptr};
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
        explicit document(error_callback_t cb = nullptr)
            : _error_cb(std::move(cb))
        {
            _doc = HPDF_New(_error_handler, this);
            if(!_doc)
                throw exception("Failed to initialize HPDF_Doc context");

            HPDF_SetCompressionMode(_doc, HPDF_COMP_ALL);
        }

        ~document()
        {
            if(_doc)
                HPDF_Free(_doc);
        }

        document(const document &)            = delete;
        document &operator=(const document &) = delete;

        document(document &&other) noexcept
            : _doc(other._doc)
            , _page_count(other._page_count)
            , _perm(other._perm)
            , _error_cb(std::move(other._error_cb))
        {
            other._doc        = nullptr;
            other._page_count = 0;
        }

        document &operator=(document &&other) noexcept
        {
            if(this != &other)
            {
                if(_doc)
                    HPDF_Free(_doc);

                _doc              = other._doc;
                _page_count       = other._page_count;
                _perm             = other._perm;
                _error_cb         = std::move(other._error_cb);
                other._doc        = nullptr;
                other._page_count = 0;
            }
            return *this;
        }

        page add_page()
        {
            HPDF_Page hpage = HPDF_AddPage(_doc);
            if(!hpage)
                throw exception("Failed to add page to document");

            ++_page_count;
            return page(_doc, hpage);
        }

        [[nodiscard]] page get_page(size_t index)
        {
            HPDF_Page hpage =
                HPDF_GetPageByIndex(_doc, static_cast<HPDF_UINT>(index));
            if(!hpage)
                throw exception("Page index out of bounds: "
                                + std::to_string(index));

            return page(_doc, hpage);
        }

        [[nodiscard]] size_t get_page_count() const noexcept
        {
            return _page_count;
        }

        void set_title(std::string_view title)
        {
            HPDF_SetInfoAttr(_doc, HPDF_INFO_TITLE, std::string(title).c_str());
        }

        void set_author(std::string_view author)
        {
            HPDF_SetInfoAttr(_doc,
                             HPDF_INFO_AUTHOR,
                             std::string(author).c_str());
        }

        void set_subject(std::string_view subject)
        {
            HPDF_SetInfoAttr(_doc,
                             HPDF_INFO_SUBJECT,
                             std::string(subject).c_str());
        }

        void set_keywords(std::string_view keywords)
        {
            HPDF_SetInfoAttr(_doc,
                             HPDF_INFO_KEYWORDS,
                             std::string(keywords).c_str());
        }

        void set_creator(std::string_view creator)
        {
            HPDF_SetInfoAttr(_doc,
                             HPDF_INFO_CREATOR,
                             std::string(creator).c_str());
        }

        void set_producer(std::string_view producer)
        {
            HPDF_SetInfoAttr(_doc,
                             HPDF_INFO_PRODUCER,
                             std::string(producer).c_str());
        }

        void set_password(std::string_view owner_passwd,
                          std::string_view user_passwd = "")
        {
            HPDF_SetPassword(_doc,
                             std::string(owner_passwd).c_str(),
                             std::string(user_passwd).c_str());
        }

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

        [[nodiscard]] bool has_permission(permission perm) const noexcept
        {
            return (_perm & perm) != 0;
        }

        [[nodiscard]] permission get_permission() const noexcept
        {
            return static_cast<permission>(_perm);
        }

        void clear_all_permissions() { set_permission(permission::none); }

        [[nodiscard]] bool save(const std::filesystem::path &filepath) noexcept
        {
            return HPDF_SaveToFile(_doc, filepath.string().c_str()) == HPDF_OK;
        }

        [[nodiscard]] bool save(std::vector<unsigned char> &out) noexcept
        {
            if(!_doc || HPDF_SaveToStream(_doc) != HPDF_OK)
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

        [[nodiscard]] bool save(std::ostream &out)
        {
            std::vector<unsigned char> buffer;
            if(!save(buffer))
                return false;

            out.write(reinterpret_cast<const char *>(buffer.data()),
                      static_cast<std::streamsize>(buffer.size()));
            return out.good();
        }

      private:
        static void _error_handler(HPDF_STATUS error_no,
                                   HPDF_STATUS detail_no,
                                   void       *user_data) noexcept
        {
            try
            {
                auto *self = static_cast<document *>(user_data);
                if(self && self->_error_cb)
                {
                    self->_error_cb(error_no, detail_no);
                }
            }
            catch(...)
            {
            }
        }

      private:
        HPDF_Doc         _doc{nullptr};
        size_t           _page_count{0};
        HPDF_UINT        _perm{permission::all};
        error_callback_t _error_cb;
    };
};

} // namespace hj

#endif // PDF_HPP