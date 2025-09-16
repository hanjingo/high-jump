#ifndef COMM_H
#define COMM_H

#include <system_error>
#include <libcpp/log/log.hpp>
#include <libcpp/encoding/hex.hpp>
#include <libcpp/crypto/aes.hpp>
#include <libcpp/crypto/base64.hpp>
#include <libcpp/crypto/des.hpp>
#include <libcpp/crypto/rsa.hpp>

using err = std::error_code;

libcpp::aes::mode str_to_aes_mode(const std::string& mode);
libcpp::aes::padding str_to_aes_padding(const std::string& padding);

libcpp::des::mode str_to_des_mode(const std::string& mode);
libcpp::des::padding str_to_des_padding(const std::string& padding);

libcpp::rsa::padding str_to_rsa_padding(const std::string& padding);
libcpp::rsa::key_format str_to_rsa_key_format(const std::string& fmt);
libcpp::rsa::mode str_to_rsa_mode(const std::string& mode);

#endif