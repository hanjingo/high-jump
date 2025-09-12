#include "comm.h"

#include <vector>
#include <iostream>

#include <libcpp/io/filepath.hpp>
#include <libcpp/encoding/hex.hpp>

void print_console(const std::string& msg)
{
    std::cout << msg << std::endl;
}

void print_console(const std::string& msg, const std::string& fmt)
{
    std::string buf;
    format(buf, msg, fmt);
    print_console(buf);
}

std::string print_str_vector(std::vector<std::string> vec)
{
    std::string s;
    for (const auto& e : vec)
        s += e + ", ";

    s = s.substr(0, s.size() - 2);  // remove trailing comma and space
    return s;
}

libcpp::aes::mode str_to_aes_mode(const std::string& mode)
{
    if (mode == "ecb")
        return libcpp::aes::mode::ecb;
    else if (mode == "cbc")
        return libcpp::aes::mode::cbc;
    else if (mode == "cfb1")
        return libcpp::aes::mode::cfb1;
    else if (mode == "cfb8")
        return libcpp::aes::mode::cfb8;
    else if (mode == "cfb128")
        return libcpp::aes::mode::cfb128;
    else if (mode == "cfb")
        return libcpp::aes::mode::cfb;
    else if (mode == "ofb")
        return libcpp::aes::mode::ofb;
    else if (mode == "ctr")
        return libcpp::aes::mode::ctr;
    else if (mode == "gcm")
        return libcpp::aes::mode::gcm;
    else if (mode == "ccm")
        return libcpp::aes::mode::ccm;
    else if (mode == "xts")
        return libcpp::aes::mode::xts;
    else if (mode == "wrap")
        return libcpp::aes::mode::wrap;
    else if (mode == "wrap_pad")
        return libcpp::aes::mode::wrap_pad;
    else if (mode == "cbc_hmac_sha1")
        return libcpp::aes::mode::cbc_hmac_sha1;
    else if (mode == "cbc_hmac_sha256")
        return libcpp::aes::mode::cbc_hmac_sha256;
    else if (mode == "ocb")
        return libcpp::aes::mode::ocb;
    else
        return libcpp::aes::mode::ecb; // default
}

libcpp::aes::padding str_to_aes_padding(const std::string& padding)
{
    if (padding == "pkcs5")
        return libcpp::aes::padding::pkcs5;
    else if (padding == "pkcs7")
        return libcpp::aes::padding::pkcs7;
    else if (padding == "zero")
        return libcpp::aes::padding::zero;
    else if (padding == "iso10126")
        return libcpp::aes::padding::iso10126;
    else if (padding == "ansix923")
        return libcpp::aes::padding::ansix923;
    else if (padding == "iso_iec_7816_4")
        return libcpp::aes::padding::iso_iec_7816_4;
    else if (padding == "no_padding")
        return libcpp::aes::padding::no_padding;
    else
        return libcpp::aes::padding::pkcs7; // default
}

libcpp::des::mode str_to_des_mode(const std::string& mode)
{
    if (mode == "ecb")
        return libcpp::des::mode::ecb;
    else if (mode == "cbc")
        return libcpp::des::mode::cbc;
    else if (mode == "cfb")
        return libcpp::des::mode::cfb;
    else if (mode == "ofb")
        return libcpp::des::mode::ofb;
    else if (mode == "ctr")
        return libcpp::des::mode::ctr;
    else
        return libcpp::des::mode::ecb; // default
}

libcpp::des::padding str_to_des_padding(const std::string& padding)
{
    if (padding == "pkcs5")
        return libcpp::des::padding::pkcs5;
    else if (padding == "pkcs7")
        return libcpp::des::padding::pkcs7;
    else if (padding == "zero")
        return libcpp::des::padding::zero;
    else if (padding == "iso10126")
        return libcpp::des::padding::iso10126;
    else if (padding == "ansix923")
        return libcpp::des::padding::ansix923;
    else if (padding == "iso_iec_7816_4")
        return libcpp::des::padding::iso_iec_7816_4;
    else if (padding == "no_padding")
        return libcpp::des::padding::no_padding;
    else
        return libcpp::des::padding::pkcs7; // default
}

libcpp::rsa::padding str_to_rsa_padding(const std::string& padding)
{
    if (padding == "pkcs1")
        return libcpp::rsa::padding::pkcs1;
    else if (padding == "pkcs1_oaep")
        return libcpp::rsa::padding::pkcs1_oaep;
    else if (padding == "no_padding")
        return libcpp::rsa::padding::no_padding;
    else if (padding == "x931")
        return libcpp::rsa::padding::x931;
    else if (padding == "pkcs1_pss")
        return libcpp::rsa::padding::pkcs1_pss;
    else
        return libcpp::rsa::padding::pkcs1; // default
}

libcpp::rsa::key_format str_to_rsa_key_format(const std::string& fmt)
{
    if (fmt == "x509")
        return libcpp::rsa::key_format::x509;
    else if (fmt == "pkcs1")
        return libcpp::rsa::key_format::pkcs1;
    else
        return libcpp::rsa::key_format::x509; // default
}

libcpp::rsa::mode str_to_rsa_mode(const std::string& mode)
{
    if (mode == "none")
        return libcpp::rsa::mode::none;
    else if (mode == "aes_128_ecb")
        return libcpp::rsa::mode::aes_128_ecb;
    else if (mode == "aes_192_ecb")
        return libcpp::rsa::mode::aes_192_ecb;
    else if (mode == "aes_256_ecb")
        return libcpp::rsa::mode::aes_256_ecb;
    else if (mode == "aes_128_cbc")
        return libcpp::rsa::mode::aes_128_cbc;
    else if (mode == "aes_192_cbc")
        return libcpp::rsa::mode::aes_192_cbc;
    else if (mode == "aes_256_cbc")
        return libcpp::rsa::mode::aes_256_cbc;
    else if (mode == "aes_128_cfb")
        return libcpp::rsa::mode::aes_128_cfb;
    else if (mode == "aes_192_cfb")
        return libcpp::rsa::mode::aes_192_cfb;
    else if (mode == "aes_256_cfb")
        return libcpp::rsa::mode::aes_256_cfb;
    else if (mode == "aes_128_ofb")
        return libcpp::rsa::mode::aes_128_ofb;
    else if (mode == "aes_192_ofb")
        return libcpp::rsa::mode::aes_192_ofb;
    else if (mode == "aes_256_ofb")
        return libcpp::rsa::mode::aes_256_ofb;
    else
        return libcpp::rsa::mode::none; // default
}

void format(
    std::string& out,
    const std::string& in, 
    const std::string& fmt)
{
    if (fmt == "hex")
    {
        out = libcpp::hex::encode(in, true); // upper case
    }
    else if (fmt == "base64")
    {
        libcpp::base64::encode(out, in);
    }
    else
    {
        out = in;
    }
}

void unformat(
    std::string& out,
    const std::string& in, 
    const std::string& fmt)
{
    if (fmt == "hex")
    {
        out = libcpp::hex::decode(in); // upper case
    }
    else if (fmt == "base64")
    {
        libcpp::base64::decode(out, in);
    }
}

err_t rsa_keygen(
    std::string& pubkey,
    std::string& prikey,
    std::string& name,
    const std::string& fmt,
    const std::string& mode,
    const int bits)
{
    auto key_fmt = str_to_rsa_key_format(fmt);
    auto key_mode = str_to_rsa_mode(mode);
    bool succ = false;
    if (name == "") // to console
    {
        succ = libcpp::rsa::make_key_pair(pubkey, prikey, bits, key_fmt, key_mode);
    }
    else // to file
    {

    }

    return succ ? err_t() : error(err_keygen_fail);
}

// --------------------- encrypt ----------------------------
bool is_encrypt_output_valid(const std::string& out)
{
    if (out != "" && (libcpp::filepath::is_dir(out) || libcpp::filepath::is_symlink(out)))
        return false;

    return true;
}

bool is_encrypt_input_valid(
    const std::string& in, 
    const std::string& ctx,
    const std::string& algo,
    const std::string& mode,
    const std::string& key,
    const std::string& padding)
{
    if (in == "" && ctx == "")
        return false;

    // encrypt to file
    if (in != "")
    {
        if (!libcpp::filepath::is_exist(in))
            return false;

        if(algo == "base64" || algo == "md5" || algo == "sha256")
        {
            return true;
        }
        else if (algo == "aes")
        {
            auto mod = str_to_aes_mode(mode);
            auto pad = str_to_aes_padding(padding);
            std::ifstream fin(in, std::ios::binary);
            return libcpp::aes::is_plain_valid(mod, key.size(), pad, fin);
        }
        else if (algo == "des")
        {
            auto pad = str_to_des_padding(padding);
            std::ifstream fin(in, std::ios::binary);
            return libcpp::des::is_plain_valid(fin, pad);
        }
        else if (algo == "rsa")
        {
            std::ifstream fin(in, std::ios::binary);
            auto pad = str_to_rsa_padding(padding);
            return libcpp::rsa::is_plain_valid(
                fin, 
                pad, 
                reinterpret_cast<const unsigned char*>(key.c_str()),
                key.size());
        }
    }

    // encrypt to console
    if (ctx != "")
    {
        if(algo == "base64" || algo == "md5" || algo == "sha256")
        {
            return true;
        }
        else if (algo == "aes")
        {
            auto mod = str_to_aes_mode(mode);
            auto pad = str_to_aes_padding(padding);
            return libcpp::aes::is_plain_valid(mod, key.size(), pad, ctx.size());
        }
        else if (algo == "des")
        {
            auto pad = str_to_des_padding(padding);
            return libcpp::des::is_plain_valid(
                reinterpret_cast<const unsigned char*>(ctx.c_str()), 
                ctx.size(), 
                pad);
        }
        else if (algo == "rsa")
        {
            auto pad = str_to_rsa_padding(padding);
            return libcpp::rsa::is_plain_valid(
                ctx.size(), 
                pad, 
                reinterpret_cast<const unsigned char*>(key.c_str()),
                key.size());
        }
    }

    return false;
}

bool is_encrypt_fmt_valid(const std::string& fmt)
{
    return std::find(std::begin(fmts), std::end(fmts), fmt) != std::end(fmts);
}

bool is_encrypt_algo_valid(const std::string& algo)
{
    return std::find(std::begin(algos), std::end(algos), algo) != std::end(algos);
}

bool is_encrypt_mode_valid(const std::string& algo, 
                           const std::string& mode)
{
    if (algo == "aes")
    {
        return std::find(std::begin(aes_modes), std::end(aes_modes), mode) != std::end(aes_modes);
    }
    else if (algo == "des")
    {
        return std::find(std::begin(des_modes), std::end(des_modes), mode) != std::end(des_modes);
    }
    else if (algo == "base64" || algo == "md5" || algo == "sha256" || algo == "rsa")
    {
        return true;
    }

    return false;
}

bool is_encrypt_key_valid(const std::string& key, 
                          const std::string& algo, 
                          const std::string& mode)
{
    if (algo == "aes")
    {
        auto mod = str_to_aes_mode(mode);
        return libcpp::aes::is_key_valid(
            mod, 
            reinterpret_cast<const unsigned char*>(key.c_str()), 
            key.size());
    }
    else if (algo == "des")
    {
        return libcpp::des::is_key_valid(
            reinterpret_cast<const unsigned char*>(key.c_str()), 
            key.size());
    }
    else if (algo == "rsa")
    {
        return libcpp::rsa::is_key_pair_bits_valid(key.size() * 8);
    }
    else if (algo == "base64" || algo == "md5" || algo == "sha256")
    {
        return true;
    }

    return false;
}

bool is_encrypt_padding_valid(const std::string& padding, const std::string& algo)
{
    if (algo == "aes")
    {
        return std::find(std::begin(aes_paddings), std::end(aes_paddings), padding) != std::end(aes_paddings);
    }
    else if (algo == "des")
    {
        return std::find(std::begin(des_paddings), std::end(des_paddings), padding) != std::end(des_paddings);
    }
    else if (algo == "rsa")
    {
        return std::find(std::begin(rsa_paddings), std::end(rsa_paddings), padding) != std::end(rsa_paddings);
    }
    else if (algo == "base64" || algo == "md5" || algo == "sha256")
    {
        return true;
    }

    return false;
}

bool is_encrypt_iv_valid(const std::string& iv, const std::string& algo, const std::string& mode)
{
    const unsigned char* iv_ptr = (iv == "") ? nullptr : reinterpret_cast<const unsigned char*>(iv.c_str());
    if (algo == "aes")
    {
        auto mod = str_to_aes_mode(mode);
        return libcpp::aes::is_iv_valid(mod, iv_ptr, iv.size());
    }
    else if (algo == "des")
    {
        auto mod = str_to_des_mode(mode);
        return libcpp::des::is_iv_valid(mod, iv_ptr, iv.size());
    }
    else if (algo == "base64" || algo == "md5" || algo == "sha256")
    {
        return true;
    }

    return false; 
}

err_t encrypt_aes(
    std::string& out,
    const std::string& in,
    const std::string& algo,
    const std::string& mode,
    const std::string& key,
    const std::string& padding,
    const std::string& iv,
    const std::string& ctx)
{
    err_t err;
    auto mod = str_to_aes_mode(mode);
    auto pad = str_to_aes_padding(padding);
    bool succ = false;
    if (out == "" && in == "") // encrypt: mem -> mem
    {
        succ = libcpp::aes::encrypt(out, ctx, key, mod, pad, iv);
    }
    else if (out != "" && in != "") // encrypt: file -> file
    {
        succ = libcpp::aes::encrypt_file(out, in, key, mod, pad, iv);
    }
    else if (out != "" && in == "") // encrypt: mem -> file
    {
        std::ofstream fout(out, std::ios::binary);
        std::istringstream sin(ctx);
        succ = libcpp::aes::encrypt(fout, sin, reinterpret_cast<const unsigned char*>(key.c_str()), key.size(),
            mod, pad, reinterpret_cast<const unsigned char*>(iv.c_str()), iv.size());
    }
    else // encrypt: file -> mem
    {
        std::ifstream fin(in, std::ios::binary);
        std::ostringstream sout(out);
        succ = libcpp::aes::encrypt(sout, fin, reinterpret_cast<const unsigned char*>(key.c_str()), key.size(),
            mod, pad, reinterpret_cast<const unsigned char*>(iv.c_str()), iv.size());
        out = sout.str();
    }

    return succ ? err_t() : error(err_encrypt_aes_failed);
}

err_t encrypt_base64(
    std::string& out,
    const std::string& in, 
    const std::string& ctx)
{
    bool succ = false;
    if (out == "" && in == "") // encrypt: mem -> mem
    {
        succ = libcpp::base64::encode(out, ctx);
    }
    else if (out != "" && in != "") // encrypt: file -> file
    {
        succ = libcpp::base64::encode_file(out, in);
    }
    else if (out != "" && in == "") // encrypt: mem -> file
    {
        std::ofstream fout(out, std::ios::binary);
        std::istringstream sin(ctx);
        succ = libcpp::base64::encode(fout, sin);
    }
    else // encrypt: file -> mem
    {
        std::ifstream fin(in, std::ios::binary);
        std::ostringstream sout(out);
        succ = libcpp::base64::encode(sout, fin);
        out = sout.str();
    }

    return succ ? err_t() : error(err_encrypt_base64_failed);
}

err_t encrypt_des(
    std::string& out,
    const std::string& in,
    const std::string& algo,
    const std::string& mode,
    const std::string& key,
    const std::string& padding,
    const std::string& iv,
    const std::string& ctx)
{
    auto mod = str_to_des_mode(mode);
    auto pad = str_to_des_padding(padding);
    bool succ = false;
    if (out == "" && in == "") // encrypt: mem -> mem
    {
        succ = libcpp::des::encrypt(out, ctx, key, mod, pad, iv);
    }
    else if (out != "" && in != "") // encrypt: file -> file
    {
        succ = libcpp::des::encrypt_file(out, in, key, mod, pad, iv);
    }
    else if (out != "" && in == "") // encrypt: mem -> file
    {
        std::ofstream fout(out, std::ios::binary);
        std::istringstream sin(ctx);
        succ = libcpp::des::encrypt(fout, sin, reinterpret_cast<const unsigned char*>(key.c_str()), key.size(),
            mod, pad, reinterpret_cast<const unsigned char*>(iv.c_str()), iv.size());
    }
    else // encrypt: file -> mem
    {
        std::ifstream fin(in, std::ios::binary);
        std::ostringstream sout(out);
        succ = libcpp::des::encrypt(sout, fin, reinterpret_cast<const unsigned char*>(key.c_str()), key.size(),
            mod, pad, reinterpret_cast<const unsigned char*>(iv.c_str()), iv.size());
        out = sout.str();
    }

    return succ ? err_t() : error(err_encrypt_des_failed);
}

err_t encrypt_md5(
    std::string& out,
    const std::string& in,
    const std::string& ctx)
{
    bool succ = false;
    if (out == "" && in == "") // encrypt: mem -> mem
    {
        succ = libcpp::md5::encode(out, ctx);
    }
    else if (out != "" && in != "") // encrypt: file -> file
    {
        succ = libcpp::md5::encode_file(out, in);
    }
    else if (out != "" && in == "") // encrypt: mem -> file
    {
        std::ofstream fout(out, std::ios::binary);
        std::istringstream sin(ctx);
        succ = libcpp::md5::encode(fout, sin);
    }
    else // encrypt: file -> mem
    {
        std::ifstream fin(in, std::ios::binary);
        std::ostringstream sout(out);
        succ = libcpp::md5::encode(sout, fin);
        out = sout.str();
    }

    return succ ? err_t() : error(err_encrypt_md5_failed);
}

err_t encrypt_sha256(
    std::string& out,
    const std::string& in,
    const std::string& ctx)
{
    bool succ = false;
    if (out == "" && in == "") // encrypt: mem -> mem
    {
        succ = libcpp::sha::encode(out, ctx, libcpp::sha::algorithm::sha256);
    }
    else if (out != "" && in != "") // encrypt: file -> file
    {
        succ = libcpp::sha::encode_file(out, in, libcpp::sha::algorithm::sha256);
    }
    else if (out != "" && in == "") // encrypt: mem -> file
    {
        std::ofstream fout(out, std::ios::binary);
        std::istringstream sin(ctx);
        succ = libcpp::sha::encode(fout, sin, libcpp::sha::algorithm::sha256);
    }
    else // encrypt: file -> mem
    {
        std::ifstream fin(in, std::ios::binary);
        std::ostringstream sout(out);
        succ = libcpp::sha::encode(sout, fin, libcpp::sha::algorithm::sha256);
        out = sout.str();
    }

    return succ ? err_t() : error(err_encrypt_sha256_failed);
}

err_t encrypt_rsa(
    std::string& out,
    const std::string& in,
    const std::string& algo,
    const std::string& mode,
    const std::string& key,
    const std::string& padding,
    const std::string& iv,
    const std::string& ctx)
{
    // TODO
    return err_t();
}

// --------------------- decrypt ----------------------------
bool is_decrypt_output_valid(const std::string& out)
{
    if (out != "" && (libcpp::filepath::is_dir(out) || libcpp::filepath::is_symlink(out)))
        return false;

    return true;
}

bool is_decrypt_input_valid(
    const std::string& in, 
    const std::string& ctx,
    const std::string& algo,
    const std::string& key,
    const std::string& padding)
{
    if (in == "" && ctx == "")
        return false;

    // decrypt to file
    if (in != "")
    {
        if (!libcpp::filepath::is_exist(in))
            return false;

        if(algo == "base64" || algo == "md5" || algo == "sha256" || algo == "aes" || algo == "des")
        {
            return true;
        }
        else if (algo == "rsa")
        {
            std::ifstream fin(in, std::ios::binary);
            auto pad = str_to_rsa_padding(padding);
            return libcpp::rsa::is_cipher_valid(
                fin, 
                pad, 
                reinterpret_cast<const unsigned char*>(key.c_str()),
                key.size());
        }
    }

    // decrypt to console
    if (ctx != "")
    {
        if(algo == "base64" || algo == "md5" || algo == "sha256" || algo == "aes" || algo == "des")
        {
            return true;
        }
        else if (algo == "rsa")
        {
            auto pad = str_to_rsa_padding(padding);
            return libcpp::rsa::is_cipher_valid(ctx, pad, key);
        }
    }

    return false;
}

bool is_decrypt_algo_valid(const std::string& algo)
{
    return std::find(std::begin(decrypt_algos), std::end(decrypt_algos), algo) != std::end(decrypt_algos);
}

bool is_decrypt_mode_valid(const std::string& algo, 
                           const std::string& mode)
{
    if (algo == "aes")
    {
        return std::find(std::begin(aes_modes), std::end(aes_modes), mode) != std::end(aes_modes);
    }
    else if (algo == "des")
    {
        return std::find(std::begin(des_modes), std::end(des_modes), mode) != std::end(des_modes);
    }
    else if (algo == "base64" || algo == "md5" || algo == "sha256" || algo == "rsa")
    {
        return true;
    }

    return false;
}

bool is_decrypt_key_valid(const std::string& key, 
                          const std::string& algo, 
                          const std::string& mode)
{
    if (algo == "aes")
    {
        auto mod = str_to_aes_mode(mode);
        return libcpp::aes::is_key_valid(
            mod, 
            reinterpret_cast<const unsigned char*>(key.c_str()), 
            key.size());
    }
    else if (algo == "des")
    {
        return libcpp::des::is_key_valid(
            reinterpret_cast<const unsigned char*>(key.c_str()), 
            key.size());
    }
    else if (algo == "rsa")
    {
        return libcpp::rsa::is_key_pair_bits_valid(key.size() * 8);
    }
    else if (algo == "base64" || algo == "md5" || algo == "sha256")
    {
        return true;
    }

    return false;
}

bool is_decrypt_padding_valid(const std::string& padding, const std::string& algo)
{
    if (algo == "aes")
    {
        return std::find(std::begin(aes_paddings), std::end(aes_paddings), padding) != std::end(aes_paddings);
    }
    else if (algo == "des")
    {
        return std::find(std::begin(des_paddings), std::end(des_paddings), padding) != std::end(des_paddings);
    }
    else if (algo == "rsa")
    {
        return std::find(std::begin(rsa_paddings), std::end(rsa_paddings), padding) != std::end(rsa_paddings);
    }
    else if (algo == "base64" || algo == "md5" || algo == "sha256")
    {
        return true;
    }

    return false;
}

bool is_decrypt_iv_valid(const std::string& iv, const std::string& algo, const std::string& mode)
{
    const unsigned char* iv_ptr = (iv == "") ? nullptr : reinterpret_cast<const unsigned char*>(iv.c_str());
    if (algo == "aes")
    {
        auto mod = str_to_aes_mode(mode);
        return libcpp::aes::is_iv_valid(mod, iv_ptr, iv.size());
    }
    else if (algo == "des")
    {
        auto mod = str_to_des_mode(mode);
        return libcpp::des::is_iv_valid(mod, iv_ptr, iv.size());
    }
    else if (algo == "base64" || algo == "md5" || algo == "sha256")
    {
        return true;
    }

    return false; 
}

bool is_decrypt_fmt_valid(const std::string& fmt)
{
    return std::find(std::begin(fmts), std::end(fmts), fmt) != std::end(fmts);
}

err_t decrypt_aes(
    std::string& out,
    const std::string& in,
    const std::string& algo,
    const std::string& mode,
    const std::string& key,
    const std::string& padding,
    const std::string& iv,
    const std::string& ctx)
{
    auto mod = str_to_aes_mode(mode);
    auto pad = str_to_aes_padding(padding);
    bool succ = false;
    if (out == "" && in == "") // decrypt: mem -> mem
    {
        succ = libcpp::aes::decrypt(out, ctx, key, mod, pad, iv);
    }
    else if (out != "" && in != "") // decrypt: file -> file
    {
        succ = libcpp::aes::decrypt_file(out, in, key, mod, pad, iv);
    }
    else if (out != "" && in == "") // decrypt: mem -> file
    {
        std::ofstream fout(out, std::ios::binary);
        std::istringstream sin(ctx);
        succ = libcpp::aes::decrypt(fout, sin, reinterpret_cast<const unsigned char*>(key.c_str()), key.size(),
            mod, pad, reinterpret_cast<const unsigned char*>(iv.c_str()), iv.size());
    }
    else // decrypt: file -> mem
    {
        std::ifstream fin(in, std::ios::binary);
        std::ostringstream sout(out);
        succ = libcpp::aes::decrypt(sout, fin, reinterpret_cast<const unsigned char*>(key.c_str()), key.size(),
            mod, pad, reinterpret_cast<const unsigned char*>(iv.c_str()), iv.size());
        out = sout.str();
    }

    return succ ? err_t() : error(err_decrypt_aes_failed);
}

err_t decrypt_base64(
    std::string& out,
    const std::string& in, 
    const std::string& ctx)
{
    bool succ = false;
    if (out == "" && in == "") // decrypt: mem -> mem
    {
        succ = libcpp::base64::decode(out, ctx);
    }
    else if (out != "" && in != "") // decrypt: file -> file
    {
        succ = libcpp::base64::decode_file(out, in);
    }
    else if (out != "" && in == "") // decrypt: mem -> file
    {
        std::ofstream fout(out, std::ios::binary);
        std::istringstream sin(ctx);
        succ = libcpp::base64::decode(fout, sin);
    }
    else // decrypt: file -> mem
    {
        std::ifstream fin(in, std::ios::binary);
        std::ostringstream sout(out);
        succ = libcpp::base64::decode(sout, fin);
        out = sout.str();
    }

    return succ ? err_t() : error(err_decrypt_base64_failed);
}

err_t decrypt_des(
    std::string& out,
    const std::string& in,
    const std::string& algo,
    const std::string& mode,
    const std::string& key,
    const std::string& padding,
    const std::string& iv,
    const std::string& ctx)
{
    auto mod = str_to_des_mode(mode);
    auto pad = str_to_des_padding(padding);
    bool succ = false;
    if (out == "" && in == "") // decrypt: mem -> mem
    {
        succ = libcpp::des::decrypt(out, ctx, key, mod, pad, iv);
    }
    else if (out != "" && in != "") // decrypt: file -> file
    {
        succ = libcpp::des::decrypt_file(out, in, key, mod, pad, iv);
    }
    else if (out != "" && in == "") // decrypt: mem -> file
    {
        std::ofstream fout(out, std::ios::binary);
        std::istringstream sin(ctx);
        succ = libcpp::des::decrypt(fout, sin, reinterpret_cast<const unsigned char*>(key.c_str()), key.size(),
            mod, pad, reinterpret_cast<const unsigned char*>(iv.c_str()), iv.size());
    }
    else // decrypt: file -> mem
    {
        std::ifstream fin(in, std::ios::binary);
        std::ostringstream sout(out);
        succ = libcpp::des::decrypt(sout, fin, reinterpret_cast<const unsigned char*>(key.c_str()), key.size(),
            mod, pad, reinterpret_cast<const unsigned char*>(iv.c_str()), iv.size());
        out = sout.str();
    }

    return succ ? err_t() : error(err_decrypt_des_failed);
}

err_t decrypt_rsa(
    std::string& out,
    const std::string& in,
    const std::string& algo,
    const std::string& mode,
    const std::string& key,
    const std::string& padding,
    const std::string& iv,
    const std::string& ctx)
{
    // TODO
    return err_t();
}