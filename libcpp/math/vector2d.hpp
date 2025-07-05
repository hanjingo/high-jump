#ifndef VECTOR2D_HPP
#define VECTOR2D_HPP

namespace libcpp {

template <typename T, int row, int col>
using vector2d = T[row][col];

}

#endif