#include <iostream>
#include <thread>

#ifdef __linux__
#include <unistd.h>
#include <sys/wait.h>
#endif

int glob1 = 0;
int glob2 = 0;

int main(int argc, char* argv[])
{
        // linux
#ifdef __linux__
    int var1 = 0;
    int var2 = 0;

    pid_t pid1;
    if ((pid1 = fork()) < 0) 
        std::cout << "fork fail" << std::endl;
    var1++;
    glob1++;
    std::cout << "fork process with pid1=" << pid1 << ", var=" 
              << var1  << ", glob1=" << glob1 << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    waitpid(pid1, NULL, WNOHANG);

    pid_t pid2;
    if ((pid2 = vfork()) < 0)
        std::cout << "vfork fail" << std::endl;
    std::cout << "vfork process with pid2=" << pid2 << ", var=" 
              << var2  << ", glob2=" << glob2 << std::endl;
    waitpid(pid2, NULL, WUNTRACED);

    //execl("/bin/sh", "echo", "execl echo hello");
    system("echo system:hello");

#endif

    std::cin.get();
    return 0;
}