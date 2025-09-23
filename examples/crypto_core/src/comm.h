#ifndef COMM_H
#define COMM_H

#include <system_error>
#include <hj/log/log.hpp>
#include <hj/encoding/hex.hpp>
#include <hj/crypto/aes.hpp>
#include <hj/crypto/base64.hpp>
#include <hj/crypto/des.hpp>
#include <hj/crypto/rsa.hpp>

using err = std::error_code;

hj::aes::mode str_to_aes_mode(const std::string& mode);
hj::aes::padding str_to_aes_padding(const std::string& padding);

hj::des::mode str_to_des_mode(const std::string& mode);
hj::des::padding str_to_des_padding(const std::string& padding);

hj::rsa::padding str_to_rsa_padding(const std::string& padding);
hj::rsa::key_format str_to_rsa_key_format(const std::string& fmt);
hj::rsa::mode str_to_rsa_mode(const std::string& mode);

#endif