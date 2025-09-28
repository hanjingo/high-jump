#ifndef COMM_H
#define COMM_H

#include <hj/util/license.hpp>

#include "lic_core.h"

int hj_err_to_int(hj::license::err_t e);

hj::license::sign_algo str_to_sign_algo(const std::string &algo);

#endif