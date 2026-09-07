#include <gtest/gtest.h>
#include <hj/misc/pdf.hpp>
#include <sstream>
#include <vector>

TEST(pdf, set_size)
{
    hj::pdf::document doc;
    auto              page = doc.add_page();
    EXPECT_TRUE(page.set_size(hj::pdf::page::size::a4));
}

TEST(pdf, get_size)
{
    hj::pdf::document doc;
    auto              page = doc.add_page();
    EXPECT_TRUE(page.set_size(hj::pdf::page::size::a4));
    float width = 0.0f, height = 0.0f;
    page.get_size(width, height);
    EXPECT_NEAR(width, 595.0f, 1.0f);
    EXPECT_NEAR(height, 842.0f, 1.0f);
}

TEST(pdf, get_width)
{
    hj::pdf::document doc;
    auto              page = doc.add_page();
    EXPECT_TRUE(page.set_size(hj::pdf::page::size::a4));
    auto width = page.get_width();
    EXPECT_NEAR(width, 595.0f, 1.0f);
}

TEST(pdf, get_height)
{
    hj::pdf::document doc;
    auto              page = doc.add_page();
    EXPECT_TRUE(page.set_size(hj::pdf::page::size::a4));
    auto height = page.get_height();
    EXPECT_NEAR(height, 842.0f, 1.0f);
}

TEST(pdf, begin_text)
{
    hj::pdf::document doc;
    auto              page = doc.add_page();
    EXPECT_TRUE(page.begin_text());
    EXPECT_TRUE(page.end_text());
}

TEST(pdf, end_text)
{
    hj::pdf::document doc;
    auto              page = doc.add_page();
    EXPECT_TRUE(page.begin_text());
    EXPECT_TRUE(page.end_text());
}

TEST(pdf, set_font)
{
    hj::pdf::document doc;
    auto              page = doc.add_page();
    EXPECT_TRUE(page.set_font(hj::pdf::font_name::helvetica, 12));
    EXPECT_TRUE(page.set_font("Helvetica", 12));
}

TEST(pdf, set_text_color)
{
    hj::pdf::document doc;
    auto              page = doc.add_page();
    hj::pdf::color    text_color(0.0f, 0.0f, 0.0f);
    EXPECT_TRUE(page.set_text_color(text_color));
}

TEST(pdf, set_stroke_color)
{
    hj::pdf::document doc;
    auto              page = doc.add_page();
    hj::pdf::color    stroke_color(1.0f, 0.0f, 0.0f);
    EXPECT_TRUE(page.set_stroke_color(stroke_color));
}

TEST(pdf, set_line_width)
{
    hj::pdf::document doc;
    auto              page = doc.add_page();
    EXPECT_TRUE(page.set_line_width(1.0f));
}

TEST(pdf, move_to)
{
    hj::pdf::document doc;
    auto              page = doc.add_page();
    EXPECT_TRUE(page.move_to(hj::pdf::point(100.0f, 200.0f)));
}

TEST(pdf, line_to)
{
    hj::pdf::document doc;
    auto              page = doc.add_page();
    EXPECT_TRUE(page.move_to(hj::pdf::point(100.0f, 200.0f)));
    EXPECT_TRUE(page.line_to(hj::pdf::point(200.0f, 300.0f)));
}

TEST(pdf, stroke)
{
    hj::pdf::document doc;
    auto              page = doc.add_page();
    EXPECT_TRUE(page.move_to(hj::pdf::point(100.0f, 200.0f)));
    EXPECT_TRUE(page.line_to(hj::pdf::point(200.0f, 300.0f)));
    EXPECT_TRUE(page.stroke());
}

TEST(pdf, fill)
{
    hj::pdf::document doc;
    auto              page = doc.add_page();
    EXPECT_TRUE(page.move_to(hj::pdf::point(100.0f, 200.0f)));
    EXPECT_TRUE(page.line_to(hj::pdf::point(200.0f, 200.0f)));
    EXPECT_TRUE(page.line_to(hj::pdf::point(200.0f, 300.0f)));
    EXPECT_TRUE(page.line_to(hj::pdf::point(100.0f, 300.0f)));
    EXPECT_TRUE(page.close_path());
    EXPECT_TRUE(page.fill());
}

TEST(pdf, set_text_position)
{
    hj::pdf::document doc;
    auto              page = doc.add_page();
    EXPECT_TRUE(page.begin_text());
    EXPECT_TRUE(page.set_text_position(hj::pdf::point(100.0f, 200.0f)));
    EXPECT_TRUE(page.end_text());
}

TEST(pdf, show_text)
{
    hj::pdf::document doc;
    auto              page = doc.add_page();
    EXPECT_TRUE(page.set_font(hj::pdf::font_name::helvetica, 12));
    EXPECT_TRUE(page.begin_text());
    EXPECT_TRUE(page.show_text("Hello, PDF!"));
    EXPECT_TRUE(page.end_text());
}

TEST(pdf, show_text_at)
{
    hj::pdf::document doc;
    auto              page = doc.add_page();
    EXPECT_TRUE(page.set_font(hj::pdf::font_name::helvetica, 12));
    EXPECT_TRUE(page.begin_text());
    EXPECT_TRUE(
        page.show_text_at("Hello, PDF!", hj::pdf::point(100.0f, 200.0f)));
    EXPECT_TRUE(page.end_text());
}

TEST(pdf, draw_rectangle)
{
    hj::pdf::document doc;
    auto              page = doc.add_page();
    EXPECT_TRUE(
        page.draw_rectangle(hj::pdf::rectangle(100.0f, 200.0f, 50.0f, 30.0f),
                            true));
}

TEST(pdf, draw_circle)
{
    hj::pdf::document doc;
    auto              page = doc.add_page();
    EXPECT_TRUE(page.draw_circle(hj::pdf::point(150.0f, 250.0f), 25.0f, true));
}

TEST(pdf, draw_ellipse)
{
    hj::pdf::document doc;
    auto              page = doc.add_page();
    EXPECT_TRUE(
        page.draw_ellipse(hj::pdf::point(150.0f, 250.0f), 25.0f, 15.0f, true));
}

TEST(pdf, draw_line)
{
    hj::pdf::document doc;
    auto              page = doc.add_page();
    EXPECT_TRUE(page.draw_line(hj::pdf::point(100.0f, 200.0f),
                               hj::pdf::point(200.0f, 300.0f)));
}

TEST(pdf, draw_text)
{
    hj::pdf::document doc;
    auto              page = doc.add_page();
    EXPECT_TRUE(page.set_font(hj::pdf::font_name::helvetica, 12));
    EXPECT_TRUE(page.begin_text());
    EXPECT_TRUE(page.draw_text("Hello, PDF!", hj::pdf::point(100.0f, 200.0f)));
    EXPECT_TRUE(page.end_text());
}

TEST(pdf, draw_text_aligned)
{
    hj::pdf::document doc;
    auto              page = doc.add_page();
    EXPECT_TRUE(page.set_font(hj::pdf::font_name::helvetica, 12));
    EXPECT_TRUE(page.begin_text());
    EXPECT_TRUE(page.draw_text_aligned("Centered Text",
                                       hj::pdf::rectangle(100, 700, 200, 50),
                                       hj::pdf::text_align::center));
    EXPECT_TRUE(page.end_text());
}

TEST(pdf, get_text_width)
{
    hj::pdf::document doc;
    auto              page = doc.add_page();
    EXPECT_TRUE(page.set_font(hj::pdf::font_name::helvetica, 12));

    float width = page.get_text_width("Hello, PDF!");
    EXPECT_GT(width, 0.0f);
}

TEST(pdf, add_page)
{
    hj::pdf::document doc;
    auto              page1 = doc.add_page();
    auto              page2 = doc.add_page();
    EXPECT_EQ(doc.get_page_count(), 2);
    ASSERT_FALSE(page1.is_same(page2));
}

TEST(pdf, get_page)
{
    hj::pdf::document doc;
    auto              page1 = doc.add_page();
    auto              page2 = doc.add_page();
    ASSERT_TRUE(doc.get_page(0).is_same(page1));
    ASSERT_TRUE(doc.get_page(1).is_same(page2));
}

TEST(pdf, get_page_count)
{
    hj::pdf::document doc;
    doc.add_page();
    doc.add_page();
    EXPECT_EQ(doc.get_page_count(), 2);
}

TEST(pdf, set_title)
{
    hj::pdf::document doc;
    EXPECT_NO_THROW(doc.set_title("Test Document"));
}

TEST(pdf, set_author)
{
    hj::pdf::document doc;
    EXPECT_NO_THROW(doc.set_author("John Doe"));
}

TEST(pdf, set_subject)
{
    hj::pdf::document doc;
    EXPECT_NO_THROW(doc.set_subject("PDF Testing"));
}

TEST(pdf, set_keywords)
{
    hj::pdf::document doc;
    EXPECT_NO_THROW(doc.set_keywords("PDF, Testing"));
}

TEST(pdf, set_creator)
{
    hj::pdf::document doc;
    EXPECT_NO_THROW(doc.set_creator("PDF Library"));
}

TEST(pdf, set_producer)
{
    hj::pdf::document doc;
    EXPECT_NO_THROW(doc.set_producer("PDF Producer"));
}

TEST(pdf, set_password)
{
    hj::pdf::document doc;
    EXPECT_NO_THROW(doc.set_password("owner_password", "user_password"));
}

TEST(pdf, set_permission)
{
    hj::pdf::document doc;
    doc.set_password("owner_password", "user_password");
    EXPECT_NO_THROW(doc.set_permission(hj::pdf::document::permission::print));
}

TEST(pdf, add_permission)
{
    hj::pdf::document doc;
    doc.set_password("owner_password", "user_password");
    doc.set_permission(hj::pdf::document::permission::none);
    EXPECT_NO_THROW(doc.add_permission(hj::pdf::document::permission::print));
}

TEST(pdf, get_permission)
{
    hj::pdf::document doc;
    doc.set_password("owner_password", "user_password");
    doc.set_permission(hj::pdf::document::permission::print);
    ASSERT_TRUE(doc.get_permission() == hj::pdf::document::permission::print);
}

TEST(pdf, clear_all_permissions)
{
    hj::pdf::document doc;
    doc.set_password("owner_password", "user_password");
    doc.set_permission(hj::pdf::document::permission::all);
    doc.clear_all_permissions();
    ASSERT_TRUE(doc.get_permission() == hj::pdf::document::permission::none);
}

TEST(pdf, save_to_file)
{
    hj::pdf::document doc;
    doc.set_title("Test Document");
    doc.set_author("John Doe");
    doc.set_subject("PDF Testing");
    doc.set_keywords("PDF, Testing");
    doc.set_creator("PDF Library");
    doc.set_producer("PDF Producer");
    doc.set_password("owner_password", "user_password");
    doc.set_permission(hj::pdf::document::permission::print);

    auto page = doc.add_page();
    EXPECT_TRUE(page.set_font(hj::pdf::font_name::helvetica, 12));
    EXPECT_TRUE(page.begin_text());
    EXPECT_TRUE(page.show_text_at("Test Content", hj::pdf::point(100, 700)));
    EXPECT_TRUE(page.end_text());

    ASSERT_TRUE(doc.save("test_document.pdf"));
}

TEST(pdf, save_to_memory)
{
    hj::pdf::document doc;
    doc.set_title("Test Document");

    auto page = doc.add_page();
    EXPECT_TRUE(page.set_font(hj::pdf::font_name::helvetica, 12));
    EXPECT_TRUE(page.begin_text());
    EXPECT_TRUE(page.show_text_at("Test Content", hj::pdf::point(100, 700)));
    EXPECT_TRUE(page.end_text());

    std::vector<unsigned char> buffer;
    EXPECT_TRUE(doc.save(buffer));
    EXPECT_GT(buffer.size(), 0);

    ASSERT_GE(buffer.size(), 4);
    EXPECT_EQ(buffer[0], '%');
    EXPECT_EQ(buffer[1], 'P');
    EXPECT_EQ(buffer[2], 'D');
    EXPECT_EQ(buffer[3], 'F');
}

TEST(pdf, save_to_ostream)
{
    hj::pdf::document doc;
    doc.set_title("OStream Test");

    auto page = doc.add_page();
    EXPECT_TRUE(page.set_font(hj::pdf::font_name::helvetica, 12));
    EXPECT_TRUE(page.begin_text());
    EXPECT_TRUE(page.show_text_at("Stream Content", hj::pdf::point(100, 700)));
    EXPECT_TRUE(page.end_text());

    std::stringstream ss;
    EXPECT_TRUE(doc.save(ss));
    std::string pdf_data = ss.str();
    EXPECT_GT(pdf_data.size(), 0);
    ASSERT_GE(pdf_data.size(), 4);
    EXPECT_EQ(pdf_data[0], '%');
    EXPECT_EQ(pdf_data[1], 'P');
    EXPECT_EQ(pdf_data[2], 'D');
    EXPECT_EQ(pdf_data[3], 'F');
}