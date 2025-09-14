#include <gtest/gtest.h>
#include <libcpp/log/logger.hpp>

TEST(logger, instance)
{
    ASSERT_EQ(libcpp::logger::instance() != nullptr, true);
}

TEST(logger, create_stdout_sink)
{
    ASSERT_EQ(libcpp::logger::create_stdout_sink() != nullptr, true);
}

TEST(logger, create_rotate_file_sink)
{
    ASSERT_EQ(libcpp::logger::create_rotate_file_sink("./007.log", 512 * 1024 * 1024, 3, true) != nullptr, true);
}

TEST(logger, create_daily_file_sink)
{
    ASSERT_EQ(libcpp::logger::create_daily_file_sink("./007.log", 1, 1, true, 2) != nullptr, true);
}

TEST(logger, add_sink)
{
    libcpp::logger::instance()->add_sink(libcpp::logger::create_stdout_sink());
}

TEST(logger, remove_sink)
{
}

TEST(logger, clear_sink)
{
    libcpp::logger::instance()->clear_sink();
}

TEST(logger, set_level)
{
    libcpp::logger::instance()->set_level(libcpp::log_lvl::info);
    ASSERT_EQ(libcpp::logger::instance()->get_level() == libcpp::log_lvl::info, true);
}

TEST(logger, get_level)
{
}

TEST(logger, set_pattern)
{

}

TEST(logger, flush)
{
    libcpp::logger::instance()->flush();
}

TEST(logger, flush_on)
{
    libcpp::logger::instance()->clear_sink();
    libcpp::logger::instance()->add_sink(libcpp::logger::create_stdout_sink());
    libcpp::logger::instance()->info("test flush_on with debug lvl");
    libcpp::logger::instance()->info("test flush_on with info lvl");
    libcpp::logger::instance()->flush_on(libcpp::log_lvl::info);
}

TEST(logger, trace)
{
    libcpp::logger::instance()->clear_sink();
    libcpp::logger::instance()->add_sink(libcpp::logger::create_stdout_sink());
}

TEST(logger, debug)
{

}

TEST(logger, info)
{

}

TEST(logger, warn)
{

}

TEST(logger, error)
{

}

TEST(logger, critical)
{

}