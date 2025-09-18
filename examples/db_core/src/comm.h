#ifndef COMM_H
#define COMM_H

#include <string>

class db
{
public:
    virtual const std::string type() = 0;
    virtual int exec(const char* sql) = 0;
};

#endif