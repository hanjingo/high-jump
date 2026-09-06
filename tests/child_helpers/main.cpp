// child_helpers/main.cpp
#include <csignal>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#if defined(_WIN32)
#include <direct.h>
#include <windows.h>
#define getcwd _getcwd

BOOL WINAPI console_handler(DWORD signal)
{
    if(signal == CTRL_BREAK_EVENT || signal == CTRL_C_EVENT)
    {
        std::exit(0);
    }
    return TRUE;
}
#else
#include <unistd.h>

void sigterm_handler(int sig)
{
    ::_exit(0);
}
#endif

int main(int argc, char *argv[])
{
    if(argc < 2)
        return 0;
    std::string mode = argv[1];

    if(mode == "--sleep")
    {
#if defined(_WIN32)
        SetConsoleCtrlHandler(console_handler, TRUE);
#else
        struct sigaction sa{};
        sa.sa_handler = sigterm_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        ::sigaction(SIGTERM, &sa, nullptr);
#endif

        std::cout << "READY" << std::endl;

        for(int i = 0; i < 100; ++i)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    } else if(mode == "--quick-exit")
    {
        return 0;
    } else if(mode == "--raise-sigterm")
    {
#if !defined(_WIN32)
        std::raise(SIGTERM);
#else
        return 128 + 15;
#endif
    } else if(mode == "--print-cwd")
    {
        char buf[1024];
        if(getcwd(buf, sizeof(buf)))
        {
            std::ofstream fout("cwd_out.txt");
            fout << buf;
        }
    } else if(mode == "--check-stdin")
    {
        std::string input;
        if(!(std::cin >> input))
        {
            std::ofstream fout("stdin_out.txt");
            fout << "EOF_REACHED";
        }
    }
    return 0;
}