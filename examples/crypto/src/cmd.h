#ifndef CMD_H
#define CMD_H

#include <string>

void encrypt(
    std::string& out,
    std::string& in,
    std::string& algo,
    std::string& mode,
    std::string& key,
    std::string& padding,
    std::string& iv,
    std::string& fmt,
    std::string& ctx
);

void decrypt(
    std::string& out,
    std::string& in,
    std::string& algo,
    std::string& mode,
    std::string& key,
    std::string& padding,
    std::string& iv,
    std::string& fmt,
    std::string& ctx
);

void keygen(

);

void help();

#endif