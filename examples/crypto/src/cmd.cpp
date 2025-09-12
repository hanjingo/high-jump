#include "cmd.h"

#include "err.h"
#include "err_handle.h"
#include "comm.h"

void encrypt(
    std::string& out,
    std::string& in,
    std::string& algo,
    std::string& mode,
    std::string& key,
    std::string& padding,
    std::string& iv,
    std::string& fmt,
    std::string& ctx)
{
    if (!is_encrypt_algo_valid(algo)) 
    {
        handle_err(error(err_invalid_algo), algo);
        return;
    }
    if (!is_encrypt_key_valid(key, algo, mode)) 
    {
        handle_err(error(err_invalid_key), key, algo, mode);
        return;
    }
    if (!is_encrypt_padding_valid(padding, algo)) 
    {
        handle_err(error(err_invalid_padding), padding, algo);
        return;
    }
    if (!is_encrypt_iv_valid(iv, algo, mode)) 
    {
        handle_err(error(err_invalid_iv), iv, algo, mode);
        return;
    }
    if (!is_encrypt_fmt_valid(fmt)) 
    {
        handle_err(error(err_invalid_fmt), fmt);
        return;
    }
    if (!is_encrypt_output_valid(out)) 
    {
        handle_err(error(err_invalid_output), out);
        return;
    }
    if (!is_encrypt_input_valid(in, ctx, algo, mode, key, padding)) 
    {
        handle_err(error(err_invalid_input), in, ctx, algo, mode, key, padding);
        return;
    }

    bool to_console = (out == "");
    if (algo == "aes")
    {
        auto err = encrypt_aes(out, in, algo, mode, key, padding, iv, ctx);
        if (err)
        {
            handle_err(err, out, in, algo, mode, key, padding, iv, ctx);
            return;
        }
        if (to_console)
            print_console(out, fmt);
    }
    else if (algo == "base64")
    {
        auto err = encrypt_base64(out, in, ctx);
        if (err)
        {
            handle_err(err, out, in, ctx);
            return;
        }
        if (to_console)
            print_console(out, fmt);
    }
    else if (algo == "des")
    {
        auto err = encrypt_des(out, in, algo, mode, key, padding, iv, ctx);
        if (err)
        {
            handle_err(err, out, in, ctx);
            return;
        }
        if (to_console)
            print_console(out, fmt);
    }
    else if (algo == "md5")
    {
        auto err = encrypt_md5(out, in, ctx);
        if (err)
        {
            handle_err(err, out, in, ctx);
            return;
        }
        if (to_console)
            print_console(out, fmt);
    }
    else if (algo == "sha256")
    {
        auto err = encrypt_sha256(out, in, ctx);
        if (err)
        {
            handle_err(err, out, in, ctx);
            return;
        }
        if (to_console)
            print_console(out, fmt);
    }
    else if (algo == "rsa")
    {
        auto err = encrypt_rsa(out, in, algo, mode, key, padding, iv, ctx);
        if (err)
        {
            handle_err(err, out, in, ctx);
            return;
        }
        if (to_console)
            print_console(out, fmt);
    }
    else if (algo == "auto")
    {
        // TODO
    }
}

void decrypt(
    std::string& out,
    std::string& in,
    std::string& algo,
    std::string& mode,
    std::string& key,
    std::string& padding,
    std::string& iv,
    std::string& fmt,
    std::string& ctx)
{
    if (!is_decrypt_algo_valid(algo)) 
    {
        handle_err(error(err_invalid_decrypt_algo), algo);
        return;
    }
    if (!is_decrypt_key_valid(key, algo, mode)) 
    {
        handle_err(error(err_invalid_decrypt_key), key, algo, mode);
        return;
    }
    if (!is_decrypt_padding_valid(padding, algo)) 
    {
        handle_err(error(err_invalid_decrypt_padding), padding, algo);
        return;
    }
    if (!is_decrypt_iv_valid(iv, algo, mode)) 
    {
        handle_err(error(err_invalid_decrypt_iv), iv, algo, mode);
        return;
    }
    if (!is_decrypt_fmt_valid(fmt)) 
    {
        handle_err(error(err_invalid_fmt), fmt);
        return;
    }
    if (!is_decrypt_output_valid(out)) 
    {
        handle_err(error(err_invalid_decrypt_output), out);
        return;
    }
    if (!is_decrypt_input_valid(in, ctx, algo, key, padding)) 
    {
        handle_err(error(err_invalid_decrypt_input), in, ctx, algo, key, padding);
        return;
    }

    std::string buf = ctx;
    ctx.clear();
    if (buf != "")
        unformat(ctx, buf, fmt);

    bool to_console = (out == "");
    if (algo == "aes")
    {
        auto err = decrypt_aes(out, in, algo, mode, key, padding, iv, ctx);
        if (err)
        {
            handle_err(err, out, in, algo, mode, key, padding, iv, ctx);
            return;
        }
        if (to_console)
            print_console(out);
    }
    else if (algo == "base64")
    {
        auto err = decrypt_base64(out, in, ctx);
        if (err)
        {
            handle_err(err, out, in, ctx);
            return;
        }
        if (to_console)
            print_console(out);
    }
    else if (algo == "des")
    {
        auto err = decrypt_des(out, in, algo, mode, key, padding, iv, ctx);
        if (err)
        {
            handle_err(err, out, in, ctx);
            return;
        }
        if (to_console)
            print_console(out);
    }
    else if (algo == "rsa")
    {
        auto err = decrypt_rsa(out, in, algo, mode, key, padding, iv, ctx);
        if (err)
        {
            handle_err(err, out, in, ctx);
            return;
        }
        if (to_console)
            print_console(out);
    }
    else if (algo == "auto")
    {
        // TODO
    }
}

void keygen(
    std::string& name,
    std::string& algo,
    std::string& fmt,
    std::string& mode,
    int bits)
{
    bool to_console = (name == "");
    if (algo == "rsa")
    {
        std::string pubkey;
        std::string prikey;
        auto err = rsa_keygen(pubkey, prikey, name, fmt, mode, bits);
        if (err)
        {
            handle_err(err, name, fmt, mode, bits);
            return;
        }
        if (to_console)
        {
            print_console(pubkey);
            print_console(prikey);
        }
    }
}

void help()
{

}