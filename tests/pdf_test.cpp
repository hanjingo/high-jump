#include <gtest/gtest.h>
#include <libcpp/misc/pdf.hpp>

TEST(pdf, set_size)
{
    libcpp::pdf::document doc;
    auto& page = doc.add_page();
    page.set_size(libcpp::pdf::page::size::a4);
}

TEST(pdf, get_size)
{
    libcpp::pdf::document doc;
    auto& page = doc.add_page();
    page.set_size(libcpp::pdf::page::size::a4);
    float width, height;
    page.get_size(width, height);
    EXPECT_NEAR(width, 595.0f, 1.0f);
    EXPECT_NEAR(height, 842.0f, 1.0f);
}

TEST(pdf, get_width)
{
    libcpp::pdf::document doc;
    auto& page = doc.add_page();
    page.set_size(libcpp::pdf::page::size::a4);
    auto width = page.get_width();
    EXPECT_NEAR(width, 595.0f, 1.0f); // A4 width in points
}

TEST(pdf, get_height)
{
    libcpp::pdf::document doc;
    auto& page = doc.add_page();
    page.set_size(libcpp::pdf::page::size::a4);
    auto height = page.get_height();
    EXPECT_NEAR(height, 842.0f, 1.0f); // A4 height in points
}

TEST(pdf, begin_text)
{
    libcpp::pdf::document doc;
    auto& page = doc.add_page();
    page.begin_text();
    page.end_text(); // Ensure begin_text can be called without issues
}

TEST(pdf, end_text)
{
    libcpp::pdf::document doc;
    auto& page = doc.add_page();
    page.begin_text();
    page.end_text();
}

TEST(pdf, set_font)
{
    libcpp::pdf::document doc;
    auto& page = doc.add_page();
    EXPECT_NO_THROW(page.set_font(libcpp::pdf::font_name::helvetica, 12));
    EXPECT_NO_THROW(page.set_font("Helvetica", 12));
}

TEST(pdf, set_text_color)
{
    libcpp::pdf::document doc;
    auto& page = doc.add_page();
    libcpp::pdf::color text_color(0.0f, 0.0f, 0.0f); // Black
    EXPECT_NO_THROW(page.set_text_color(text_color));
}

TEST(pdf, set_stroke_color)
{
    libcpp::pdf::document doc;
    auto& page = doc.add_page();
    libcpp::pdf::color stroke_color(1.0f, 0.0f, 0.0f); // Red
    EXPECT_NO_THROW(page.set_stroke_color(stroke_color));
}

TEST(pdf, set_line_width)
{
    libcpp::pdf::document doc;
    auto& page = doc.add_page();
    EXPECT_NO_THROW(page.set_line_width(1.0f));
}

TEST(pdf, move_to)
{
    libcpp::pdf::document doc;
    auto& page = doc.add_page();
    EXPECT_NO_THROW(page.move_to(libcpp::pdf::point(100.0f, 200.0f)));
}

TEST(pdf, line_to)
{
    libcpp::pdf::document doc;
    auto& page = doc.add_page();
    EXPECT_NO_THROW(page.move_to(libcpp::pdf::point(100.0f, 200.0f)));
    EXPECT_NO_THROW(page.line_to(libcpp::pdf::point(200.0f, 300.0f)));
}

TEST(pdf, stroke)
{
    libcpp::pdf::document doc;
    auto& page = doc.add_page();
    EXPECT_NO_THROW(page.move_to(libcpp::pdf::point(100.0f, 200.0f)));
    EXPECT_NO_THROW(page.line_to(libcpp::pdf::point(200.0f, 300.0f)));
    EXPECT_NO_THROW(page.stroke());
}

TEST(pdf, fill)
{
    libcpp::pdf::document doc;
    auto& page = doc.add_page();
    EXPECT_NO_THROW(page.move_to(libcpp::pdf::point(100.0f, 200.0f)));
    EXPECT_NO_THROW(page.line_to(libcpp::pdf::point(200.0f, 200.0f)));
    EXPECT_NO_THROW(page.line_to(libcpp::pdf::point(200.0f, 300.0f)));
    EXPECT_NO_THROW(page.line_to(libcpp::pdf::point(100.0f, 300.0f)));
    EXPECT_NO_THROW(page.close_path());
    EXPECT_NO_THROW(page.fill());
}

TEST(pdf, set_text_position)
{
    libcpp::pdf::document doc;
    auto& page = doc.add_page();
    page.begin_text();
    page.set_text_position(libcpp::pdf::point(100.0f, 200.0f));
    page.end_text();
}

TEST(pdf, show_text)
{
    libcpp::pdf::document doc;
    auto& page = doc.add_page();
    page.set_font(libcpp::pdf::font_name::helvetica, 12);
    page.begin_text();
    EXPECT_NO_THROW(page.show_text("Hello, PDF!"));
    page.end_text();
}

TEST(pdf, show_text_at)
{
    libcpp::pdf::document doc;
    auto& page = doc.add_page();
    page.set_font(libcpp::pdf::font_name::helvetica, 12);
    page.begin_text();
    EXPECT_NO_THROW(page.show_text_at("Hello, PDF!", libcpp::pdf::point(100.0f, 200.0f)));
    page.end_text();
}

TEST(pdf, draw_rectangle)
{
    libcpp::pdf::document doc;
    auto& page = doc.add_page();
    EXPECT_NO_THROW(page.draw_rectangle(libcpp::pdf::rectangle(100.0f, 200.0f, 50.0f, 30.0f), true));
}

TEST(pdf, draw_circle)
{
    libcpp::pdf::document doc;
    auto& page = doc.add_page();
    EXPECT_NO_THROW(page.draw_circle(libcpp::pdf::point(150.0f, 250.0f), 25.0f, true));
}

TEST(pdf, draw_ellipse)
{
    libcpp::pdf::document doc;
    auto& page = doc.add_page();
    page.draw_ellipse(libcpp::pdf::point(150.0f, 250.0f), 25.0f, 15.0f, true);
}

TEST(pdf, draw_line)
{
    libcpp::pdf::document doc;
    auto& page = doc.add_page();
    page.draw_line(libcpp::pdf::point(100.0f, 200.0f), libcpp::pdf::point(200.0f, 300.0f), true);
}

TEST(pdf, draw_text)
{
    libcpp::pdf::document doc;
    auto& page = doc.add_page();
    page.set_font(libcpp::pdf::font_name::helvetica, 12);
    page.begin_text();
    EXPECT_NO_THROW(page.draw_text("Hello, PDF!", libcpp::pdf::point(100.0f, 200.0f)));
    page.end_text();
}

TEST(pdf, set_text_alignment)
{
    libcpp::pdf::document doc;
    auto& page = doc.add_page();
    page.set_font(libcpp::pdf::font_name::helvetica, 12);
    page.begin_text();
    EXPECT_NO_THROW(page.set_text_alignment(libcpp::pdf::text_align::center));
    page.show_text("Centered Text");
    page.end_text();
}

TEST(pdf, draw_image)
{
    GTEST_SKIP() << "Image file not available for testing";

    // libcpp::pdf::document doc;
    // auto& page = doc.add_page();
    // if (std::filesystem::exists("test_image.png"))
    // {
    //     EXPECT_NO_THROW(page.draw_image("test_image.png", libcpp::pdf::rectangle(100.0f, 200.0f, 50.0f, 50.0f)));
    //     EXPECT_NO_THROW(page.draw_image("test_image.png", libcpp::pdf::point(100.0f, 200.0f), 50.0f, 50.0f));
    // }
}

TEST(pdf, get_text_width)
{
    libcpp::pdf::document doc;
    auto& page = doc.add_page();
    page.set_font(libcpp::pdf::font_name::helvetica, 12);
    
    float width = 0.0f;
    EXPECT_NO_THROW(width = page.get_text_width("Hello, PDF!"));
    EXPECT_GT(width, 0.0f);
}

TEST(pdf, insert_image)
{
    GTEST_SKIP() << "Image file not available for testing";
    
    // libcpp::pdf::document doc;
    // auto& page = doc.add_page();
    // page.insert_image("image.png", libcpp::pdf::rectangle(100.0f, 200.0f, 50.0f, 50.0f));
    // page.insert_image("image.png", libcpp::pdf::point(100.0f, 200.0f), 50.0f, 50.0f);
}

TEST(pdf, add_page)
{
    libcpp::pdf::document doc;
    auto& page1 = doc.add_page();
    auto& page2 = doc.add_page();
    EXPECT_EQ(doc.get_page_count(), 2);
    EXPECT_NE(&page1, &page2);
    ASSERT_FALSE(page1.is_same(page2));
}

TEST(pdf, get_page)
{
    libcpp::pdf::document doc;
    auto& page1 = doc.add_page();
    auto& page2 = doc.add_page();
    ASSERT_TRUE(doc.get_page(0).is_same(page1));
    ASSERT_TRUE(doc.get_page(1).is_same(page2));
}

TEST(pdf, get_page_count)
{
    libcpp::pdf::document doc;
    auto& page1 = doc.add_page();
    auto& page2 = doc.add_page();
    EXPECT_EQ(doc.get_page_count(), 2);
}

TEST(pdf, set_title)
{
    libcpp::pdf::document doc;
    doc.set_title("Test Document");
}

TEST(pdf, set_author)
{
    libcpp::pdf::document doc;
    doc.set_author("John Doe");
}

TEST(pdf, set_subject)
{
    libcpp::pdf::document doc;
    doc.set_subject("PDF Testing");
}

TEST(pdf, set_keywords)
{
    libcpp::pdf::document doc;
    doc.set_keywords("PDF, Testing");
}

TEST(pdf, set_creator)
{
    libcpp::pdf::document doc;
    doc.set_creator("PDF Library");
}

TEST(pdf, set_producer)
{
    libcpp::pdf::document doc;
    doc.set_producer("PDF Producer");
}

TEST(pdf, set_password)
{
    libcpp::pdf::document doc;
    doc.set_password("user_password", "owner_password");
}

TEST(pdf, set_permission)
{
    libcpp::pdf::document doc;
    doc.set_password("user_password", "owner_password"); // Set password before setting permissions
    doc.set_permission(libcpp::pdf::document::permission::print);
}

TEST(pdf, add_permission)
{
    libcpp::pdf::document doc;
    doc.set_password("user_password", "owner_password"); // Set password before setting permissions
    doc.set_permission(libcpp::pdf::document::permission::none);
    doc.add_permission(libcpp::pdf::document::permission::print);
}

TEST(pdf, get_permission)
{
    libcpp::pdf::document doc;
    doc.set_password("user_password", "owner_password"); // Set password before setting permissions
    doc.set_permission(libcpp::pdf::document::permission::print);
    ASSERT_TRUE(doc.get_permission() == libcpp::pdf::document::permission::print);
}

TEST(pdf, clear_all_permissions)
{
    libcpp::pdf::document doc;
    doc.set_password("user_password", "owner_password"); // Set password before setting permissions
    doc.set_permission(libcpp::pdf::document::permission::all);
    doc.clear_all_permissions();
    ASSERT_TRUE(doc.get_permission() == libcpp::pdf::document::permission::none);
}

TEST(pdf, save_to_file)
{
    libcpp::pdf::document doc;
    doc.set_title("Test Document");
    doc.set_author("John Doe");
    doc.set_subject("PDF Testing");
    doc.set_keywords("PDF, Testing");
    doc.set_creator("PDF Library");
    doc.set_producer("PDF Producer");
    doc.set_password("user_password", "owner_password");
    doc.set_permission(libcpp::pdf::document::permission::print);

    // Save to a file
    ASSERT_TRUE(doc.save_to_file("test_document.pdf"));
}

TEST(pdf, save_to_memory)
{
    libcpp::pdf::document doc;
    doc.set_title("Test Document");
    doc.set_author("John Doe");
    doc.set_subject("PDF Testing");
    doc.set_keywords("PDF, Testing");
    doc.set_creator("PDF Library");
    doc.set_producer("PDF Producer");

    auto& page = doc.add_page();
    page.set_font(libcpp::pdf::font_name::helvetica, 12);
    page.begin_text();
    page.show_text_at("Test Content", libcpp::pdf::point(100, 700));
    page.end_text();
    
    std::vector<unsigned char> buffer;
    EXPECT_NO_THROW(doc.save_to_memory(buffer));
    EXPECT_GT(buffer.size(), 0);
    EXPECT_LT(buffer.size(), 1024 * 1024);
    
    ASSERT_GE(buffer.size(), 4);
    EXPECT_EQ(buffer[0], '%');
    EXPECT_EQ(buffer[1], 'P');
    EXPECT_EQ(buffer[2], 'D');
    EXPECT_EQ(buffer[3], 'F');
}