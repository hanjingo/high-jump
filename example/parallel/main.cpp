#include <iostream>

#include <libcpp/sync/parallel.hpp>

class Hello
{
public:
    void operator()(const libcpp::blocked_range<std::size_t>& range)const
    {
        for (size_t i = range.begin(); i != range.end(); ++i) {
            std::cout << i;
        }
        std::cout << std::endl;
    }
};

int main(int argc, char* argv[])
{
    std::cout << "for range example>>" << std::endl;
    libcpp::parallel::for_range(libcpp::blocked_range < std::size_t > (0, 5), Hello());

    // std::cout << "for [start, end] example>>" << std::endl;
    // std::vector<int> vec{1, 2, 3, 4, 5};
    // libcpp::parallel::for(vec.begin(), vec.end(), Hello());

    std::cin.get();
    return 0;
}