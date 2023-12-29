#include <iostream>

#include <libcpp/math/vector2d.hpp>
#include <libcpp/math/vector2d_iterator.hpp>
#include <libcpp/math/vector2d_row_iterator.hpp>

void itr_example()
{
    std::cout << "vector2d iterator example >>>>>>>>>>>>" << std::endl;
    //int vec1[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    libcpp::vector2d<int, 3, 3> vec1{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

    libcpp::vector2d_iterator<decltype(vec1), int> itr(&vec1, 0, 0, 3, 3);
    std::cout << "*itr = " << *itr << std::endl;
    std::cout << std::endl;

    std::cout << "*(++itr) = " << *(++itr) << std::endl;
    std::cout << "*(itr++) = " << *(itr++) << std::endl;
    std::cout << "*(itr+1) = " << *(itr + 1) << std::endl;
    std::cout << "*(itr+=3) = " << *(itr += 3) << std::endl;
    std::cout << std::endl;

    std::cout << "*(--itr) = " << *(--itr) << std::endl;
    std::cout << "*(itr--) = " << *(itr--) << std::endl;
    std::cout << "*(itr-1) = " << *(itr - 1) << std::endl;
    std::cout << "*(itr -= 3) = " << *(itr -= 3) << std::endl;
    std::cout << std::endl;

    std::cout << "(itr == itr) = " << (itr == itr) << std::endl;
    std::cout << "(itr == (itr+1)) = " << (itr == (itr + 1)) << std::endl;
    std::cout << "((itr + 1) == (itr + 1)) = " << ((itr + 1) == (itr + 1)) << std::endl;
    std::cout << std::endl;

    std::cout << "(itr != itr) = " << (itr != itr) << std::endl;
    std::cout << "(itr != (itr+1)) = " << (itr != (itr + 1)) << std::endl;
    std::cout << "((itr + 1) != (itr + 1)) = " << ((itr + 1) != (itr + 1)) << std::endl;
    std::cout << std::endl;

    std::cout << "(itr < itr) = " << (itr < itr) << std::endl;
    std::cout << "(itr < (itr + 1)) = " << (itr < (itr + 1)) << std::endl;
    std::cout << "((itr + 1) < itr) = " << ((itr + 1) < itr) << std::endl;
    std::cout << std::endl;

    std::cout << "(itr <= itr) = " << (itr <= itr) << std::endl;
    std::cout << "(itr <= (itr + 1)) = " << (itr <= (itr + 1)) << std::endl;
    std::cout << "((itr + 1) <= itr) = " << ((itr + 1) <= itr) << std::endl;
    std::cout << std::endl;

    std::cout << "(itr > itr) = " << (itr > itr) << std::endl;
    std::cout << "(itr > (itr + 1)) = " << (itr > (itr + 1)) << std::endl;
    std::cout << "((itr + 1) > itr) = " << ((itr + 1) > itr) << std::endl;
    std::cout << std::endl;

    std::cout << "(itr >= itr) = " << (itr >= itr) << std::endl;
    std::cout << "(itr >= (itr + 1)) = " << (itr >= (itr + 1)) << std::endl;
    std::cout << "((itr + 1) >= itr) = " << ((itr + 1) >= itr) << std::endl;
    std::cout << std::endl;

    std::cout << "*itr = " << *itr << std::endl;
    std::cout << "*(itr + 4) = " << *(itr + 4) << std::endl;
    std::cout << "itr->distance(itr + 1) = " << itr.distance(itr + 1) << std::endl;
    std::cout << "itr->distance(itr + 4) = " << itr.distance(itr + 4) << std::endl;
    std::cout << "itr->distance(itr + 6) = " << itr.distance(itr + 6) << std::endl;
    std::cout << "(itr + 1).distance(itr) = " << (itr + 1).distance(itr) << std::endl;
    std::cout << "(itr + 4).distance(itr) = " << (itr + 4).distance(itr) << std::endl;
    std::cout << "(itr + 6).distance(itr) = " << (itr + 6).distance(itr) << std::endl;
    std::cout << std::endl;
}

void row_itr_example()
{
    std::cout << "matrix row iterator example >>>>>>>>>>>>" << std::endl;
    //int vec2[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    libcpp::vector2d<int, 3, 3> vec2{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

    libcpp::vector2d_row_iterator<decltype(vec2), int> vitr(&vec2, 0, 0, 3, 3);
    std::cout << "*vitr = " << *vitr << std::endl;
    std::cout << std::endl;

    std::cout << "*(++vitr) = " << *(++vitr) << std::endl;
    std::cout << "*(vitr++) = " << *(vitr++) << std::endl;
    std::cout << "*(vitr+1) = " << *(vitr + 1) << std::endl;
    std::cout << "*(vitr+=3) = " << *(vitr += 3) << std::endl;
    std::cout << std::endl;

    std::cout << "*(--vitr) = " << *(--vitr) << std::endl;
    std::cout << "*(vitr--) = " << *(vitr--) << std::endl;
    std::cout << "*(vitr-1) = " << *(vitr - 1) << std::endl;
    std::cout << "*(vitr -= 3) = " << *(vitr -= 3) << std::endl;
    std::cout << std::endl;

    std::cout << "(vitr == vitr) = " << (vitr == vitr) << std::endl;
    std::cout << "(vitr == (vitr+1)) = " << (vitr == (vitr + 1)) << std::endl;
    std::cout << "((vitr + 1) == (vitr + 1)) = " << ((vitr + 1) == (vitr + 1)) << std::endl;
    std::cout << std::endl;

    std::cout << "(vitr != vitr) = " << (vitr != vitr) << std::endl;
    std::cout << "(vitr != (vitr+1)) = " << (vitr != (vitr + 1)) << std::endl;
    std::cout << "((vitr + 1) != (vitr + 1)) = " << ((vitr + 1) != (vitr + 1)) << std::endl;
    std::cout << std::endl;

    std::cout << "(vitr < vitr) = " << (vitr < vitr) << std::endl;
    std::cout << "(vitr < (vitr + 1)) = " << (vitr < (vitr + 1)) << std::endl;
    std::cout << "((vitr + 1) < vitr) = " << ((vitr + 1) < vitr) << std::endl;
    std::cout << std::endl;

    std::cout << "(vitr <= vitr) = " << (vitr <= vitr) << std::endl;
    std::cout << "(vitr <= (vitr + 1)) = " << (vitr <= (vitr + 1)) << std::endl;
    std::cout << "((vitr + 1) <= vitr) = " << ((vitr + 1) <= vitr) << std::endl;
    std::cout << std::endl;

    std::cout << "(vitr > vitr) = " << (vitr > vitr) << std::endl;
    std::cout << "(vitr > (vitr + 1)) = " << (vitr > (vitr + 1)) << std::endl;
    std::cout << "((vitr + 1) > vitr) = " << ((vitr + 1) > vitr) << std::endl;
    std::cout << std::endl;

    std::cout << "(vitr >= vitr) = " << (vitr >= vitr) << std::endl;
    std::cout << "(vitr >= (vitr + 1)) = " << (vitr >= (vitr + 1)) << std::endl;
    std::cout << "((vitr + 1) >= vitr) = " << ((vitr + 1) >= vitr) << std::endl;
    std::cout << std::endl;

    std::cout << "*vitr = " << *vitr << std::endl;
    std::cout << "vitr->distance(vitr + 1) = " << vitr.distance(vitr + 1) << std::endl;
    std::cout << "vitr->distance(vitr + 4) = " << vitr.distance(vitr + 4) << std::endl;
    std::cout << "vitr->distance(vitr + 6) = " << vitr.distance(vitr + 6) << std::endl;
    std::cout << "(vitr + 1).distance(vitr) = " << (vitr + 1).distance(vitr) << std::endl;
    std::cout << "(vitr + 4).distance(vitr) = " << (vitr + 4).distance(vitr) << std::endl;
    std::cout << "(vitr + 6).distance(vitr) = " << (vitr + 6).distance(vitr) << std::endl;
    std::cout << std::endl;
}

int main(int argc, char* argv[])
{
    itr_example();

    row_itr_example();

    std::cin.get();
    return 0;
}