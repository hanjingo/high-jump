/*
 *  This file is part of high-jump(hj).
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

#include <algorithm>
#include <array>
#include <cctype>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

#ifdef _WIN32

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <winioctl.h>

#elif defined(__APPLE__)

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/storage/IOStorageDeviceCharacteristics.h>

#elif defined(__linux__)

#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

#endif

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
    invalid_token,
    invalid_algorithm,
    invalid_configuration,
    invalid_claim,
    invalid_input_stream,
    invalid_output_stream,
    param_too_big,
    claim_mismatch,
    keys_not_changed,
    keys_not_enough,
    file_not_exist,
    file_open_failed,
    crypto_error,
    system_call_failed,
    insufficient_permissions,
    network_error,
    timeout_error,
    memory_allocation_failed,
    parse_license_info_failed,
};

inline constexpr const char *JWT_TYPE = "JWT";

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
            case error_code::err_start:
                return "License Error start";
            case error_code::invalid_times:
                return "Invalid time configuration";
            case error_code::invalid_token:
                return "Invalid license token";
            case error_code::invalid_algorithm:
                return "Unsupported cryptographic algorithm";
            case error_code::invalid_configuration:
                return "Invalid license configuration";
            case error_code::invalid_claim:
                return "Invalid license claim";
            case error_code::invalid_input_stream:
                return "Invalid input stream";
            case error_code::invalid_output_stream:
                return "Invalid output stream";
            case error_code::param_too_big:
                return "Parameter too big";
            case error_code::claim_mismatch:
                return "License claim mismatch";
            case error_code::keys_not_changed:
                return "License keys not changed";
            case error_code::keys_not_enough:
                return "License keys not enough";
            case error_code::file_not_exist:
                return "File not exist";
            case error_code::file_open_failed:
                return "File open failed";
            case error_code::crypto_error:
                return "Cryptographic error";
            case error_code::system_call_failed:
                return "System call failed";
            case error_code::insufficient_permissions:
                return "Insufficient permissions";
            case error_code::network_error:
                return "Network error";
            case error_code::timeout_error:
                return "Timeout error";
            case error_code::memory_allocation_failed:
                return "Memory allocation failed";
            case error_code::parse_license_info_failed:
                return "Parse license info failed";
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
                   const std::size_t               valid_secs,
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
    lic.set_expires_at(issue_tm + std::chrono::seconds(valid_secs));
    for(const auto &p : claims)
        lic.set_payload_claim(p.first, claim_t(p.second));

    switch(algo)
    {
        case sign_algo::rsa256: {
            if(keys.size() < 2)
                return detail::make_err(error_code::keys_not_enough);
            // public_key, private_key, public_key_password(optional), private_key_password(optional)
            token =
                lic.sign(jwt::algorithm::rs256(keys[0],
                                               keys[1],
                                               keys.size() > 2 ? keys[2] : "",
                                               keys.size() > 3 ? keys[3] : ""),
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
                    const sign_algo                 algo,
                    const std::vector<std::string> &keys,
                    const std::vector<pair_t>      &claims,
                    std::size_t                     leeway_secs = 5) noexcept
{
    try
    {
        err_t error_code;
        auto  decoded  = jwt::decode<json_traits>(token);
        auto  verifier = jwt::verify<json_traits>()
                             .with_type(JWT_TYPE)
                             .with_issuer(issuer)
                             .with_audience(licensee)
                             .leeway(leeway_secs);

        switch(algo)
        {
            case sign_algo::rsa256: {
                if(keys.size() < 1)
                    return detail::make_err(error_code::keys_not_enough);

                verifier.allow_algorithm(
                    jwt::algorithm::rs256(keys[0],
                                          keys.size() > 1 ? keys[1] : "",
                                          keys.size() > 2 ? keys[2] : "",
                                          keys.size() > 3 ? keys[3] : ""));
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
    catch(const std::exception &e)
    {
        return detail::make_err(error_code::parse_license_info_failed);
    }
    catch(...)
    {
        return detail::make_err(error_code::parse_license_info_failed);
    }
}

struct disk_identity
{
    std::string id;
    std::string device;
    std::string type;
    bool        stable = false;
};

inline std::string trim(std::string value)
{
    const auto first = value.find_first_not_of(" \t\r\n");

    if(first == std::string::npos)
        return {};

    const auto last = value.find_last_not_of(" \t\r\n");

    value = value.substr(first, last - first + 1);

    return value;
}

inline bool valid_identity(const std::string &value)
{
    if(value.empty())
        return false;

    // Reject obvious placeholders.
    const std::string lower = [&] {
        std::string s = value;

        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        return s;
    }();

    return lower != "unknown" && lower != "none" && lower != "null"
           && lower != "n/a";
}

#ifdef _WIN32
inline std::optional<DWORD>
windows_physical_drive_number(const std::string &volume)
{
    HANDLE handle = CreateFileA(volume.c_str(),
                                0,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                nullptr,
                                OPEN_EXISTING,
                                0,
                                nullptr);
    if(handle == INVALID_HANDLE_VALUE)
        return std::nullopt;

    STORAGE_DEVICE_NUMBER number{};
    DWORD                 bytes_returned = 0;
    const BOOL            ok = DeviceIoControl(handle,
                                               IOCTL_STORAGE_GET_DEVICE_NUMBER,
                                               nullptr,
                                               0,
                                               &number,
                                               sizeof(number),
                                               &bytes_returned,
                                               nullptr);
    CloseHandle(handle);
    if(!ok)
        return std::nullopt;

    if(number.DeviceType != FILE_DEVICE_DISK
       && number.DeviceType != FILE_DEVICE_MASS_STORAGE)
    {
        return std::nullopt;
    }

    return number.DeviceNumber;
}

inline std::optional<std::string> windows_query_serial(DWORD device_number)
{
    std::string path   = "\\\\.\\PhysicalDrive" + std::to_string(device_number);
    HANDLE      handle = CreateFileA(path.c_str(),
                                     0,
                                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                                     nullptr,
                                     OPEN_EXISTING,
                                     0,
                                     nullptr);
    if(handle == INVALID_HANDLE_VALUE)
        return std::nullopt;

    STORAGE_PROPERTY_QUERY query{};
    query.PropertyId = StorageDeviceProperty;
    query.QueryType  = PropertyStandardQuery;
    std::array<std::byte, 4096> buffer{};
    DWORD                       bytes_returned = 0;
    const BOOL ok = DeviceIoControl(handle,
                                    IOCTL_STORAGE_QUERY_PROPERTY,
                                    &query,
                                    sizeof(query),
                                    buffer.data(),
                                    static_cast<DWORD>(buffer.size()),
                                    &bytes_returned,
                                    nullptr);
    CloseHandle(handle);

    if(!ok || bytes_returned < sizeof(STORAGE_DEVICE_DESCRIPTOR))
        return std::nullopt;

    const auto *descriptor =
        reinterpret_cast<const STORAGE_DEVICE_DESCRIPTOR *>(buffer.data());
    if(descriptor->SerialNumberOffset == 0)
        return std::nullopt;

    if(descriptor->SerialNumberOffset >= bytes_returned)
        return std::nullopt;

    const char *serial = reinterpret_cast<const char *>(buffer.data())
                         + descriptor->SerialNumberOffset;
    std::string result = trim(serial);
    if(!valid_identity(result))
        return std::nullopt;

    return result;
}

inline std::optional<disk_identity> get_disk_identity_windows()
{
    char volume_path[MAX_PATH + 1] = {};
    if(!GetVolumePathNameA("C:\\",
                           volume_path,
                           static_cast<DWORD>(sizeof(volume_path))))
    {
        return std::nullopt;
    }

    char volume_name[MAX_PATH + 1] = {};
    if(!GetVolumeNameForVolumeMountPointA(
           volume_path,
           volume_name,
           static_cast<DWORD>(sizeof(volume_name))))
    {
        return std::nullopt;
    }

    auto device_number = windows_physical_drive_number(volume_name);
    if(!device_number)
        return std::nullopt;

    auto serial = windows_query_serial(*device_number);
    if(!serial)
        return std::nullopt;

    disk_identity result;
    result.id     = *serial;
    result.device = "\\\\.\\PhysicalDrive" + std::to_string(*device_number);
    result.type   = "serial";
    result.stable = true;

    return result;
}
#endif // _WIN32

#ifdef __linux__
inline std::optional<std::pair<unsigned int, unsigned int>>
linux_root_device_number()
{
    std::ifstream in("/proc/self/mountinfo");

    if(!in)
        return std::nullopt;

    std::string line;
    while(std::getline(in, line))
    {
        std::istringstream iss(line);
        std::string        mount_id;
        std::string        parent_id;
        std::string        major_minor;
        std::string        root;
        std::string        mount_point;
        if(!(iss >> mount_id >> parent_id >> major_minor >> root
             >> mount_point))
        {
            continue;
        }

        if(mount_point != "/")
            continue;

        const auto pos = major_minor.find(':');
        if(pos == std::string::npos)
            return std::nullopt;

        try
        {
            const auto major = static_cast<unsigned int>(
                std::stoul(major_minor.substr(0, pos)));

            const auto minor = static_cast<unsigned int>(
                std::stoul(major_minor.substr(pos + 1)));

            return std::make_pair(major, minor);
        }
        catch(...)
        {
            return std::nullopt;
        }
    }

    return std::nullopt;
}

inline std::filesystem::path linux_sysfs_device_path(unsigned int major_number,
                                                     unsigned int minor_number)
{
    return std::filesystem::path("/sys/dev/block")
           / (std::to_string(major_number) + ":"
              + std::to_string(minor_number));
}

inline std::filesystem::path
linux_resolve_partition(const std::filesystem::path &path)
{
    std::error_code ec;

    if(std::filesystem::exists(path / "partition", ec))
    {
        return path.parent_path();
    }

    return path;
}

inline std::string linux_block_device_name(const std::filesystem::path &path)
{
    std::error_code ec;

    auto canonical = std::filesystem::weakly_canonical(path, ec);

    if(ec)
        return {};

    return canonical.filename().string();
}

inline std::optional<std::string>
linux_read_file(const std::filesystem::path &path)
{
    std::ifstream in(path);

    if(!in)
        return std::nullopt;

    std::string value;

    std::getline(in, value);

    value = trim(value);

    if(!valid_identity(value))
        return std::nullopt;

    return value;
}

inline std::vector<std::filesystem::path>
linux_get_slaves(const std::filesystem::path &device)
{
    std::vector<std::filesystem::path> result;

    const auto slaves = device / "slaves";

    std::error_code ec;

    if(!std::filesystem::exists(slaves, ec))
        return result;

    for(const auto &entry : std::filesystem::directory_iterator(slaves, ec))
    {
        if(ec)
            break;

        result.emplace_back(entry.path());
    }

    return result;
}

inline void
linux_collect_physical_devices(const std::filesystem::path &device,
                               std::set<std::string>       &visited,
                               std::set<std::string>       &physical_devices)
{
    std::error_code ec;

    const auto canonical = std::filesystem::weakly_canonical(device, ec);

    if(ec)
        return;

    const std::string key = canonical.string();

    if(!visited.insert(key).second)
        return;

    const auto slaves = linux_get_slaves(canonical);

    if(!slaves.empty())
    {
        for(const auto &slave : slaves)
        {
            linux_collect_physical_devices(slave, visited, physical_devices);
        }

        return;
    }

    auto device_path = linux_resolve_partition(canonical);

    auto name = linux_block_device_name(device_path);

    if(!name.empty())
        physical_devices.insert(name);
}

inline std::optional<std::string> linux_get_serial(const std::string &device)
{
    const auto path =
        std::filesystem::path("/sys/block") / device / "device" / "serial";

    if(auto serial = linux_read_file(path))
        return serial;

    // Some devices expose WWN instead.
    const auto wwn_path =
        std::filesystem::path("/sys/block") / device / "device" / "wwid";

    if(auto wwn = linux_read_file(wwn_path))
        return wwn;

    // SCSI-style VPD WWN fallback.
    const auto vpd_path =
        std::filesystem::path("/sys/block") / device / "device" / "vpd_pg83";

    if(std::filesystem::exists(vpd_path))
    {
        // Do not parse binary VPD blindly here.
        // A missing serial is intentionally treated as unavailable.
    }

    return std::nullopt;
}

inline std::optional<disk_identity> get_disk_identity_linux()
{
    auto root_device = linux_root_device_number();

    if(!root_device)
        return std::nullopt;

    const auto sysfs_path =
        linux_sysfs_device_path(root_device->first, root_device->second);

    std::error_code ec;

    if(!std::filesystem::exists(sysfs_path, ec))
        return std::nullopt;

    std::set<std::string> visited;
    std::set<std::string> physical_devices;

    linux_collect_physical_devices(sysfs_path, visited, physical_devices);

    if(physical_devices.empty())
        return std::nullopt;

    struct disk_record
    {
        std::string device;
        std::string id;
        std::string type;
    };

    std::vector<disk_record> disks;

    for(const auto &device : physical_devices)
    {
        auto id = linux_get_serial(device);

        if(!id)
            continue;

        disks.push_back({device, *id, "serial"});
    }

    if(disks.empty())
        return std::nullopt;

    // Deterministic ordering.
    std::sort(disks.begin(), disks.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.device < rhs.device;
    });

    // Single physical disk.
    if(disks.size() == 1)
    {
        disk_identity result;

        result.id     = disks.front().id;
        result.device = disks.front().device;
        result.type   = disks.front().type;
        result.stable = true;

        return result;
    }

    // RAID / multipath / other multi-disk topology.
    //
    // Do NOT arbitrarily select one disk.
    //
    // Build a deterministic composite identity.
    std::ostringstream identity;
    std::ostringstream devices;
    identity << "multi:";
    for(std::size_t i = 0; i < disks.size(); ++i)
    {
        if(i != 0)
        {
            identity << '|';
            devices << ',';
        }

        identity << disks[i].id;
        devices << disks[i].device;
    }

    disk_identity result;
    result.id     = identity.str();
    result.device = devices.str();
    result.type   = "serial-set";
    result.stable = true;
    return result;
}
#endif // __linux__

#ifdef __APPLE__
inline std::optional<std::string> get_disk_identity_macos()
{
    CFMutableDictionaryRef matching = IOServiceMatching("IOBlockStorageDevice");

    if(!matching)
        return std::nullopt;

    io_iterator_t iterator = IO_OBJECT_NULL;

    const kern_return_t kr =
        IOServiceGetMatchingServices(kIOMainPortDefault, matching, &iterator);

    if(kr != KERN_SUCCESS)
        return std::nullopt;

    std::vector<std::string> serials;

    io_service_t service;

    while((service = IOIteratorNext(iterator)) != IO_OBJECT_NULL)
    {
        CFTypeRef serial = IORegistryEntrySearchCFProperty(
            service,
            kIOServicePlane,
            CFSTR(kIOPropertySerialNumberKey),
            kCFAllocatorDefault,
            kIORegistryIterateRecursively | kIORegistryIterateParents);

        if(serial && CFGetTypeID(serial) == CFStringGetTypeID())
        {
            char buffer[512] = {};

            if(CFStringGetCString(static_cast<CFStringRef>(serial),
                                  buffer,
                                  sizeof(buffer),
                                  kCFStringEncodingUTF8))
            {
                std::string value = trim(buffer);

                if(valid_identity(value))
                    serials.push_back(std::move(value));
            }
        }

        if(serial)
            CFRelease(serial);

        IOObjectRelease(service);
    }

    IOObjectRelease(iterator);

    if(serials.empty())
        return std::nullopt;

    std::sort(serials.begin(), serials.end());
    serials.erase(std::unique(serials.begin(), serials.end()), serials.end());

    disk_identity result;

    if(serials.size() == 1)
    {
        result.id     = serials.front();
        result.type   = "serial";
        result.stable = true;
    } else
    {
        std::ostringstream identity;

        identity << "multi:";

        for(std::size_t i = 0; i < serials.size(); ++i)
        {
            if(i != 0)
                identity << '|';

            identity << serials[i];
        }

        result.id     = identity.str();
        result.type   = "serial-set";
        result.stable = true;
    }

    return result;
}
#endif // __APPLE__

inline std::optional<disk_identity> get_disk_identity()
{
#ifdef _WIN32
    return get_disk_identity_windows();

#elif defined(__linux__)
    return get_disk_identity_linux();

#elif defined(__APPLE__)
    return get_disk_identity_macos();

#else
    return std::nullopt;

#endif
}

inline std::string get_disk_sn() noexcept
{
    try
    {
        auto identity = get_disk_identity();
        if(!identity || !identity->stable)
            return {};

        return identity->id;
    }
    catch(...)
    {
        return {};
    }
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
    issuer(const issuer &other) = delete;
    issuer(issuer &&)           = delete;

    virtual ~issuer() = default;

    issuer &operator=(const issuer &other) = delete;
    issuer &operator=(issuer &&)           = delete;

    inline const std::string &id() const { return _id; }
    inline sign_algo          algo() const { return _algo; }
    inline std::size_t valid_times() const { return _valid_times.load(); }
    inline void        set_keys(std::vector<std::string> &&keys) noexcept
    {
        std::lock_guard<std::mutex> lock(_mu);
        _keys = std::move(keys);
    }

    err_t issue(token_t                   &token,
                const std::string         &licensee,
                const std::size_t          valid_days,
                const std::vector<pair_t> &claims = {})
    {
        if(valid_days > (std::numeric_limits<std::size_t>::max)() / 86400)
            return detail::make_err(error_code::param_too_big);

        std::size_t old_count = _valid_times.load();
        do
        {
            if(old_count == 0)
                return detail::make_err(error_code::invalid_times);
        } while(!_valid_times.compare_exchange_weak(old_count, old_count - 1));

        std::lock_guard<std::mutex> lock(_mu);
        return detail::issue(token,
                             _id,
                             licensee,
                             jwt::date::clock::now(),
                             valid_days * 24 * 60 * 60,
                             _algo,
                             _keys,
                             claims);
    }

    err_t issue(std::ostream              &out,
                const std::string         &licensee,
                const std::size_t          valid_days,
                const std::vector<pair_t> &claims = {})
    {
        if(!out || !out.good())
            return detail::make_err(error_code::invalid_output_stream);

        token_t token;
        auto    error_code = issue(token, licensee, valid_days, claims);
        if(error_code)
            return error_code;

        out << token;
        if(!out.good())
            return detail::make_err(error_code::invalid_output_stream);
        return err_t{};
    }

    err_t issue_file(const std::string         &filepath,
                     const std::string         &licensee,
                     const std::size_t          valid_days,
                     const std::vector<pair_t> &claims = {})
    {
        std::ofstream out(filepath, std::ios::binary);
        if(!out || !out.is_open())
            return detail::make_err(error_code::file_not_exist);

        return issue(out, licensee, valid_days, claims);
    }

    // release all issued licences by reset encrypt keys
    err_t release(const sign_algo algo, const std::vector<std::string> &keys)
    {
        std::lock_guard<std::mutex> lock(_mu);
        if(algo == _algo && keys == _keys)
            return detail::make_err(error_code::keys_not_changed);

        _algo = algo;
        _keys = keys;
        return err_t{};
    }

  private:
    mutable std::mutex       _mu;
    std::string              _id;
    sign_algo                _algo;
    std::vector<std::string> _keys;
    std::atomic<std::size_t> _valid_times;
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
    verifier(const verifier &other) = delete;
    verifier(verifier &&)           = delete;

    virtual ~verifier() = default;

    verifier &operator=(const verifier &other) = delete;
    verifier &operator=(verifier &&)           = delete;

    inline const std::string &id() const { return _id; }
    inline sign_algo          algo() const
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _algo;
    }
    inline void set_keys(std::vector<std::string> &&keys)
    {
        std::lock_guard<std::mutex> lock(_mu);
        _keys = std::move(keys);
    }

    err_t verify(const token_t             &token,
                 const std::string         &licensee,
                 const std::vector<pair_t> &claims      = {},
                 std::size_t                leeway_secs = 5)
    {
        std::lock_guard<std::mutex> lock(_mu);
        return detail::verify(token,
                              _id,
                              licensee,
                              _algo,
                              _keys,
                              claims,
                              leeway_secs);
    }

    err_t verify(std::istream              &in,
                 const std::string         &licensee,
                 const std::vector<pair_t> &claims      = {},
                 std::size_t                leeway_secs = 5)
    {
        if(!in)
            return detail::make_err(error_code::invalid_input_stream);

        std::string line, buf;
        while(std::getline(in, line))
            buf += line;

        if(buf.empty())
            return detail::make_err(error_code::invalid_input_stream);

        token_t                     token = buf;
        std::lock_guard<std::mutex> lock(_mu);
        return detail::verify(token,
                              _id,
                              licensee,
                              _algo,
                              _keys,
                              claims,
                              leeway_secs);
    }

    err_t verify_file(const std::string         &filepath,
                      const std::string         &licensee,
                      const std::vector<pair_t> &claims      = {},
                      std::size_t                leeway_secs = 5)
    {
        if(!is_file_exist(filepath))
            return detail::make_err(error_code::file_not_exist);

        std::ifstream in(filepath, std::ios::binary);
        if(!in || !in.is_open())
            return detail::make_err(error_code::file_open_failed);

        return verify(in, licensee, claims, leeway_secs);
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

                // avoid "\"xxx\"" format, use get<std::string>() to get the value directly
                info.claims.emplace_back(key,
                                         pair.second.is_string()
                                             ? pair.second.get<std::string>()
                                             : pair.second.dump());
            }
            return detail::make_err(error_code::ok);
        }
        catch(...)
        {
            return detail::make_err(error_code::parse_license_info_failed);
        }
    }

  private:
    mutable std::mutex       _mu;
    std::string              _id;
    sign_algo                _algo;
    std::vector<std::string> _keys;
};

} // namespace license
} // namespace hj

#endif // LICENSE_HPP