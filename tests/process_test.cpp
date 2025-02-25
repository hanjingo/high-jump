#include <gtest/gtest.h>
#include <libcpp/os/process.hpp>

// TEST(process, is_running)
// {
//     ASSERT_EQ(libcpp::process::is_running(libcpp::process::getpid()), true);
// }

TEST(process, child)
{
    ASSERT_EQ(libcpp::process::child("ping").valid(), true);
}

TEST(process, spawn)
{
    libcpp::process::spawn("ping");
}

TEST(process, system)
{
    libcpp::process::error_code_t ec;
    libcpp::process::system("ping", ec);
    ASSERT_EQ(ec.value() == 0, true);
}

TEST(process, getpid)
{
    ASSERT_EQ(libcpp::process::getpid() > -1, true);
}

TEST(process, getppid)
{
    ASSERT_EQ(libcpp::process::getppid() > -1, true);
}

TEST(process, search_path)
{
    ASSERT_EQ(!boost::process::search_path("/bin/ping", {"/bin"}).empty(), true);
}