#include <gtest/gtest.h>
#include <libcpp/os/process.hpp>
#include <thread>
#include <chrono>
#include <fstream>

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

TEST(process, child)
{
#if defined(_WIN32)
    std::string exe = "./child.exe";
#else
    std::string exe = "./child";
#endif
    auto pid = libcpp::process::child(exe);
    ASSERT_GT(pid, 0);
#if !defined(_WIN32)
    int status = 0;
    waitpid(pid, &status, 0);
    ASSERT_TRUE(WIFEXITED(status));
#endif
}

TEST(process, daemon)
{
#if defined(_WIN32)
    std::string exe = "./daemon.exe";
#else
    std::string exe = "./daemon";
#endif
    libcpp::process::daemon(exe);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::ifstream fin("daemon_test.txt");
    ASSERT_TRUE(fin.is_open());
    std::string line;
    std::getline(fin, line);
    fin.close();
    ASSERT_TRUE(line.find("daemon") != std::string::npos);
    std::remove("daemon_test.txt");

    std::vector<libcpp::process::pid_t> vec;
    libcpp::process::list(vec, [](std::vector<std::string> arg) -> bool{
        if (arg.size() < 2)
            return false;
        if (arg[0].find("daemon") == std::string::npos)
            return false;
        return true;
    });
    for (auto pid : vec) {
        libcpp::process::kill(pid);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

TEST(process, spawn)
{
#if defined(_WIN32)
    std::string exe = "./child.exe";
#else
    std::string exe = "./child";
#endif
    libcpp::process::spawn(exe);
}

TEST(process, list)
{
#if defined(_WIN32)
    std::string exe = "./child.exe";
#else
    std::string exe = "./child";
#endif

    // clear env
    std::vector<libcpp::process::pid_t> vec;
    libcpp::process::list(vec, [](std::vector<std::string> arg) -> bool{
        if (arg.size() < 2)
            return false;
        if (arg[0].find("child") == std::string::npos)
            return false;
        return true;
    });
    for (auto var : vec) {
        libcpp::process::kill(var);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // test start
    vec.clear();
    libcpp::process::spawn(exe);
    libcpp::process::list(vec, [](std::vector<std::string> arg) -> bool{
        if (arg.size() < 2)
            return false;
        if (arg[0].find("child") == std::string::npos)
            return false;
        return true;
    });
    std::cout << "Process list: ";
    for (auto pid : vec) {
        std::cout << pid << " ";
    }
    std::cout << std::endl;
    ASSERT_EQ(vec.size(), 1);
}

TEST(process, kill)
{
#if defined(_WIN32)
    std::string exe = "./child.exe";
#else
    std::string exe = "./child";
#endif
    libcpp::process::spawn(exe);
    std::vector<libcpp::process::pid_t> vec;
    libcpp::process::list(vec, [](std::vector<std::string> arg) -> bool{
        if (arg.size() < 2)
            return false;
        if (arg[0].find("child") == std::string::npos)
            return false;
        return true;
    });
    for (auto var : vec) {
        libcpp::process::kill(var);
    }
    vec.clear();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    libcpp::process::list(vec, [](std::vector<std::string> arg) -> bool{
        if (arg.size() < 2)
            return false;
        if (arg[0].find("child") == std::string::npos)
            return false;
        return true;
    });
    ASSERT_EQ(vec.empty(), true);
}