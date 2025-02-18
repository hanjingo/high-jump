#include <gtest/gtest.h>
#include <libcpp/testing/crash.hpp>

TEST(crash, set_local_path)
{
    libcpp::crash_handler::instance()->set_local_path("./");
}
