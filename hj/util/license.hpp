/*
 *  This file is part of hj.
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// JWT-based license management system
// About JWT: https://www.jwt.io/
#ifndef LICENSE_HPP
#define LICENSE_HPP

#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <mutex>

#include <jwt-cpp/traits/nlohmann-json/traits.h>
#include <jwt-cpp/jwt.h>

namespace hj
{
namespace license
{

using json_traits = jwt::traits::nlohmann_json;

using claim_t      = jwt::basic_claim<json_traits>;
using key_t        = std::string;
using value_t      = std::string;
using pair_t       = std::pair<key_t, value_t>;
using license_t    = jwt::builder<jwt::default_clock, json_traits>;
using token_t      = jwt::traits::nlohmann_json::string_type;
using time_point_t = std::chrono::time_point<std::chrono::system_clock>;
using err_t        = std::error_code;
using verify_ctx_t = jwt::verify_ops::verify_context<json_traits>;

enum class sign_algo
{
    none,
    rsa256,
};

enum class error_code
{
    ok        = 0,
    err_start = 1000,
    invalid_times,
    claim_mismatch,
    keys_not_changed,
    keys_not_enough,
    file_not_exist,
    input_stream_invalid,
    output_stream_invalid,
    invalid_algorithm,
    crypto_error,
    system_call_failed,
    insufficient_permissions,
    network_error,
    timeout_error,
    memory_allocation_failed,
    invalid_configuration,
    parse_license_info_failed,
};

static const char *JWT_TYPE = "JWT";

namespace detail
{
class error_category : public std::error_category
{
  public:
    const char *name() const noexcept override { return "license"; }

    std::string message(int ev) const override
    {
        switch(static_cast<error_code>(ev))
        {
            case error_code::ok:
                return "Ok";
            case error_code::invalid_times:
                return "Invalid time configuration";
            case error_code::claim_mismatch:
                return "License claim verification failed";
            case error_code::keys_not_changed:
                return "Cryptographic keys unchanged";
            case error_code::keys_not_enough:
                return "Insufficient cryptographic keys";
            case error_code::file_not_exist:
                return "License file not found";
            case error_code::input_stream_invalid:
                return "Invalid input stream";
            case error_code::output_stream_invalid:
                return "Invalid output stream";
            case error_code::invalid_algorithm:
                return "Unsupported cryptographic algorithm";
            case error_code::crypto_error:
                return "Cryptographic operation failed";
            case error_code::system_call_failed:
                return "System call failed";
            case error_code::insufficient_permissions:
                return "Insufficient permissions";
            case error_code::network_error:
                return "Network operation failed";
            case error_code::timeout_error:
                return "Operation timeout";
            case error_code::memory_allocation_failed:
                return "Memory allocation failed";
            case error_code::invalid_configuration:
                return "Invalid configuration";
            case error_code::parse_license_info_failed:
                return "Failed to parse license information";
            default:
                return "Unknown error";
        }
    }

    bool
    equivalent(int                         code,
               const std::error_condition &condition) const noexcept override
    {
        return std::error_category::equivalent(code, condition);
    }
};

static inline std::error_code make_err(error_code e)
{
    static error_category cat;
    return std::error_code(static_cast<int>(e), cat);
}

static err_t issue(token_t                        &token,
                   const std::string              &issuer,
                   const std::string              &licensee,
                   const time_point_t              issue_tm,
                   const std::size_t               leeway_secs,
                   const sign_algo                 algo,
                   const std::vector<std::string> &keys,
                   const std::vector<pair_t>      &claims)
{
    err_t error_code;
    auto  lic = jwt::create<json_traits>();
    lic.set_type(JWT_TYPE);
    lic.set_issuer(issuer);
    lic.set_audience(licensee);
    lic.set_issued_at(issue_tm);
    lic.set_expires_at(issue_tm + std::chrono::seconds(leeway_secs));
    for(const auto &p : claims)
        lic.set_payload_claim(p.first, claim_t(p.second));

    switch(algo)
    {
        case sign_algo::rsa256: {
            if(keys.size() < 4)
                return detail::make_err(error_code::keys_not_enough);
            // public_key, private_key, public_key_password, private_key_password
            token = lic.sign(jwt::algorithm::rs256(keys.at(0),
                                                   keys.at(1),
                                                   keys.at(2),
                                                   keys.at(3)),
                             error_code);
            break;
        }
        default: {
            token = lic.sign(jwt::algorithm::none{}, error_code);
            break;
        }
    }

    return error_code;
}

static err_t verify(const token_t                  &token,
                    const std::string              &issuer,
                    const std::string              &licensee,
                    const std::size_t               leeway_secs,
                    const sign_algo                 algo,
                    const std::vector<std::string> &keys,
                    const std::vector<pair_t>      &claims)
{
    err_t error_code;
    auto  decoded = jwt::decode<json_traits>(token);
    auto  verifier =
        jwt::verify<json_traits>()
            .with_type(JWT_TYPE)
            .with_issuer(issuer)
            .with_audience(licensee)
            .expires_at_leeway(std::chrono::seconds(leeway_secs).count());

    switch(algo)
    {
        case sign_algo::rsa256: {
            verifier.allow_algorithm(jwt::algorithm::rs256(keys.at(0),
                                                           keys.at(1),
                                                           keys.at(2),
                                                           keys.at(3)));
            break;
        }
        default: {
            verifier.allow_algorithm(jwt::algorithm::none{});
            break;
        }
    }

    verifier.verify(decoded, error_code);
    if(error_code)
        return error_code;

    for(const auto &p : claims)
    {
        if(decoded.get_payload_claim(p.first).as_string() != p.second)
            return detail::make_err(error_code::claim_mismatch);
    }
    return error_code;
}

static std::string get_disk_sn() noexcept
{
#ifdef _WIN32
    try
    {
        char  volume_name[MAX_PATH + 1]      = {0};
        char  file_system_name[MAX_PATH + 1] = {0};
        DWORD serial_number = 0, max_component_len = 0, file_system_flags = 0;
        if(GetVolumeInformationA("C:\\",
                                 volume_name,
                                 sizeof(volume_name),
                                 &serial_number,
                                 &max_component_len,
                                 &file_system_flags,
                                 file_system_name,
                                 sizeof(file_system_name)))
        {
            char buffer[32] = {0};
            if(sprintf_s(buffer, sizeof(buffer), "%08X", serial_number) > 0)
                return std::string(buffer);
        }
        return std::string();
    }
    catch(...)
    {
        return std::string();
    }

#elif defined(__linux__)
    try
    {
        // Try multiple methods for better reliability
        std::vector<std::string> commands = {
            "lsblk -ndo SERIAL /dev/sda 2>/dev/null",
            "hdparm -I /dev/sda 2>/dev/null | grep 'Serial Number' | awk "
            "'{print $3}'",
            "cat /sys/block/sda/device/serial 2>/dev/null"};
        for(const auto &cmd : commands)
        {
            if(auto result = _execute_command_safe(cmd))
            {
                if(!result->empty())
                    return *result;
            }
        }
        return std::string();
    }
    catch(...)
    {
        return std::string();
    }

#elif defined(__APPLE__)
    try
    {
        auto result = _execute_command_safe(
            "ioreg -l | grep \"Serial Number\" | grep IOPlatformSerialNumber | "
            "awk -F'\"' '{print $4}' 2>/dev/null");
        return result && !result->empty() ? result : std::string();
    }
    catch(...)
    {
        return std::string();
    }

#else
    return std::string();

#endif
}

} // namespace detail

static inline std::string get_disk_sn()
{
    return detail::get_disk_sn();
}
static inline bool is_file_exist(const std::string &filepath)
{
    struct stat buffer;
    return stat(filepath.c_str(), &buffer) == 0;
}

class issuer
{
  public:
    issuer() = delete;
    issuer(const std::string              &id,
           const sign_algo                 algo,
           const std::vector<std::string> &keys,
           const size_t                    valid_times)
        : _id{id}
        , _algo{algo}
        , _keys{keys}
        , _valid_times{valid_times}
    {
    }
    issuer(const issuer &other)
        : _id{other._id}
        , _algo{other._algo}
        , _keys{other._keys}
        , _valid_times{other._valid_times}
    {
    }
    issuer(issuer &&) = default;

    virtual ~issuer() = default;

    issuer &operator=(const issuer &other)
    {
        _id          = other._id;
        _algo        = other._algo;
        _keys        = other._keys;
        _valid_times = other._valid_times;
        return *this;
    }
    issuer &operator=(issuer &&) = default;

    inline const std::string &id() const { return _id; }
    inline sign_algo          algo() const { return _algo; }
    inline std::size_t        valid_times() const { return _valid_times; }
    inline void               set_keys(std::vector<std::string> &&keys) noexcept
    {
        std::lock_guard<std::mutex> lock(_mu);
        _keys = std::move(keys);
    }

    err_t issue(token_t                   &token,
                const std::string         &licensee,
                const std::size_t          leeway_days,
                const std::vector<pair_t> &claims = {})
    {
        err_t error_code;
        if(_valid_times < 1)
            return detail::make_err(error_code::invalid_times);

        _valid_times--;
        return detail::issue(token,
                             _id,
                             licensee,
                             jwt::date::clock::now(),
                             leeway_days * 24 * 60 * 60,
                             _algo,
                             _keys,
                             claims);
    }

    err_t issue(std::ostream              &out,
                const std::string         &licensee,
                const std::size_t          leeway_days,
                const std::vector<pair_t> &claims = {})
    {
        if(!out || !out.good())
            return detail::make_err(error_code::output_stream_invalid);

        token_t token;
        auto    error_code = issue(token, licensee, leeway_days, claims);
        if(error_code)
            return error_code;

        out << token;
        if(!out.good())
            return detail::make_err(error_code::output_stream_invalid);
        return err_t{};
    }

    err_t issue_file(const std::string         &filepath,
                     const std::string         &licensee,
                     const std::size_t          leeway_days,
                     const std::vector<pair_t> &claims = {})
    {
        std::ofstream out(filepath, std::ios::binary);
        if(!out || !out.is_open())
            return detail::make_err(error_code::file_not_exist);

        return issue(out, licensee, leeway_days, claims);
    }

    // release all issued licences by reset encrypt keys
    err_t release(const sign_algo algo, const std::vector<std::string> &keys)
    {
        if(algo == _algo && keys == _keys)
            return detail::make_err(error_code::keys_not_changed);

        err_t error_code;
        _algo = algo;
        _keys = keys;
        return error_code;
    }

  private:
    mutable std::mutex       _mu;
    std::string              _id;
    sign_algo                _algo;
    std::vector<std::string> _keys;
    std::size_t              _valid_times;
};

struct license_info
{
    std::string                           issuer;
    std::string                           audience;
    std::chrono::system_clock::time_point issued_at;
    std::chrono::system_clock::time_point expires_at;
    std::vector<pair_t>                   claims;
};

class verifier
{
  public:
    verifier() = delete;
    verifier(const std::string              &id,
             const sign_algo                 algo,
             const std::vector<std::string> &keys)
        : _id{id}
        , _algo{algo}
        , _keys{keys}
    {
    }
    verifier(const verifier &other)
        : _id{other._id}
        , _algo{other._algo}
        , _keys{other._keys}
    {
    }
    verifier(verifier &&) = default;

    virtual ~verifier() = default;

    err_t verify(const token_t             &token,
                 const std::string         &licensee,
                 const std::size_t          leeway_days,
                 const std::vector<pair_t> &claims = {})
    {
        return detail::verify(token,
                              _id,
                              licensee,
                              leeway_days * 24 * 60 * 60,
                              _algo,
                              _keys,
                              claims);
    }

    err_t verify(std::istream              &in,
                 const std::string         &licensee,
                 const std::size_t          leeway_days,
                 const std::vector<pair_t> &claims = {})
    {
        if(!in)
            return detail::make_err(error_code::input_stream_invalid);

        std::string line, buf;
        while(std::getline(in, line))
            buf += line;

        if(buf.empty())
            return detail::make_err(error_code::input_stream_invalid);

        token_t token = buf;
        return detail::verify(token,
                              _id,
                              licensee,
                              leeway_days * 24 * 60 * 60,
                              _algo,
                              _keys,
                              claims);
    }

    err_t verify_file(const token_t             &filepath,
                      const std::string         &licensee,
                      const std::size_t          leeway_days,
                      const std::vector<pair_t> &claims = {})
    {
        std::ifstream in(filepath, std::ios::binary);
        if(!in || !in.is_open())
            return detail::make_err(error_code::file_not_exist);

        return verify(in, licensee, leeway_days, claims);
    }

    static err_t parse(license_info &info, const token_t &token) noexcept
    {
        try
        {
            auto decoded    = jwt::decode<json_traits>(token);
            info.issuer     = decoded.get_issuer();
            info.issued_at  = decoded.get_issued_at();
            info.expires_at = decoded.get_expires_at();
            auto audience   = decoded.get_audience();
            if(!audience.empty())
                info.audience = *audience.begin();

            std::string key;
            auto        claims = decoded.get_payload_json();
            for(const auto &pair : claims)
            {
                key = pair.first;
                if(key == "iss" || key == "aud" || key == "iat" || key == "exp")
                    continue;

                info.claims.emplace_back(key, pair.second.dump());
            }
            return detail::make_err(error_code::ok);
        }
        catch(...)
        {
            return detail::make_err(error_code::parse_license_info_failed);
        }
    }

  private:
    std::string              _id;
    sign_algo                _algo;
    std::vector<std::string> _keys;
};

} // namespace license
} // namespace hj

#endif // LICENSE_HPP