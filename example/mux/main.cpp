#include <iostream>
#include <string>

#include <libcpp/misc/multiplexer.hpp>

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
    std::cout << "on error code FATAL with name = " << name << ", return " << f << std::endl;
    return f;
}

void OnWarn()
{
    std::cout << "on error code WARN" << std::endl;
}

MUX(1, {
    std::cout << "on 1" << std::endl;
})

MUX(OnHello, {
    std::cout << "on hello" << std::endl;
})

MUX(OnWorld, {
    num++;
    std::cout << "on world with num = " << num << std::endl;
}, int& num)

int main(int argc, char* argv[])
{
    // ------------------by macro-------------------------------
    ON(1)
    ON(OnHello)
    int iret = 0;
    ON(OnWorld, iret);

    auto ptr = &OnOk;

    // ------------------by object-------------------------------
    std::cout << std::endl;
    libcpp::multiplexer<std::string> hm;
    hm.reg("OK", OnOk);
    hm.on("OK");

    hm.reg("FAIL", &OnFail);
    int iret2 = hm.on<int>("FAIL");
    std::cout << "iret2 = " << iret2 << std::endl;

    hm.reg("FATAL", &OnFatal);
    float fret2 = hm.on<float>("FATAL", std::string("hello")); 
    std::cout << "fret2 = " << fret2 << std::endl;

    hm.reg<void(*)()>("CRITAL", []() {
        std::cout << "on error code CRITAL" << std::endl;
    });
    hm.on("CRITAL");


    // ------------------by singleton-------------------------------
    std::cout << std::endl;
    libcpp::multiplexer<std::string>::instance()->reg("OK", OnOk);
    libcpp::multiplexer<std::string>::instance()->on("OK");

    libcpp::multiplexer<std::string>::instance()->reg("FAIL", &OnFail);
    int iret1 = libcpp::multiplexer<std::string>::instance()->on<int>("FAIL");
    std::cout << "iret1 = " << iret1 << std::endl;

    libcpp::multiplexer<std::string>::instance()->reg("FATAL", &OnFatal);
    float fret1 = libcpp::multiplexer<std::string>::instance()->on<float>("FATAL", std::string("hello"));
    std::cout << "fret1 = " << fret1 << std::endl;

    libcpp::multiplexer<std::string>::instance()->reg<void(*)()>("CRITAL", []() {
        std::cout << "on error code CRITAL" << std::endl;
    });
    libcpp::multiplexer<std::string>::instance()->on("CRITAL");


    std::cin.get();
    return 0;
}