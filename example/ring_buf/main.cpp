#include <iostream>

#include <libcpp/io/ring_buffer.hpp>

int main(int argc, char* argv[])
{
    libcpp::ring_buffer<int> buf{3};
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);
    buf.push_back(4);

    std::cout << "buf.front() = " << buf.front() << std::endl;

    std::cout << "buf.back() = " << buf.back() << std::endl;

    std::cout << "*(buf.begin()) = " << *(buf.begin()) << std::endl;

    std::cout << "range buf: ";
    for (auto itr = buf.begin(); itr != buf.end(); itr++) {
        std::cout << *itr << ", ";
    }
    std::cout << std::endl;

    std::cout << "buf.at(0) = " << buf.at(0) << std::endl;

    std::cout << "buf[0] = " << buf[0] << std::endl;

    std::cout << "buf.full() = " << buf.full() << std::endl;

    std::cout << "buf.size() = " << buf.size() << std::endl;

    std::cout << "buf.capacity() = " << buf.capacity() << std::endl;

    std::cin.get();
    return 0;
}