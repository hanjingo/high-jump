#ifndef COMM_H
#define COMM_H

#include <string>
#include <vector>

class db
{
public:
    virtual const std::string id() = 0;
    virtual int exec(const char* sql) = 0;
    virtual int query(
        std::vector<std::vector<std::string>>& outs, 
        const char* sql) = 0;
};

#endif