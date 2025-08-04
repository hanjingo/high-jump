#include <iostream>

#include <libcpp/os/process.hpp>

int main(int argc, char* argv[])
{
    std::cout << "getpid() = " << libcpp::process::getpid() << std::endl;

    std::cout << "getppid() = " << libcpp::process::getppid() << std::endl;

    std::cout << "\nrun system>>" << std::endl;
    libcpp::process::error_code_t err;
    libcpp::process::system("child", err);
    std::cout << "system(\"child\", err); err = " << err.message() << std::endl;

    std::cout << "\nrun child>>" << std::endl;
    libcpp::process::pstream_t p;
    auto child = libcpp::process::child("child", libcpp::process::std_out > p);
    std::string ret;
    p >> ret;
    std::cout << "child(\"child\", libcpp::process::std_out > ret); ret = "
              << ret << std::endl;
    child.wait();

    std::cout << "\nrun spawn>>" << std::endl;
    libcpp::process::spawn("child");

    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "\nrun daemon>>" << std::endl;
    libcpp::process::daemon("daemon");

    std::vector<libcpp::process::pid_t> vec;
    libcpp::process::list(vec);
    for (auto pid : vec)
        std::cout << "list()> " << pid << std::endl;

    vec.clear();
    libcpp::process::list(vec, [](std::vector<std::string> arg) -> bool {
        if (arg.size() < 2)
            return false;

        if (arg[0] != "daemon")
            return false;

        return true;
    });
    if (vec.empty())
    {
        std::cout << "proc not found" << std::endl;
    }
    else
    {
        for (auto var : vec)
        {
            libcpp::process::kill(var);
            std::cout << "kill(" << var << ")" << std::endl;
        }
    }

    std::cin.get();
    return 0;
}