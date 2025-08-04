#ifndef FILE_HPP
#define FILE_HPP

#ifndef FSIZE
#define FSIZE unsigned long long
#endif

#ifndef BYTE
#define BYTE(n) ((FSIZE) (n))
#endif

#ifndef KB
#define KB(n) ((FSIZE) (n) * 0x400)
#endif

#ifndef MB
#define MB(n) ((FSIZE) (n) * 0x100000)
#endif

#ifndef GB
#define GB(n) ((FSIZE) (n) * 0x40000000)
#endif

#ifndef TB
#define TB(n) ((FSIZE) (n) * 0x10000000000)
#endif

namespace libcpp
{

}

#endif