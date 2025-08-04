#include <gtest/gtest.h>
#include <libcpp/os/signal.hpp>

static int signal_num = 0;

void handler1 (int sig)
{
    (void) sig;
    signal_num += 1;
}

void handler2 (int sig)
{
    (void) sig;
    signal_num += 2;
}

TEST (signal, sigcatch)
{
    libcpp::_sigcatch (SIGABRT, std::bind (handler1, std::placeholders::_1),
                       true);

    libcpp::sigcatch ({SIGILL, SIGTERM},
                      std::bind (handler2, std::placeholders::_1), true);

    libcpp::sig_raise (SIGABRT);
    ASSERT_EQ (signal_num, 1);
    libcpp::sig_raise (SIGABRT);
    ASSERT_EQ (signal_num, 1);

    libcpp::sig_raise (SIGILL);
    ASSERT_EQ (signal_num, 3);
    libcpp::sig_raise (SIGTERM);
    ASSERT_EQ (signal_num, 5);

    libcpp::sig_raise (SIGILL, SIGTERM);
    ASSERT_EQ (signal_num, 5);
}