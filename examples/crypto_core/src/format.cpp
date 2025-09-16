#include "format.h"

#include <libcpp/crypto/base64.hpp>
#include <libcpp/encoding/hex.hpp>
#include <libcpp/io/filepath.hpp>
#include <libcpp/math/random.hpp>

#include "api.h"

int formator::format(
    std::string& out,
    const std::string& in,
    const std::string& fmt,
    const fmt_target tgt)
{
    switch (tgt)
    {
        case fmt_target::memory:
            return _format_memory(out, in, fmt);
        case fmt_target::file:
            return _format_file(out, in, fmt);
        default:
            return ERR_FORMAT_INVALID_TARGET;
    }
}

int formator::unformat(
    std::string& out,
    const std::string& in,
    const std::string& fmt,
    const fmt_target tgt)
{
    switch (tgt)
    {
        case fmt_target::memory:
            return _unformat_memory(out, in, fmt);
        case fmt_target::file:
            return _unformat_file(out, in, fmt);
        default:
            return ERR_FORMAT_INVALID_TARGET;
    }
}

int formator::_format_memory(
    std::string& out,
    const std::string& in,
    const std::string& fmt)
{
    int err = FAIL;
    std::string tmp = in;
    if (fmt == "hex")
    {
        tmp = libcpp::hex::encode(in, true); // upper case
        err = (!tmp.empty()) ? OK : ERR_FORMAT_HEX_FAILED;
    }
    else if (fmt == "base64")
    {
        err = libcpp::base64::encode(tmp, in) ? OK : ERR_FORMAT_BASE64_FAILED;
    }
    else if (fmt == "none" || fmt == "")
    {
        err = OK;
    }
    else
    {
        err = ERR_FORMAT_STYLE_NOT_FOUND;
    }

    if (err == OK)
        out = tmp;

    return err;
}

int formator::_unformat_memory(
    std::string& out,
    const std::string& in,
    const std::string& fmt)
{
    int err = FAIL;
    std::string tmp = in;
    if (fmt == "hex")
    {
        tmp = libcpp::hex::decode(in); // upper case
        err = (!tmp.empty()) ? OK : ERR_UNFORMAT_HEX_FAILED;
    }
    else if (fmt == "base64")
    {
        err = libcpp::base64::decode(out, in) ? OK : ERR_UNFORMAT_BASE64_FAILED;
    }
    else if (fmt == "none" || fmt == "")
    {
        err = OK;
    }
    else
    {
        err = ERR_UNFORMAT_STYLE_NOT_FOUND;
    }

    if (err == OK)
        out = tmp;
        
    return OK;
}

int formator::_format_file(
    const std::string& out,
    const std::string& in,
    const std::string& fmt)
{
    int err = FAIL;
    std::string tmp = "";
    if (out == in)
    {
        // the same file
        auto name = libcpp::filepath::file_name(out, false);
        auto ext = libcpp::filepath::extension(out);
        auto path = libcpp::filepath::path_name(out);
        tmp = libcpp::filepath::join(path, name + std::to_string(libcpp::random::range<0, 999>()) + ext);
    }

    // do format
    if (fmt == "hex")
    {
        // not same file
        if (tmp == "")
        {
            err = libcpp::hex::encode_file(out, in) ? OK : ERR_FORMAT_HEX_FAILED;
        }
        else
        {
            // same file
            libcpp::filepath::copy_file(in, tmp);
            err = libcpp::hex::encode_file(out, tmp) ? OK : ERR_FORMAT_HEX_FAILED;
            libcpp::filepath::remove(tmp);
        }
    }
    else if (fmt == "base64")
    {
        if (tmp == "")
        {
            // not same file
            err = libcpp::base64::encode_file(out, in) ? OK : ERR_FORMAT_BASE64_FAILED;
        }
        else
        {
            // same file
            libcpp::filepath::copy_file(in, tmp);
            err = libcpp::base64::encode_file(out, tmp) ? OK : ERR_FORMAT_BASE64_FAILED;
            libcpp::filepath::remove(tmp);
        }
    }
    else if (fmt == "none" || fmt == "")
    {
        err = OK;
    }
    else
    {
        err = ERR_FORMAT_STYLE_NOT_FOUND;
    }

    return err;
}

int formator::_unformat_file(
    const std::string& out,
    const std::string& in,
    const std::string& fmt)
{
    int err = FAIL;
    std::string tmp = "";
    if (out == in)
    {
        // the same file
        auto name = libcpp::filepath::file_name(out, false);
        auto ext = libcpp::filepath::extension(out);
        auto path = libcpp::filepath::path_name(out);
        tmp = libcpp::filepath::join(path, name + std::to_string(libcpp::random::range<0, 999>()) + ext);
    }

    // do unformat
    if (fmt == "hex")
    {
        if (tmp == "")
        {
            // not same file
            err = libcpp::hex::decode_file(out, in) ? OK : ERR_UNFORMAT_HEX_FAILED;
        }
        else
        {
            // same file
            libcpp::filepath::copy_file(in, tmp);
            err = libcpp::hex::decode_file(out, tmp) ? OK : ERR_UNFORMAT_HEX_FAILED;
            libcpp::filepath::remove(tmp);
        }
    }
    else if (fmt == "base64")
    {
        if (tmp == "")
        {
            // not same file
            err = libcpp::base64::decode_file(out, in) ? OK : ERR_FORMAT_BASE64_FAILED;
        }
        else
        {
            // same file
            libcpp::filepath::copy_file(in, tmp);
            err = libcpp::base64::decode_file(out, tmp) ? OK : ERR_FORMAT_BASE64_FAILED;
            libcpp::filepath::remove(tmp);
        }
    }
    else if (fmt == "none" || fmt == "")
    {
        err = OK;
    }
    else
    {
        err = ERR_FORMAT_STYLE_NOT_FOUND;
    }

    return err;
}