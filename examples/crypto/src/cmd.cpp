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
        handle_err(error(err_invalid_key), key);
        return;
    }
    if (!is_encrypt_padding_valid(padding, algo)) 
    {
        handle_err(error(err_invalid_padding), padding);
        return;
    }
    if (!is_encrypt_iv_valid(iv, algo, mode)) 
    {
        handle_err(error(err_invalid_iv), iv);
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
        handle_err(error(err_invalid_input), in, ctx);
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
        auto err = encrypt_rsa(out, in, key, padding, ctx);
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
    std::string& passwd,
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
        handle_err(error(err_invalid_decrypt_key), key);
        return;
    }
    if (!is_decrypt_padding_valid(padding, algo)) 
    {
        handle_err(error(err_invalid_decrypt_padding), padding);
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
        handle_err(error(err_invalid_decrypt_input), in, ctx);
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
        auto err = decrypt_rsa(out, in, key, padding, passwd, ctx);
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

void guess(
    std::string& out,
    std::string& in,
    std::string& algo,
    std::string& mode,
    std::string& key,
    std::string& passwd,
    std::string& padding,
    std::string& iv,
    std::string& fmt,
    std::string& ctx,
    std::size_t timeout_sec)
{
    // TODO
}

void dict()
{
    // TODO
}

void attach()
{
    // TODO
}

void help()
{
    print_console(R"(
usage: crypto [-o | --output] [-i | --input] [-a | --algo] [-m | --mode] [-k | --key]
              [-p | --padding] [-v | --iv] [-f | --fmt] [-n | --name] [-b | --bits] 
              [-h | --help] [-t | --timeout] [-s | --set] [-d | --del] [-l | --list]
              <command> [<args>]

These are common Git commands used in various situations:

encrypt or decrypt content
   encrypt    Encrypt a file or content
   decrypt    Decrypt a file or content

generate keys
   keygen     Generate public and private keys

guess the encryption algo
   guess      Guess the encryption algo by input file or content
   dict       Use dictionary to guess the encryption algo

attach to a process
   attach     Attach to a process to monitor its crypto operations

'crypto help -a' and 'crypto help -g' list available subcommands and some
concept guides. See 'crypto help <command>' or 'crypto help <concept>'
to read about a specific subcommand or concept.
See 'crypto help crypto' for an overview of the system.
)");
}