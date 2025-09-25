#ifndef COMM_H
#define COMM_H

#include <vector>
#include <string>

#include <hj/encoding/i18n.hpp>

#include "err.h"

static const std::vector<std::string> all_subcmds{"issue", "verify", "help"};

enum class output_type
{
    console,
    file
};

static inline std::string tr(const std::string& key, const std::string& default_text = "") 
{
    return hj::tr(key, default_text);
}

template<typename... Args>
static inline std::string tr(const std::string& key, Args... args) 
{
    return hj::tr(key, args...);
}

static inline err_t error(const int e) { return hj::make_err(e, "lic"); }

output_type select_output_type(const std::string& out);

std::string fmt_strs(const std::vector<std::string>& vec);

void print(const std::string& msg, const output_type& otype);
void print(const std::vector<std::string>& msgs, const output_type& otype);
void print(const std::vector<std::vector<std::string>>& msgs, const output_type& otype);

#endif