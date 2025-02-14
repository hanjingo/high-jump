#include <iostream>

#include <Eigen/Core>
#include <Eigen/Dense>

#include <libcpp/math/matrix_iterator.hpp>
#include <libcpp/math/matrix_row_iterator.hpp>

void itr_example()
{
    std::cout << "matrix iterator example >>>>>>>>>>>>" << std::endl;
    Eigen::Matrix<int, 3, 3> mat1;
    mat1 <<
         1, 2, 3,
         4, 5, 6,
         7, 8, 9;
    std::cout << "mat1 = \n" << mat1 << std::endl;
    std::cout << std::endl;

    libcpp::matrix_iterator<decltype(mat1), int> itr(&mat1, 0, 0, 3, 3);
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
    Eigen::Matrix<int, 3, 3> mat2;
    mat2 <<
         1, 2, 3,
         4, 5, 6,
         7, 8, 9;
    std::cout << "mat2 = \n" << mat2 << std::endl;
    std::cout << std::endl;

    libcpp::matrix_row_iterator<decltype(mat2), int> vitr(&mat2, 0, 0, 3, 3);
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