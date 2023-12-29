#include <iostream>

#include <libcpp/sync/once.hpp>

void hello()
{
    std::cout << "hello once before" << std::endl;
    ONCE(
        std::cout << "hello" << std::endl;
    );
    std::cout << "hello once after" << std::endl;
}

class C
{
public:
    void hello()
    {
        std::cout << "C.hello once before" << std::endl;
        ONCE(
            std::cout << "C.hello" << std::endl;
        );
        std::cout << "C.hello once after" << std::endl;
    }
};

namespace tmp
{
void hello()
{
    std::cout << "tmp.hello once before" << std::endl;
    ONCE(
        std::cout << "tmp.hello" << std::endl;
    );
    std::cout << "tmp.hello once after" << std::endl;
}

}

int main(int argc, char* argv[])
{
    hello();
    std::cout << std::endl;

    hello();
    std::cout << std::endl;

    C c;
    c.hello();
    std::cout << std::endl;

    c.hello();
    std::cout << std::endl;

    tmp::hello();
    std::cout << std::endl;

    tmp::hello();
    std::cout << std::endl;

    auto fn = []() {
        ONCE(
            std::cout << "main.hello" << std::endl;
        )
    };

    fn();
    fn();

    std::cin.get();
    return 0;
}