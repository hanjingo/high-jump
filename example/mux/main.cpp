#include <iostream>
#include <string>

#include <libcpp/util/multiplexer.hpp>

void OnOk()
{
    std::cout << "on error code OK" << std::endl;
}

int OnFail()
{
    int n = 1;
    std::cout << "on error code FAIL, return " << n << std::endl;
    return n;
}

float OnFatal(std::string name)
{
    float f = 2.1;
    std::cout << "on error code FATAL with name = " << name << ", return " << f
              << std::endl;
    return f;
}

MUX(100, { std::cout << "on 1" << std::endl; })

MUX(std::string("hello"), { std::cout << "on hello" << std::endl; })

MUX(
    std::string("world"),
    {
        num++;
        std::cout << "on world with num = " << num << std::endl;
    },
    int& num)

int main(int argc, char* argv[])
{
    // ------------------by macro-------------------------------
    ON(100)
    ON(std::string("hello"))
    int iret = 0;
    ON(std::string("world"), iret);
    std::cout << "iret = " << iret << std::endl;

    // ------------------by object-------------------------------
    std::cout << std::endl;
    libcpp::multiplexer<std::string> hm;
    hm.reg(std::string("OK"), OnOk);
    hm.on(std::string("OK"));

    hm.reg(std::string("FAIL"), &OnFail);
    int iret2 = hm.on<int>(std::string("FAIL"));
    std::cout << "iret2 = " << iret2 << std::endl;

    hm.reg(std::string("FATAL"), &OnFatal);
    float fret2 = hm.on<float>(std::string("FATAL"), std::string("hello"));
    std::cout << "fret2 = " << fret2 << std::endl;

    hm.reg<void (*)()>(std::string("CRITAL"), []() {
        std::cout << "on error code CRITAL" << std::endl;
    });
    hm.on(std::string("CRITAL"));


    // ------------------by singleton-------------------------------
    std::cout << std::endl;
    libcpp::multiplexer<int>::instance()->reg(1, OnOk);
    libcpp::multiplexer<int>::instance()->on(1);

    libcpp::multiplexer<int>::instance()->reg(2, &OnFail);
    int iret1 = libcpp::multiplexer<int>::instance()->on<int>(2);
    std::cout << "iret1 = " << iret1 << std::endl;

    libcpp::multiplexer<int>::instance()->reg(3, &OnFatal);
    float fret1 =
        libcpp::multiplexer<int>::instance()->on<float>(3,
                                                        std::string("hello"));
    std::cout << "fret1 = " << fret1 << std::endl;

    libcpp::multiplexer<int>::instance()->reg<void (*)()>(4, []() {
        std::cout << "on error code CRITAL" << std::endl;
    });
    libcpp::multiplexer<int>::instance()->on(4);


    std::cin.get();
    return 0;
}