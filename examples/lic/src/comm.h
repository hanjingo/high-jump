#ifndef COMM_H
#define COMM_H

#include <vector>
#include <string>

#include "err.h"

static const std::vector<std::string> all_subcmds{
    "keygen", "add", "issue", "verify", "help"};

enum class output_type
{
    console,
    file
};

static inline err_t error(const int e, const char *category = "app")
{
    return hj::make_err(e, category);
}

output_type select_output_type(const std::string &out);

std::string fmt_strs(const std::vector<std::string> &vec);

void print(const std::string &msg, const output_type &otype);
void print(const std::vector<std::string> &msgs, const output_type &otype);
void print(const std::vector<std::vector<std::string> > &msgs,
           const output_type                            &otype);

std::vector<std::pair<std::string, std::string> >
parse_claims(const std::string &s);

#endif