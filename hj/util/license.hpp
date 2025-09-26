// About JWT: https://www.jwt.io/
#ifndef LICENSE_HPP
#define LICENSE_HPP

#include <sys/stat.h>
#include <fstream>
#include <iostream>

#include <jwt-cpp/traits/nlohmann-json/traits.h>
#include <jwt-cpp/jwt.h>

namespace hj
{

namespace license
{

using json_traits = jwt::traits::nlohmann_json;

using claim_t = jwt::basic_claim<json_traits>;
using key_t = std::string;
using value_t = std::string;
using pair_t = std::pair<key_t, value_t>;
using license_t = jwt::builder<jwt::default_clock, json_traits>;
using token_t = jwt::traits::nlohmann_json::string_type;
using time_point_t = std::chrono::time_point<std::chrono::system_clock>;
using err_t = std::error_code;
using verify_ctx_t = jwt::verify_ops::verify_context<json_traits>;

enum class sign_algo
{
    none,
    rsa256,
};

enum class err 
{
    err_start = 1000,
    invalid_times,
    claim_mismatch,
    keys_not_changed,
    keys_not_enough,
    file_not_exist,
    input_stream_invalid,
    output_stream_invalid
};

constexpr const char* JWT_TYPE = "JWT";

namespace detail
{
class error_category : public std::error_category 
{
public:
    const char* name() const noexcept override { return "license"; }
    std::string message(int ev) const override 
    {
        switch (static_cast<err>(ev)) 
        {
            case err::err_start: return "error start";
            case err::invalid_times: return "invalid times";
            case err::claim_mismatch: return "claim mismatch";
            case err::keys_not_changed: return "keys not changed";
            case err::keys_not_enough: return "keys not enough";
            case err::file_not_exist: return "file not exist";
            case err::input_stream_invalid: return "input stream invalid";
            case err::output_stream_invalid: return "output stream invalid";
            default: return "unknown";
        }
    }
};

inline const error_category& license_category() 
{
    static error_category cat;
    return cat;
}

static inline std::error_code make_err(err e) { return std::error_code(static_cast<int>(e), license_category()); }

static err_t issue(token_t& token,
                   const std::string& issuer,
                   const std::string& licensee,
                   const time_point_t issue_tm,
                   const std::size_t leeway_secs,
                   const sign_algo algo,
                   const std::vector<std::string>& keys,
                   const std::vector<pair_t>& claims)
{
    err_t err;
    auto lic = jwt::create<json_traits>();
    lic.set_type(JWT_TYPE);
    lic.set_issuer(issuer);
    lic.set_audience(licensee);
    lic.set_issued_at(issue_tm);
    lic.set_expires_at(issue_tm + std::chrono::seconds(leeway_secs));
    for (const auto& p : claims)
        lic.set_payload_claim(p.first, claim_t(p.second));

    switch (algo)
    {
        case sign_algo::rsa256:
        {
            if (keys.size() < 4)
                return detail::make_err(err::keys_not_enough);
            // public_key, private_key, public_key_password, private_key_password
            token = lic.sign(jwt::algorithm::rs256(keys.at(0), keys.at(1), keys.at(2), keys.at(3)), err);
            break;
        }
        default:
        {
            token = lic.sign(jwt::algorithm::none{}, err);
            break;
        }
    }

    return err;
}

static err_t verify(const token_t& token,
                    const std::string& issuer,
                    const std::string& licensee,
                    const std::size_t leeway_secs,
                    const sign_algo algo,
                    const std::vector<std::string>& keys,
                    const std::vector<pair_t>& claims)
{
    err_t err;
    auto decoded = jwt::decode<json_traits>(token);
    auto verifier = jwt::verify<json_traits>()
        .with_type(JWT_TYPE)
        .with_issuer(issuer)
        .with_audience(licensee)
        .expires_at_leeway(std::chrono::seconds(leeway_secs).count());

    switch (algo)
    {
        case sign_algo::rsa256:
        {
            verifier.allow_algorithm(jwt::algorithm::rs256(keys.at(0), keys.at(1), keys.at(2), keys.at(3)));
            break;
        }
        default:
        {
            verifier.allow_algorithm(jwt::algorithm::none{});
            break;
        }
    }

    verifier.verify(decoded, err);
    if (err)
        return err;

    for (const auto& p : claims)
    {
        if (decoded.get_payload_claim(p.first).as_string() != p.second)
            return detail::make_err(err::claim_mismatch);
    }
    return err;
}

static std::string get_disk_sn()
{
#ifdef _WIN32
    char volumeName[MAX_PATH + 1] = {0};
    char fileSystemName[MAX_PATH + 1] = {0};
    DWORD serialNumber = 0, maxComponentLen = 0, fileSystemFlags = 0;
    if (GetVolumeInformationA(
        "C:\\",
        volumeName,
        sizeof(volumeName),
        &serialNumber,
        &maxComponentLen,
        &fileSystemFlags,
        fileSystemName,
        sizeof(fileSystemName)))
    {
        char sn[32] = {0};
        sprintf_s(sn, "%08X", serialNumber);
        return std::string(sn);
    }
    return std::string();

#elif defined(__linux__)
    FILE* fp = fopen("/sys/block/sda/device/serial", "r");
    if (fp) 
    {
        char buf[128] = {0};
        if (fgets(buf, sizeof(buf), fp)) 
        {
            fclose(fp);
            size_t len = strlen(buf);
            if (len > 0 && buf[len-1] == '\n') 
                buf[len-1] = '\0';

            return std::string(buf);
        }
        fclose(fp);
    }

    FILE* cmd = popen("lsblk -ndo SERIAL /dev/sda", "r");
    if (cmd) 
    {
        char buf[128] = {0};
        if (fgets(buf, sizeof(buf), cmd)) 
        {
            pclose(cmd);
            size_t len = strlen(buf);
            if (len > 0 && buf[len-1] == '\n') 
                buf[len-1] = '\0';

            return std::string(buf);
        }
        pclose(cmd);
    }
    return std::string();

#elif defined(__APPLE__)
    FILE* cmd = popen("ioreg -l | grep \"Serial Number\" | grep IOPlatformSerialNumber | awk -F\"\" '{print $4}'", "r");
    if (cmd) 
    {
        char buf[128] = {0};
        if (fgets(buf, sizeof(buf), cmd)) 
        {
            pclose(cmd);
            size_t len = strlen(buf);
            if (len > 0 && buf[len-1] == '\n') 
                buf[len-1] = '\0';

            return std::string(buf);
        }
        pclose(cmd);
    }
    return std::string();

#else
    return std::string();

#endif
}

} // namespace detail

static inline std::string get_disk_sn() { return detail::get_disk_sn(); }
static inline bool is_file_exist(const std::string& filepath)
{
    struct stat buffer;
    return stat(filepath.c_str(), &buffer) == 0;
}

class issuer
{
public:
    issuer() = delete;
    issuer(const std::string& id,
           const sign_algo algo,
           const std::vector<std::string>& keys,
           const size_t valid_times)
        : _id{id}
        , _algo{algo}
        , _keys{keys}
        , _valid_times{valid_times}
    {
    }
    issuer(const issuer& other)
        : _id{other._id}
        , _algo{other._algo}
        , _keys{other._keys}
        , _valid_times{other._valid_times}
    {
    }
    virtual ~issuer() = default;
    issuer& operator=(const issuer& other)
    {
        _id = other._id;
        _algo = other._algo;
        _keys = other._keys;
        _valid_times = other._valid_times;
        return *this;
    }

    inline const std::string& id() const { return _id; }
    inline sign_algo algo() const { return _algo; }
    inline std::size_t valid_times() const { return _valid_times; }

    err_t issue(token_t& token,
                const std::string& licensee,
                const std::size_t leeway_days,
                const std::vector<pair_t>& claims = {})
    {
        err_t err;
        if (_valid_times < 1)
            return detail::make_err(err::invalid_times);

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

    err_t issue(std::ostream& out,
                const std::string& licensee,
                const std::size_t leeway_days,
                const std::vector<pair_t>& claims = {})
    {
        if (!out || !out.good())
            return detail::make_err(err::output_stream_invalid);

        token_t token;
        auto err = issue(token, licensee, leeway_days, claims);
        if (err)
            return err;

        out << token;
        if (!out.good())
            return detail::make_err(err::output_stream_invalid);
        return err_t{};
    }

    err_t issue_file(const std::string& filepath,
                     const std::string& licensee,
                     const std::size_t leeway_days,
                     const std::vector<pair_t>& claims = {})
    {
        std::ofstream out(filepath, std::ios::binary);
        if (!out || !out.is_open())
            return detail::make_err(err::file_not_exist);

        return issue(out, licensee, leeway_days, claims);
    }

    // release all issued licences by reset encrypt keys
    err_t release(const sign_algo algo,
                  const std::vector<std::string>& keys)
    {
        if (algo == _algo && keys == _keys)
            return detail::make_err(err::keys_not_changed);

        err_t err;
        _algo = algo;
        _keys = keys;
        return err;
    }

private:
    std::string              _id;
    sign_algo                _algo;
    std::vector<std::string> _keys;
    std::size_t              _valid_times;
};

class verifier
{
public:
    verifier() = delete;
    verifier(const std::string& id,
             const sign_algo algo,
             const std::vector<std::string>& keys)
        : _id{id}
        , _algo{algo}
        , _keys{keys}
    {
    }
    verifier(const verifier& other)
        : _id{other._id}
        , _algo{other._algo}
        , _keys{other._keys}
    {
    }
    virtual ~verifier() = default;

    err_t verify(const token_t& token,
                 const std::string& licensee,
                 const std::size_t leeway_days,
                 const std::vector<pair_t>& claims = {})
    {
        return detail::verify(token, 
                              _id, 
                              licensee, 
                              leeway_days * 24 * 60 * 60, 
                              _algo, 
                              _keys, 
                              claims);
    }

    err_t verify(std::istream& in, 
                 const std::string& licensee,
                 const std::size_t leeway_days,
                 const std::vector<pair_t>& claims = {})
    {
        if (!in)
            return detail::make_err(err::input_stream_invalid);

        std::string line, buf;
        while (std::getline(in, line)) 
            buf += line;

        if (buf.empty())
            return detail::make_err(err::input_stream_invalid);

        token_t token = buf;
        return detail::verify(token, _id, licensee, leeway_days * 24 * 60 * 60, _algo, _keys, claims);
    }

    err_t verify_file(const token_t& filepath,
                      const std::string& licensee,
                      const std::size_t leeway_days,
                      const std::vector<pair_t>& claims = {})
    {
        std::ifstream in(filepath, std::ios::binary);
        if (!in || !in.is_open())
            return detail::make_err(err::file_not_exist);

        return verify(in, licensee, leeway_days, claims);
    }

private:
    std::string              _id;
    sign_algo                _algo;
    std::vector<std::string> _keys;
};

} // namespace license
} // namespace hj

#endif // LICENSE_HPP