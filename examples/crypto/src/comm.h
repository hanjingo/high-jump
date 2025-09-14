#ifndef COMM_H
#define COMM_H

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>

#include <libcpp/log/log.hpp>
#include <libcpp/encoding/i18n.hpp>
#include <libcpp/crypto/crypto.hpp>

#include "err.h"

static const std::vector<std::string> subcmds{"encrypt", "decrypt", "keygen", "guess", "dict", "help"};
static const std::vector<std::string> algos{
    "auto", "aes", "base64", "des", "md5", "rsa", "sha256"};
static const std::vector<std::string> decrypt_algos{
    "auto", "aes", "base64", "des", "rsa"};

static const std::vector<std::string> aes_modes{
    "auto", "ecb", "cbc", "cfb1", "cfb8", "cfb128", "cfb", "ofb", "ctr", "gcm", "ccm", "xts", 
    "wrap", "wrap_pad", "cbc_hmac_sha1", "cbc_hmac_sha256", "ocb"};
static const std::vector<std::string> aes_paddings{
    "auto", "pkcs5", "pkcs7", "zero", "iso10126", "ansix923", "iso_iec_7816_4", "none"};

static const std::vector<std::string> des_modes{
    "auto", "ecb", "cbc", "cfb", "ofb", "ctr"};
static const std::vector<std::string> des_paddings{
    "auto", "pkcs5", "pkcs7", "zero", "iso10126", "ansix923", "iso_iec_7816_4", "none"};

static const std::vector<std::string> rsa_paddings{
    "auto", "pkcs1", "no_padding", "pkcs1_oaep", "x931", "pkcs1_pss"};

static const std::vector<std::string> fmts{
    "auto", "hex", "base64"};

static const std::vector<std::string> keygen_algos{
    "auto", "rsa"};
static const std::vector<std::string> keygen_rsa_fmts{
    "auto", "x509", "pkcs1"};
static const std::vector<std::string> keygen_rsa_modes{
    "auto", "none", "aes_128_ecb", "aes_192_ecb", "aes_256_ecb",
    "aes_128_cbc", "aes_192_cbc", "aes_256_cbc",
    "aes_128_cfb", "aes_192_cfb", "aes_256_cfb",
    "aes_128_ofb", "aes_192_ofb", "aes_256_ofb"};

static inline std::string tr(const std::string& key, const std::string& default_text = "") 
{
    return libcpp::tr(key, default_text);
}

template<typename... Args>
static inline std::string tr(const std::string& key, Args... args) 
{
    return libcpp::tr(key, args...);
}

void print_console(const std::string& msg);
void print_console(const std::string& msg, const std::string& fmt);
std::string print_str_vector(std::vector<std::string> vec);
libcpp::aes::mode str_to_aes_mode(const std::string& mode);
libcpp::aes::padding str_to_aes_padding(const std::string& padding);

libcpp::des::mode str_to_des_mode(const std::string& mode);
libcpp::des::padding str_to_des_padding(const std::string& padding);

libcpp::rsa::padding str_to_rsa_padding(const std::string& padding);
libcpp::rsa::key_format str_to_rsa_key_format(const std::string& fmt);
libcpp::rsa::mode str_to_rsa_mode(const std::string& mode);

void format(
    std::string& out,
    const std::string& in, 
    const std::string& fmt);
void unformat(
    std::string& out, 
    const std::string& in, 
    const std::string& fmt);

err_t rsa_keygen(
    std::string& pubkey,
    std::string& prikey,
    std::string& name,
    const std::string& fmt,
    const std::string& mode,
    const int bits);

// --------------------- encrypt ----------------------------
bool is_encrypt_output_valid(const std::string& out);
bool is_encrypt_fmt_valid(const std::string& fmt);
bool is_encrypt_input_valid(
    const std::string& in, 
    const std::string& ctx,
    const std::string& algo,
    const std::string& mode,
    const std::string& key,
    const std::string& padding);
bool is_encrypt_algo_valid(const std::string& algo);
bool is_encrypt_mode_valid(
    const std::string& algo, 
    const std::string& mode);
bool is_encrypt_key_valid(
    const std::string& key, 
    const std::string& algo, 
    const std::string& mode);
bool is_encrypt_padding_valid(
    const std::string& padding, 
    const std::string& algo);
bool is_encrypt_iv_valid(
    const std::string& iv, 
    const std::string& algo, 
    const std::string& mode);
err_t encrypt_aes(
    std::string& out,
    const std::string& in,
    const std::string& algo,
    const std::string& mode,
    const std::string& key,
    const std::string& padding,
    const std::string& iv,
    const std::string& ctx);
err_t encrypt_base64(
    std::string& out,
    const std::string& in, 
    const std::string& ctx);
err_t encrypt_des(
    std::string& out,
    const std::string& in,
    const std::string& algo,
    const std::string& mode,
    const std::string& key,
    const std::string& padding,
    const std::string& iv,
    const std::string& ctx);
err_t encrypt_md5(
    std::string& out,
    const std::string& in,
    const std::string& ctx);
err_t encrypt_sha256(
    std::string& out,
    const std::string& in,
    const std::string& ctx);
err_t encrypt_rsa(
    std::string& out,
    const std::string& in,
    const std::string& key,
    const std::string& padding,
    const std::string& ctx);

// --------------------- encrypt ----------------------------
bool is_decrypt_output_valid(const std::string& out);
bool is_decrypt_input_valid(
    const std::string& in, 
    const std::string& ctx,
    const std::string& algo,
    const std::string& key,
    const std::string& padding);
bool is_decrypt_algo_valid(const std::string& algo);
bool is_decrypt_mode_valid(
    const std::string& algo, 
    const std::string& mode);
bool is_decrypt_key_valid(
    const std::string& key, 
    const std::string& algo, 
    const std::string& mode);
bool is_decrypt_padding_valid(
    const std::string& padding, 
    const std::string& algo);
bool is_decrypt_iv_valid(
    const std::string& iv, 
    const std::string& algo, 
    const std::string& mode);
bool is_decrypt_fmt_valid(const std::string& fmt);
err_t decrypt_aes(
    std::string& out,
    const std::string& in,
    const std::string& algo,
    const std::string& mode,
    const std::string& key,
    const std::string& padding,
    const std::string& iv,
    const std::string& ctx);
err_t decrypt_base64(
    std::string& out,
    const std::string& in, 
    const std::string& ctx);
err_t decrypt_des(
    std::string& out,
    const std::string& in,
    const std::string& algo,
    const std::string& mode,
    const std::string& key,
    const std::string& padding,
    const std::string& iv,
    const std::string& ctx);
err_t decrypt_rsa(
    std::string& out,
    const std::string& in,
    const std::string& key,
    const std::string& padding,
    const std::string& password,
    const std::string& ctx);

#endif