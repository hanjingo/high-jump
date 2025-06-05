#include <gtest/gtest.h>
#include <libcpp/os/process.hpp>

TEST(process, getpid)
{
    ASSERT_EQ(libcpp::process::getpid() >= 0, true);
}

TEST(process, getppid)
{
#if defined(_WIN32)
    ASSERT_EQ(libcpp::process::getppid() == 0, true);
#else
    ASSERT_EQ(libcpp::process::getppid() > -1, true);
#endif
}

TEST(process, system)
{
    libcpp::process::error_code_t ec;
    libcpp::process::pstream_t p;
    libcpp::process::system("child", libcpp::process::std_out > p, ec);
    std::string ret;
    p >> ret;
    ASSERT_EQ(ec.value() == 0, true);
//    ASSERT_EQ(ret == std::string("hello"), true);
}

TEST(process, child)
{
    libcpp::process::error_code_t ec;
    libcpp::process::pstream_t p;
    auto child = libcpp::process::child("child", libcpp::process::std_out > p, ec);
    std::string ret;
    p >> ret;
    child.wait();
    ASSERT_EQ(ec.value() == 0, true);
//    ASSERT_EQ(ret == std::string("hello"), true);
}

TEST(process, spawn)
{
    libcpp::process::spawn("child");
}

TEST(process, daemon)
{
}

TEST(process, group)
{
}

TEST(process, list)
{
    libcpp::process::spawn("child");
    std::vector<libcpp::process::pid_t> vec;
    libcpp::process::list(vec);
    ASSERT_EQ(vec.empty(), false);
}

TEST(process, kill)
{
    libcpp::process::spawn("child");
    std::vector<libcpp::process::pid_t> vec;
    libcpp::process::list(vec, [](std::vector<std::string> arg) -> bool{
        if (arg.size() < 2)
            return false;

        if (arg[0] != "child")
            return false;
            
        return true;
    });
    for (auto var : vec) {
        libcpp::process::kill(var);
    }

    vec.clear();
    libcpp::process::list(vec, [](std::vector<std::string> arg) -> bool{
        if (arg.size() < 2)
            return false;

        if (arg[0] != "child")
            return false;
            
        return true;
    });
    ASSERT_EQ(vec.empty(), true);
}