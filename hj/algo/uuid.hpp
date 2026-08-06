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

#ifndef UUID_HPP
#define UUID_HPP

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <string>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <type_traits>

namespace hj
{

namespace detail
{
// tool function to swap endianness of a 64-bit integer
inline uint64_t swap64(uint64_t val)
{
#if defined(_MSC_VER)
    return _byteswap_uint64(val);
#elif defined(__GNUG__) || defined(__clang__)
    return __builtin_bswap64(val);
#else
    return (val >> 56) | ((val << 40) & 0x00FF000000000000ULL)
           | ((val << 24) & 0x0000FF0000000000ULL)
           | ((val << 8) & 0x000000FF00000000ULL)
           | ((val >> 8) & 0x00000000FF000000ULL)
           | ((val >> 24) & 0x0000000000FF0000ULL)
           | ((val >> 40) & 0x000000000000FF00ULL) | (val << 56);
#endif
}

inline uint64_t to_endian(uint64_t val, bool big_endian)
{
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    if(!big_endian)
        return swap64(val);
#else
    if(big_endian)
        return swap64(val);
#endif
    return val;
}

inline std::string rfc_uuid_format(uint64_t high, uint64_t low)
{
    uint64_t rfc_high = (high & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000008000ULL;
    uint64_t rfc_low  = (low & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

    char buf[37];
    std::snprintf(buf,
                  sizeof(buf),
                  "%08x-%04x-%04x-%04x-%012llx",
                  static_cast<uint32_t>(rfc_high >> 32),
                  static_cast<uint16_t>((rfc_high >> 16) & 0xFFFF),
                  static_cast<uint16_t>(rfc_high & 0xFFFF),
                  static_cast<uint16_t>((rfc_low >> 48) & 0xFFFF),
                  static_cast<unsigned long long>(rfc_low & 0xFFFFFFFFFFFFULL));
    return std::string(buf, 36);
}

// SFINAE traits to check if a Generator has a static gen_u64(bool) method
template <typename T, typename = void>
struct has_gen_u64_with_param : std::false_type
{
};
template <typename T>
struct has_gen_u64_with_param<T, std::void_t<decltype(T::gen_u64(true))>>
    : std::true_type
{
};
template <typename T, typename = void>
struct has_gen : std::false_type
{
};
template <typename T>
struct has_gen<T, std::void_t<decltype(T::gen())>> : std::true_type
{
};

// Generator_impl struct to call the appropriate gen_u64 method
template <typename Generator, typename Enable = void>
struct generator_impl
{
    static uint64_t gen_u64(bool big_endian = true)
    {
        if constexpr(has_gen_u64_with_param<Generator>::value)
        {
            return Generator::gen_u64(big_endian);
        } else
        {
            uint64_t id = Generator::gen_u64();
            return to_endian(id, big_endian);
        }
    }

    static std::string gen()
    {
        if constexpr(has_gen<Generator>::value)
        {
            return Generator::gen();
        } else
        {
            uint64_t                     id   = gen_u64(false);
            thread_local static uint64_t seed = 0x9e3779b97f4a7c15ULL;
            seed ^= (seed << 13);
            seed ^= (seed >> 7);
            seed ^= (seed << 17);
            return rfc_uuid_format(id, seed);
        }
    }
};

// snowflake engine implementation
class snowflake_engine
{
  public:
    static constexpr uint64_t _worker_idbits  = 10ULL;
    static constexpr uint64_t _seqbits        = 12ULL;
    static constexpr uint64_t max_worker_id   = (1ULL << _worker_idbits) - 1ULL;
    static constexpr uint64_t _seqmask        = (1ULL << _seqbits) - 1ULL;
    static constexpr uint64_t _worker_idshift = _seqbits;
    static constexpr uint64_t timestamp_shift = _seqbits + _worker_idbits;
    static constexpr uint64_t epoch =
        1735689600000ULL; // 2025-01-01 00:00:00 UTC
    static constexpr uint64_t max_backward_ms = 5000ULL;

    explicit snowflake_engine(uint64_t worker_id = 1)
        : _worker_id(worker_id)
    {
        if(_worker_id > max_worker_id)
            throw std::invalid_argument("Worker ID exceeds limit (1023)");
    }

    inline void reset_worker_id(uint64_t worker_id)
    {
        std::lock_guard<std::mutex> lock(_mu);
        if(worker_id > max_worker_id)
            throw std::invalid_argument("Worker ID exceeds limit (1023)");

        _worker_id = worker_id;
    }

    uint64_t next_id()
    {
        std::unique_lock<std::mutex> lock(_mu);
        uint64_t                     timestamp = _current_time_ms();
        if(timestamp < _last_timestamp)
        {
            uint64_t offset = _last_timestamp - timestamp;
            if(offset > max_backward_ms)
                throw std::runtime_error("Clock moved backwards too much!");

            // avoid busy waiting by releasing the lock and sleeping for a short duration
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(offset));
            lock.lock();

            timestamp = _current_time_ms();
            if(timestamp < _last_timestamp)
                timestamp = _wait_next_millis(_last_timestamp);
        }

        if(timestamp == _last_timestamp)
        {
            _seq = (_seq + 1) & _seqmask;
            if(_seq == 0)
                timestamp = _wait_next_millis(_last_timestamp);
        } else
        {
            _seq = 0;
        }
        _last_timestamp = timestamp;
        return ((timestamp - epoch) << timestamp_shift)
               | (_worker_id << _worker_idshift) | _seq;
    }

  private:
    static uint64_t _current_time_ms()
    {
        using namespace std::chrono;
        return duration_cast<milliseconds>(
                   system_clock::now().time_since_epoch())
            .count();
    }

    static uint64_t _wait_next_millis(uint64_t last_time)
    {
        uint64_t timestamp = _current_time_ms();
        while(timestamp <= last_time)
        {
            // avoid busy waiting by yielding the thread and updating the timestamp
            std::this_thread::yield();
            timestamp = _current_time_ms();
        }

        return timestamp;
    }

  private:
    std::mutex _mu;
    uint64_t   _worker_id{1};
    uint64_t   _seq{0};
    uint64_t   _last_timestamp{0};
};

struct snowflake
{
    static void init(uint64_t worker_id) { _instance(worker_id, true); }

    static uint64_t gen_u64(bool big_endian = true)
    {
        uint64_t id = _instance().next_id();
        return detail::to_endian(id, big_endian);
    }

    static std::string gen()
    {
        uint64_t                     id   = gen_u64(false);
        thread_local static uint64_t seed = static_cast<uint64_t>(
            std::chrono::steady_clock::now().time_since_epoch().count());
        seed ^= (seed << 13);
        seed ^= (seed >> 7);
        seed ^= (seed << 17);
        return detail::rfc_uuid_format(id, seed);
    }

  private:
    static detail::snowflake_engine &_instance(uint64_t worker_id = 1,
                                               bool     reinit    = false)
    {
        static detail::snowflake_engine inst{worker_id};
        if(reinit)
            inst.reset_worker_id(worker_id);

        return inst;
    }
};

} // namespace detail

class uuid
{
  public:
    uuid()                        = delete;
    ~uuid()                       = delete;
    uuid(const uuid &)            = delete;
    uuid &operator=(const uuid &) = delete;
    uuid(uuid &&)                 = delete;
    uuid &operator=(uuid &&)      = delete;

    template <typename Generator = hj::detail::snowflake>
    static uint64_t gen_u64(bool big_endian = true)
    {
        return detail::generator_impl<Generator>::gen_u64(big_endian);
    }

    // Generate RFC 4122 compliant UUID string using the specified generator
    template <typename Generator = hj::detail::snowflake>
    static std::string gen()
    {
        return detail::generator_impl<Generator>::gen();
    }
};

} // namespace hj

#endif