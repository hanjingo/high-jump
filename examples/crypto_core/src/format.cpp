#include "format.h"

#include <hj/crypto/base64.hpp>
#include <hj/encoding/hex.hpp>
#include <hj/io/filepath.hpp>
#include <hj/math/random.hpp>

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
            return CRYPTO_ERR_FORMAT_INVALID_TARGET;
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
            return CRYPTO_ERR_FORMAT_INVALID_TARGET;
    }
}

int formator::_format_memory(
    std::string& out,
    const std::string& in,
    const std::string& fmt)
{
    int err = CRYPTO_ERR_FAIL;
    std::string tmp = in;
    if (fmt == "hex")
    {
        tmp = hj::hex::encode(in, true); // upper case
        err = (!tmp.empty()) ? CRYPTO_OK : CRYPTO_ERR_FORMAT_HEX_FAILED;
    }
    else if (fmt == "base64")
    {
        err = hj::base64::encode(tmp, in) ? CRYPTO_OK : CRYPTO_ERR_FORMAT_BASE64_FAILED;
    }
    else if (fmt == "none" || fmt == "")
    {
        err = CRYPTO_OK;
    }
    else
    {
        err = CRYPTO_ERR_FORMAT_STYLE_NOT_FOUND;
    }

    if (err == CRYPTO_OK)
        out = tmp;

    return err;
}

int formator::_unformat_memory(
    std::string& out,
    const std::string& in,
    const std::string& fmt)
{
    int err = CRYPTO_ERR_FAIL;
    std::string tmp = in;
    if (fmt == "hex")
    {
        tmp = hj::hex::decode(in); // upper case
        err = (!tmp.empty()) ? CRYPTO_OK : CRYPTO_ERR_UNFORMAT_HEX_FAILED;
    }
    else if (fmt == "base64")
    {
        err = hj::base64::decode(out, in) ? 
            CRYPTO_OK : CRYPTO_ERR_UNFORMAT_BASE64_FAILED;
    }
    else if (fmt == "none" || fmt == "")
    {
        err = CRYPTO_OK;
    }
    else
    {
        err = CRYPTO_ERR_UNFORMAT_STYLE_NOT_FOUND;
    }

    if (err == CRYPTO_OK)
        out = tmp;

    return CRYPTO_OK;
}

int formator::_format_file(
    const std::string& out,
    const std::string& in,
    const std::string& fmt)
{
    int err = CRYPTO_ERR_FAIL;
    std::string tmp = "";
    if (out == in)
    {
        // the same file
        auto name = hj::filepath::file_name(out, false);
        auto ext = hj::filepath::extension(out);
        auto path = hj::filepath::path_name(out);
        tmp = hj::filepath::join(path, name + std::to_string(hj::random::range<0, 999>()) + ext);
    }

    // do format
    if (fmt == "hex")
    {
        // not same file
        if (tmp == "")
        {
            err = hj::hex::encode_file(out, in) ? 
                CRYPTO_OK : CRYPTO_ERR_FORMAT_HEX_FAILED;
        }
        else
        {
            // same file
            hj::filepath::copy_file(in, tmp);
            err = hj::hex::encode_file(out, tmp) ? 
                CRYPTO_OK : CRYPTO_ERR_FORMAT_HEX_FAILED;
            hj::filepath::remove(tmp);
        }
    }
    else if (fmt == "base64")
    {
        if (tmp == "")
        {
            // not same file
            err = hj::base64::encode_file(out, in) ? 
                CRYPTO_OK : CRYPTO_ERR_FORMAT_BASE64_FAILED;
        }
        else
        {
            // same file
            hj::filepath::copy_file(in, tmp);
            err = hj::base64::encode_file(out, tmp) ? 
                CRYPTO_OK : CRYPTO_ERR_FORMAT_BASE64_FAILED;
            hj::filepath::remove(tmp);
        }
    }
    else if (fmt == "none" || fmt == "")
    {
        err = CRYPTO_OK;
    }
    else
    {
        err = CRYPTO_ERR_FORMAT_STYLE_NOT_FOUND;
    }

    return err;
}

int formator::_unformat_file(
    const std::string& out,
    const std::string& in,
    const std::string& fmt)
{
    int err = CRYPTO_ERR_FAIL;
    std::string tmp = "";
    if (out == in)
    {
        // the same file
        auto name = hj::filepath::file_name(out, false);
        auto ext = hj::filepath::extension(out);
        auto path = hj::filepath::path_name(out);
        tmp = hj::filepath::join(path, name + std::to_string(hj::random::range<0, 999>()) + ext);
    }

    // do unformat
    if (fmt == "hex")
    {
        if (tmp == "")
        {
            // not same file
            err = hj::hex::decode_file(out, in) ? 
                CRYPTO_OK : CRYPTO_ERR_UNFORMAT_HEX_FAILED;
        }
        else
        {
            // same file
            hj::filepath::copy_file(in, tmp);
            err = hj::hex::decode_file(out, tmp) ? 
                CRYPTO_OK : CRYPTO_ERR_UNFORMAT_HEX_FAILED;
            hj::filepath::remove(tmp);
        }
    }
    else if (fmt == "base64")
    {
        if (tmp == "")
        {
            // not same file
            err = hj::base64::decode_file(out, in) ? 
                CRYPTO_OK : CRYPTO_ERR_FORMAT_BASE64_FAILED;
        }
        else
        {
            // same file
            hj::filepath::copy_file(in, tmp);
            err = hj::base64::decode_file(out, tmp) ? 
                CRYPTO_OK : CRYPTO_ERR_FORMAT_BASE64_FAILED;
            hj::filepath::remove(tmp);
        }
    }
    else if (fmt == "none" || fmt == "")
    {
        err = CRYPTO_OK;
    }
    else
    {
        err = CRYPTO_ERR_FORMAT_STYLE_NOT_FOUND;
    }

    return err;
}