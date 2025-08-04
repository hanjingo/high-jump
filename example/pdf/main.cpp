#include <iostream>

#include <libcpp/misc/pdf.hpp>

int main(int argc, char* argv[])
{
    try
    {
        libcpp::pdf::document doc;
        doc.set_title("my PDF Document");
        doc.set_author("libcpp");
        doc.set_subject("PDF testing");
        doc.set_keywords("libcpp, pdf, example");
        doc.set_creator("hehehunanchina@live.com");
        doc.set_password("hello", "world");

        auto& page = doc.add_page();
        page.set_size(libcpp::pdf::page::size::a4,
                      libcpp::pdf::page_orientation::portrait);

        page.set_font(libcpp::pdf::font_name::helvetica_bold, 18);
        page.set_text_color(libcpp::pdf::color::blue());
        page.begin_text();
        page.show_text_at("PDF example using libcpp",
                          libcpp::pdf::point(50, 750));
        page.end_text();

        page.set_font(libcpp::pdf::font_name::helvetica, 12);
        page.set_text_color(libcpp::pdf::color::black());
        page.begin_text();
        page.show_text_at("line1", libcpp::pdf::point(50, 700));
        page.show_text_at("line2", libcpp::pdf::point(50, 680));
        page.end_text();

        page.set_stroke_color(libcpp::pdf::color::red());
        page.set_line_width(2.0f);
        page.draw_rectangle(libcpp::pdf::rectangle(50, 600, 200, 50), false);

        page.set_stroke_color(libcpp::pdf::color::green());
        page.draw_circle(libcpp::pdf::point(150, 500), 30, true);

        doc.save_to_file("example.pdf");

        std::cout << "PDF create succ with example.pdf" << std::endl;
    }
    catch (const libcpp::pdf::exception& e)
    {
        std::cerr << "PDF exception: " << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "exception: " << e.what() << std::endl;
        return 1;
    }

    std::cin.get();
    return 0;
}