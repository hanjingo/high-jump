#include <gtest/gtest.h>
#include <libcpp/log/logger.hpp>

TEST(logger, instance)
{
    ASSERT_EQ(libcpp::logger::instance() != nullptr, true);
}

TEST(logger, set_default)
{

}

TEST(logger, creat_sink)
{
}

TEST(logger, add_sink)
{

}

TEST(logger, set_level)
{

}

TEST(logger, get_level)
{

}

TEST(logger, set_pattern)
{

}

TEST(logger, flush)
{

}

TEST(logger, flush_on)
{

}

TEST(logger, trace)
{

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