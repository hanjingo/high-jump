#include <iostream>
#include <thread>
#include <chrono>

#include <libcpp/sync/counter.hpp>

int main(int argc, char* argv[])
{
    libcpp::counter<int> ct1{0, 0};
    libcpp::counter<int> ct2{1, 0, 100, 1};

    std::cout << "ct1++ = " << ct1++ << std::endl;
    std::cout << "++ct1 = " << ++ct1 << std::endl;
    std::cout << "ct1 + ct2 = " << ct1 + ct2 << std::endl;
    std::cout << "ct1 + 2 = " << ct1 + 2 << std::endl;
    std::cout << "(ct1 += ct2) = " << (ct1 += ct2) << std::endl;
    std::cout << "(ct1 += 1) = " << (ct1 += 1) << std::endl;
    std::cout << std::endl;

    std::cout << "ct1-- = " << ct1-- << std::endl;
    std::cout << "--ct1 = " << --ct1 << std::endl;
    std::cout << "(ct1 -= ct2) = " << (ct1 -= ct2) << std::endl;
    std::cout << "(ct1 -= 1) = " << (ct1 -= 1) << std::endl;
    std::cout << std::endl;

    std::cout << "ct1 * ct2 = " << ct1 * ct2 << std::endl;
    std::cout << "ct2 * 2 = " << ct2 * 2 << std::endl;
    std::cout << "(ct1 *= ct2) = " << (ct1 *= ct2) << std::endl;
    std::cout << "(ct2 *= 2) = " << (ct2 * 2) << std::endl;
    std::cout << std::endl;

    std::cout << "ct1 = " << ct1 << ", ct2 = " << ct2 << " -> "<< "ct1 / ct2 = " << (ct1/ct2) << std::endl;
    std::cout << "ct2 = " << ct2 << " -> " << "ct2 / 2 = " << ct2 / 2 << std::endl;
    std::cout << "ct2 = " << ct2 << " -> " << "2 / ct2 = " << 2 / ct2 << std::endl;
    std::cout << "ct1 = " << ct1 << ", ct2 = " << ct2 << " -> " << "(ct1 /= ct2) = " << (ct1 /= ct2) << std::endl;
    std::cout << "ct2 = " << ct2 << " -> " << "(ct2 /= 2) = " << (ct2 / 2) << std::endl;
    std::cout << std::endl;

    std::cout << "ct1 = " << ct1 << " -> " << "(ct1 = 1) = " << (ct1 = 1) << std::endl;
    std::cout << "ct1 = " << ct1 << ", ct2 = " << ct2 << " -> " << "(ct2 = ct1) = " << (ct2 = ct1) << std::endl;
    std::cout << std::endl;

    std::cout << "ct1 = " << ct1 << " -> " << "(ct1 == 1) = " << (ct1 == 1) << std::endl;
    std::cout << "ct1 = " << ct1 << " -> " << "(ct1 != 1) = " << (ct1 != 1) << std::endl;
    std::cout << "ct1 = " << ct1 << " -> " << "(ct1 < 1) = " << (ct1 < 1) << std::endl;
    std::cout << "ct1 = " << ct1 << " -> " << "(ct1 <= 1) = " << (ct1 <= 1) << std::endl;
    std::cout << "ct1 = " << ct1 << " -> " << "(ct1 > 1) = " << (ct1 > 1) << std::endl;
    std::cout << "ct1 = " << ct1 << " -> " << "(ct1 >= 1) = " << (ct1 >= 1) << std::endl;
    std::cout << "ct1 = " << ct1 << ", ct2 = " << ct2 << " -> " << "(ct2 == ct1) = " << (ct2 == ct1) << std::endl;
    std::cout << "ct1 = " << ct1 << ", ct2 = " << ct2 << " -> " << "(ct2 != ct1) = " << (ct2 != ct1) << std::endl;
    std::cout << "ct1 = " << ct1 << ", ct2 = " << ct2 << " -> " << "(ct2 < ct1) = " << (ct2 < ct1) << std::endl;
    std::cout << "ct1 = " << ct1 << ", ct2 = " << ct2 << " -> " << "(ct2 <= ct1) = " << (ct2 <= ct1) << std::endl;
    std::cout << "ct1 = " << ct1 << ", ct2 = " << ct2 << " -> " << "(ct2 > ct1) = " << (ct2 > ct1) << std::endl;
    std::cout << "ct1 = " << ct1 << ", ct2 = " << ct2 << " -> " << "(ct2 >= ct1) = " << (ct2 >= ct1) << std::endl;
    std::cout << std::endl;

    std::cout << "ct1 = " << ct1 << " -> " << "ct1.inc() = " << ct1.inc() << std::endl;
    std::cout << "ct1 = " << ct1 << " -> " << "ct1.dec() = " << ct1.dec() << std::endl;
    std::cout << "ct1 = " << ct1 << " -> " << "ct1.step() = " << ct1.step() << std::endl;
    std::cout << "ct1 = " << ct1 << " -> " << "ct1.value() = " << ct1.value() << std::endl;
    std::cout << "ct1 = " << ct1 << " -> " << "ct1.max() = " << ct1.max() << std::endl;
    std::cout << "ct1 = " << ct1 << " -> " << "ct1.min() = " << ct1.min() << std::endl;
    std::cout << "ct1 = " << ct1 << " -> " << "ct1.reset() = " << ct1.reset() << std::endl;
    std::cout << std::endl;
    return 0;
}