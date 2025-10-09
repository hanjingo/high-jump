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

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace hj
{

class uuid
{
  public:
    using uuid_type = boost::uuids::uuid;

  public:
    uuid()                        = default;
    ~uuid()                       = default;
    uuid(const uuid &)            = delete;
    uuid &operator=(const uuid &) = delete;
    uuid(uuid &&)                 = delete;
    uuid &operator=(uuid &&)      = delete;

    template <typename Generator = boost::uuids::random_generator>
    static uuid_type generator() noexcept
    {
        thread_local static Generator generator;
        return generator();
    }

    template <typename Generator = boost::uuids::random_generator>
    static std::string gen() noexcept
    {
        return boost::uuids::to_string(generator());
    }

    template <typename Generator = boost::uuids::random_generator>
    static unsigned long long gen_u64(bool big_endian = true) noexcept
    {
        auto               rand  = generator();
        auto              &bytes = rand.data;
        unsigned long long ull   = 0;
        if(big_endian)
        {
            ull = bytes[7] & 0xFF;
            ull |= ((bytes[6] << 8) & 0xFF00);
            ull |= ((bytes[5] << 16) & 0xFF0000);
            ull |= ((bytes[4] << 24) & 0xFF000000);
            ull |= ((((long long) bytes[3]) << 32) & 0xFF00000000);
            ull |= ((((long long) bytes[2]) << 40) & 0xFF0000000000);
            ull |= ((((long long) bytes[1]) << 48) & 0xFF000000000000);
            ull |= ((((long long) bytes[0]) << 56) & 0xFF00000000000000);
        } else
        {
            ull = bytes[0] & 0xFF;
            ull |= ((bytes[1] << 8) & 0xFF00);
            ull |= ((bytes[2] << 16) & 0xFF0000);
            ull |= ((bytes[3] << 24) & 0xFF000000);
            ull |= ((((long long) bytes[4]) << 32) & 0xFF00000000);
            ull |= ((((long long) bytes[5]) << 40) & 0xFF0000000000);
            ull |= ((((long long) bytes[6]) << 48) & 0xFF000000000000);
            ull |= ((((long long) bytes[7]) << 56) & 0xFF00000000000000);
        }
        return ull;
    }

    static bool equal(const uuid_type &a, const uuid_type &b) noexcept
    {
        return a == b;
    }
};

}

#endif