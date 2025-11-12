#ifndef COMM_H
#define COMM_H

#include <iostream>
#include <vector>
#include <string>

#include "err.h"

static const std::vector<std::string> all_subcmds{
    "encrypt", "decrypt", "keygen", "guess", "add", "list", "help"};
static const std::vector<std::string> all_keygen_algos{"rsa"};
static const std::vector<std::string> all_encrypt_algos{
    "aes", "des", "rsa", "sha256", "md5", "base64"};
static const std::vector<std::string> all_decrypt_algos{
    "aes", "des", "rsa", "sha256", "md5", "base64"};
static const std::vector<std::string> all_encrypt_fmts{"hex", "base64", "none"};

enum class output_type
{
    console,
    file
};

static inline err_t error(const int e)
{
    return hj::make_err(e, "crypto");
}

output_type select_output_type(const std::string &out);

std::string select_encrypt_output_fmt(std::string       &fmt,
                                      const std::string &out,
                                      const std::string &algo);

std::string fmt_strs(const std::vector<std::string> &vec);

void print(const std::string &msg, const output_type &otype);
void print(const std::vector<std::string> &msgs, const output_type &otype);
void print(const std::vector<std::vector<std::string> > &msgs,
           const output_type                            &otype);

#endif