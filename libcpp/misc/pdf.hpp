#ifndef PDF_HPP
#define PDF_HPP

#include <hpdf.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace libcpp {

class pdf
{
  public:
    class exception : public std::runtime_error
    {
      public:
        explicit exception(const std::string& message)
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
            : r(red), g(green), b(blue)
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
        point(float x_pos = 0.0f, float y_pos = 0.0f) : x(x_pos), y(y_pos) {}
    };

    struct rectangle
    {
        float x, y, width, height;
        rectangle(float x_pos = 0.0f,
                  float y_pos = 0.0f,
                  float w = 0.0f,
                  float h = 0.0f)
            : x(x_pos), y(y_pos), width(w), height(h)
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
        page(HPDF_Doc doc, HPDF_Page page) : doc_(doc), page_(page)
        {
            if (!page_)
                throw exception("Invalid page handle");
        }
        ~page()
        {
            // No need to free page explicitly; HPDF_Free(doc_) will handle it.
            page_ = nullptr;
        }

        bool is_same(const page& other) const
        {
            return this->page_ == other.page_;
        }

        bool set_size(size sz,
                      page_orientation orientation = page_orientation::portrait)
        {
            HPDF_PageSizes hpdf_size;

            switch (sz)
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

            return HPDF_Page_SetSize(page_, hpdf_size, hpdf_direction) ==
                   HPDF_OK;
        }

        bool set_size(float width, float height)
        {
            return HPDF_Page_SetWidth(page_, width) == HPDF_OK &&
                   HPDF_Page_SetHeight(page_, height) == HPDF_OK;
        }

        void get_size(float& width, float& height) const
        {
            width = HPDF_Page_GetWidth(page_);
            height = HPDF_Page_GetHeight(page_);
        }

        float get_width() const { return HPDF_Page_GetWidth(page_); }

        float get_height() const { return HPDF_Page_GetHeight(page_); }

        bool begin_text() { return HPDF_Page_BeginText(page_) == HPDF_OK; }

        bool end_text() { return HPDF_Page_EndText(page_) == HPDF_OK; }

        bool set_font(font_name font, float size)
        {
            const char* font_name;

            switch (font)
            {
                case font_name::helvetica:
                    font_name = HPDF_Font_GetFontName(
                        HPDF_GetFont(doc_, "Helvetica", NULL));
                    break;
                case font_name::helvetica_bold:
                    font_name = HPDF_Font_GetFontName(
                        HPDF_GetFont(doc_, "Helvetica-Bold", NULL));
                    break;
                case font_name::helvetica_oblique:
                    font_name = HPDF_Font_GetFontName(
                        HPDF_GetFont(doc_, "Helvetica-Oblique", NULL));
                    break;
                case font_name::helvetica_bold_oblique:
                    font_name = HPDF_Font_GetFontName(
                        HPDF_GetFont(doc_, "Helvetica-BoldOblique", NULL));
                    break;
                case font_name::times_roman:
                    font_name = HPDF_Font_GetFontName(
                        HPDF_GetFont(doc_, "Times-Roman", NULL));
                    break;
                case font_name::times_bold:
                    font_name = HPDF_Font_GetFontName(
                        HPDF_GetFont(doc_, "Times-Bold", NULL));
                    break;
                case font_name::times_italic:
                    font_name = HPDF_Font_GetFontName(
                        HPDF_GetFont(doc_, "Times-Italic", NULL));
                    break;
                case font_name::times_bold_italic:
                    font_name = HPDF_Font_GetFontName(
                        HPDF_GetFont(doc_, "Times-BoldItalic", NULL));
                    break;
                case font_name::courier:
                    font_name = HPDF_Font_GetFontName(
                        HPDF_GetFont(doc_, "Courier", NULL));
                    break;
                case font_name::courier_bold:
                    font_name = HPDF_Font_GetFontName(
                        HPDF_GetFont(doc_, "Courier-Bold", NULL));
                    break;
                case font_name::courier_oblique:
                    font_name = HPDF_Font_GetFontName(
                        HPDF_GetFont(doc_, "Courier-Oblique", NULL));
                    break;
                case font_name::courier_bold_oblique:
                    font_name = HPDF_Font_GetFontName(
                        HPDF_GetFont(doc_, "Courier-BoldOblique", NULL));
                    break;
                default:
                    font_name = "Helvetica";
                    break;
            }

            HPDF_Font hfont = HPDF_GetFont(doc_, font_name, NULL);
            return HPDF_Page_SetFontAndSize(page_, hfont, size) == HPDF_OK;
        }

        bool set_font(const char* font_name, float size)
        {
            HPDF_Font hfont = HPDF_GetFont(doc_, font_name, NULL);
            return HPDF_Page_SetFontAndSize(page_, hfont, size) == HPDF_OK;
        }

        bool set_text_color(const color& color)
        {
            return HPDF_Page_SetRGBFill(page_, color.r, color.g, color.b) ==
                   HPDF_OK;
        }

        bool set_stroke_color(const color& color)
        {
            return HPDF_Page_SetRGBStroke(page_, color.r, color.g, color.b) ==
                   HPDF_OK;
        }

        bool set_line_width(float width)
        {
            return HPDF_Page_SetLineWidth(page_, width) == HPDF_OK;
        }

        bool move_to(const point& point)
        {
            return HPDF_Page_MoveTo(page_, point.x, point.y) == HPDF_OK;
        }

        bool line_to(const point& point)
        {
            return HPDF_Page_LineTo(page_, point.x, point.y) == HPDF_OK;
        }

        bool close_path() { return HPDF_Page_ClosePath(page_) == HPDF_OK; }

        bool stroke() { return HPDF_Page_Stroke(page_) == HPDF_OK; }

        bool fill() { return HPDF_Page_Fill(page_) == HPDF_OK; }

        bool set_text_position(const point& point)
        {
            return HPDF_Page_MoveTextPos(page_, point.x, point.y) == HPDF_OK;
        }

        bool show_text(const std::string& text)
        {
            return HPDF_Page_ShowText(page_, text.c_str()) == HPDF_OK;
        }

        bool show_text_at(const std::string& text, const point& point)
        {
            return HPDF_Page_TextOut(page_, point.x, point.y, text.c_str()) ==
                   HPDF_OK;
        }

        bool draw_rectangle(const rectangle& rect, bool fill = false)
        {
            HPDF_Page_Rectangle(page_, rect.x, rect.y, rect.width, rect.height);
            if (fill)
                return HPDF_Page_Fill(page_) == HPDF_OK;
            else
                return HPDF_Page_Stroke(page_) == HPDF_OK;
        }

        bool draw_circle(const point& center, float radius, bool fill = false)
        {
            HPDF_Page_Circle(page_, center.x, center.y, radius);
            if (fill)
                return HPDF_Page_Fill(page_) == HPDF_OK;
            else
                return HPDF_Page_Stroke(page_) == HPDF_OK;
        }

        bool draw_ellipse(const point& center,
                          float x_radius,
                          float y_radius,
                          bool fill = false)
        {
            HPDF_Page_Ellipse(page_, center.x, center.y, x_radius, y_radius);
            if (fill)
                return HPDF_Page_Fill(page_) == HPDF_OK;
            else
                return HPDF_Page_Stroke(page_) == HPDF_OK;
        }

        bool draw_line(const point& start, const point& end, bool fill = false)
        {
            HPDF_Page_MoveTo(page_, start.x, start.y);
            HPDF_Page_LineTo(page_, end.x, end.y);
            if (fill)
                return HPDF_Page_Fill(page_) == HPDF_OK;
            else
                return HPDF_Page_Stroke(page_) == HPDF_OK;
        }

        bool draw_text(const std::string& text, const point& position)
        {
            return HPDF_Page_TextOut(page_,
                                     position.x,
                                     position.y,
                                     text.c_str()) == HPDF_OK;
        }

        void set_text_alignment(text_align align)
        {
            switch (align)
            {
                case text_align::left:
                    HPDF_Page_SetTextLeading(page_, 0);
                    break;
                case text_align::center:
                    HPDF_Page_SetTextLeading(page_, 1);
                    break;
                case text_align::right:
                    HPDF_Page_SetTextLeading(page_, 2);
                    break;
                case text_align::justify:
                    HPDF_Page_SetTextLeading(page_, 3);
                    break;
                default:
                    break;
            }
        }

        bool draw_image(const std::string& filename, const rectangle& rect)
        {
            HPDF_Image image =
                HPDF_LoadPngImageFromFile(doc_, filename.c_str());
            if (!image)
                image = HPDF_LoadJpegImageFromFile(doc_, filename.c_str());

            if (image)
                HPDF_Page_DrawImage(page_,
                                    image,
                                    rect.x,
                                    rect.y,
                                    rect.width,
                                    rect.height);
            else
                return false;

            return true;
        }

        bool draw_image(const std::string& filename,
                        const point& pos,
                        const float width,
                        const float height)
        {
            HPDF_Image image =
                HPDF_LoadPngImageFromFile(doc_, filename.c_str());
            if (!image)
                image = HPDF_LoadJpegImageFromFile(doc_, filename.c_str());

            if (image)
                HPDF_Page_DrawImage(page_, image, pos.x, pos.y, width, height);
            else
                return false;

            return true;
        }

        float get_text_width(const std::string& text) const
        {
            return HPDF_Page_TextWidth(page_, text.c_str());
        }

        bool insert_image(const std::string& filename,
                          const rectangle& rectangle)
        {
            HPDF_Image image =
                HPDF_LoadPngImageFromFile(doc_, filename.c_str());
            if (!image)
                image = HPDF_LoadJpegImageFromFile(doc_, filename.c_str());

            if (image)
                HPDF_Page_DrawImage(page_,
                                    image,
                                    rectangle.x,
                                    rectangle.y,
                                    rectangle.width,
                                    rectangle.height);
            else
                return false;

            return true;
        }

        bool insert_image(const std::string& filename,
                          const point& pos,
                          const float width,
                          const float height)
        {
            HPDF_Image image =
                HPDF_LoadPngImageFromFile(doc_, filename.c_str());
            if (!image)
                image = HPDF_LoadJpegImageFromFile(doc_, filename.c_str());

            if (image)
                HPDF_Page_DrawImage(page_, image, pos.x, pos.y, width, height);
            else
                return false;

            return true;
        }

      private:
        HPDF_Page page_;
        HPDF_Doc doc_;
    };

    class document
    {
      public:
        enum permission : HPDF_UINT
        {
            none = 0,
            print = HPDF_ENABLE_PRINT,
            edit_all = HPDF_ENABLE_EDIT_ALL,
            copy = HPDF_ENABLE_COPY,
            edit = HPDF_ENABLE_EDIT,
            all = HPDF_ENABLE_PRINT | HPDF_ENABLE_EDIT_ALL | HPDF_ENABLE_COPY |
                  HPDF_ENABLE_EDIT
        };

      public:
        document() : doc_(nullptr), perm_(permission::all)
        {
            doc_ = HPDF_New(error_handler, NULL);
            if (!doc_)
                throw exception("Failed to create PDF document");

            HPDF_SetCompressionMode(doc_, HPDF_COMP_ALL);
        }

        ~document()
        {
            if (doc_)
                HPDF_Free(doc_);
        }

        document(document&& other) noexcept
            : doc_(other.doc_), pages_(std::move(other.pages_))
        {
            other.doc_ = nullptr;
        }

        document& operator=(document&& other) noexcept
        {
            if (this == &other)
                return *this;


            if (doc_)
                HPDF_Free(doc_);

            doc_ = other.doc_;
            pages_ = std::move(other.pages_);
            other.doc_ = nullptr;
            return *this;
        }

        page& add_page()
        {
            HPDF_Page origin = HPDF_AddPage(doc_);
            if (!origin)
                throw exception("Failed to add page");

            auto pdf_page = std::make_unique<libcpp::pdf::page>(doc_, origin);
            libcpp::pdf::page& page_ref = *pdf_page;
            pages_.push_back(std::move(pdf_page));

            return page_ref;
        }

        page& get_page(size_t index)
        {
            if (index >= pages_.size())
                throw exception("Page index out of range");

            return *pages_[index];
        }

        size_t get_page_count() const { return pages_.size(); }

        void set_title(const char* title)
        {
            HPDF_SetInfoAttr(doc_, HPDF_INFO_TITLE, title);
        }

        void set_author(const char* author)
        {
            HPDF_SetInfoAttr(doc_, HPDF_INFO_AUTHOR, author);
        }

        void set_subject(const char* subject)
        {
            HPDF_SetInfoAttr(doc_, HPDF_INFO_SUBJECT, subject);
        }

        void set_keywords(const char* keywords)
        {
            HPDF_SetInfoAttr(doc_, HPDF_INFO_KEYWORDS, keywords);
        }

        void set_creator(const char* creator)
        {
            HPDF_SetInfoAttr(doc_, HPDF_INFO_CREATOR, creator);
        }

        void set_producer(const char* producer)
        {
            HPDF_SetInfoAttr(doc_, HPDF_INFO_PRODUCER, producer);
        }

        void set_password(const char* owner_passwd,
                          const char* user_passwd = "")
        {
            HPDF_SetPassword(doc_, owner_passwd, user_passwd);
        }

        // NOTE: you should set_password before setting permissions!!!
        void set_permission(permission perm)
        {
            perm_ = perm;
            HPDF_SetPermission(doc_, perm_);
        }

        void add_permission(permission perm)
        {
            perm_ |= perm;
            HPDF_SetPermission(doc_, perm_);
        }

        void remove_permission(permission perm)
        {
            perm_ &= ~perm;
            HPDF_SetPermission(doc_, perm_);
        }

        bool has_permission(permission perm) const
        {
            return (perm_ & perm) != 0;
        }

        permission get_permission() const
        {
            return static_cast<permission>(perm_);
        }

        void clear_all_permissions() { set_permission(permission::none); }

        bool save_to_file(const std::string& filename)
        {
            return HPDF_SaveToFile(doc_, filename.c_str()) == HPDF_OK;
        }

        bool save_to_memory(std::vector<unsigned char>& out)
        {
            if (!doc_)
                return false;

            if (HPDF_SaveToStream(doc_) != HPDF_OK)
                return false;

            HPDF_UINT32 size = HPDF_GetStreamSize(doc_);
            if (size == 0)
                return false;

            out.resize(size);
            HPDF_UINT32 actual_size = size;

            if (HPDF_ReadFromStream(doc_, out.data(), &actual_size) != HPDF_OK)
                return false;

            out.resize(actual_size);

            HPDF_ResetStream(doc_);
            return true;
        }

      private:
        HPDF_Doc doc_;
        std::vector<std::unique_ptr<page> > pages_;
        HPDF_UINT perm_;

        static void error_handler(HPDF_STATUS error_no,
                                  HPDF_STATUS detail_no,
                                  void* user_data)
        {
            throw exception("HPDF Error: " + std::to_string(error_no) +
                            ", Detail: " + std::to_string(detail_no));
        }

        document(const document&) = delete;
        document& operator=(const document&) = delete;
    };
};

}  // namespace libcpp

#endif  // PDF_HPP