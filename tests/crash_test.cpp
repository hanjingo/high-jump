#include <gtest/gtest.h>
#include <hj/testing/crash.hpp>

TEST(crash, set_local_path)
{
    hj::crash_handler::instance()->prevent_set_unhandled_exception_filter();
    hj::crash_handler::instance()->set_local_path("./");
}
