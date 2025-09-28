#include <gtest/gtest.h>
#include <hj/log/logger.hpp>

TEST(logger, instance)
{
    ASSERT_EQ(hj::logger::instance() != nullptr, true);
}

TEST(logger, create_stdout_sink)
{
    ASSERT_EQ(hj::logger::create_stdout_sink() != nullptr, true);
}

TEST(logger, create_rotate_file_sink)
{
    ASSERT_EQ(hj::logger::create_rotate_file_sink("./007.log",
                                                  512 * 1024 * 1024,
                                                  3,
                                                  true)
                  != nullptr,
              true);
}

TEST(logger, create_daily_file_sink)
{
    ASSERT_EQ(hj::logger::create_daily_file_sink("./007.log", 1, 1, true, 2)
                  != nullptr,
              true);
}

TEST(logger, add_sink)
{
    hj::logger::instance()->add_sink(hj::logger::create_stdout_sink());
}

TEST(logger, remove_sink)
{
}

TEST(logger, clear_sink)
{
    hj::logger::instance()->clear_sink();
}

TEST(logger, set_level)
{
    hj::logger::instance()->set_level(hj::log_lvl::info);
    ASSERT_EQ(hj::logger::instance()->get_level() == hj::log_lvl::info, true);
}

TEST(logger, get_level)
{
}

TEST(logger, set_pattern)
{
}

TEST(logger, flush)
{
    hj::logger::instance()->flush();
}

TEST(logger, flush_on)
{
    hj::logger::instance()->clear_sink();
    hj::logger::instance()->add_sink(hj::logger::create_stdout_sink());
    hj::logger::instance()->info("test flush_on with debug lvl");
    hj::logger::instance()->info("test flush_on with info lvl");
    hj::logger::instance()->flush_on(hj::log_lvl::info);
}

TEST(logger, trace)
{
    hj::logger::instance()->clear_sink();
    hj::logger::instance()->add_sink(hj::logger::create_stdout_sink());
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