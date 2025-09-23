#ifndef COMM_H
#define COMM_H

#include <iostream>
#include <hj/encoding/i18n.hpp>

#include "err.h"

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

static inline err_t error(const int e) { return hj::make_err(e, "crypto"); }

output_type select_output_type(const std::string& out);

std::string select_encrypt_output_fmt(
    std::string& fmt,
    const std::string& out,
    const std::string& algo);

std::string fmt_strs(const std::vector<std::string>& vec);

void print(const std::string& msg, const output_type& otype);
void print(const std::vector<std::string>& msgs, const output_type& otype);
void print(const std::vector<std::vector<std::string>>& msgs, const output_type& otype);

#endif