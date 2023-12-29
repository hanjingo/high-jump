#ifndef FSIZE_HPP
#define FSIZE_HPP

namespace libcpp
{

using fsize = unsigned long long;

#define BYTE(n) fsize(n)
#define KB(n) (fsize(n) * 0x400)
#define MB(n) (fsize(n) * 0x100000)
#define GB(n) (fsize(n) * 0x40000000)
#define TB(n) (fsize(n) * 0x10000000000)

}

#endif