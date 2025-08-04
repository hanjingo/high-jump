#ifndef SQLITE_HPP
#define SQLITE_HPP

#include <sqlite3.h>

namespace libcpp {

typedef int (*sqlite_exec_cb)(void* in, int argc, char** argv, char** col_name);

}

#endif