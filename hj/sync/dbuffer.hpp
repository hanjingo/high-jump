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

#ifndef HJ_DBUFFER_HPP
#define HJ_DBUFFER_HPP

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <mutex>
#include <type_traits>
#include <utility>
#include <vector>

namespace hj
{

struct default_copy_policy
{
    template <typename Container>
    bool operator()(const Container &src, Container &dst) const noexcept
    {
        try
        {
            dst = src;
            return true;
        }
        catch(...)
        {
            return false;
        }
    }

    template <typename Container>
    bool move(Container &&src, Container &dst) const noexcept
    {
        try
        {
            dst = std::move(src);
            return true;
        }
        catch(...)
        {
            return false;
        }
    }
};

template <typename Container, typename CopyPolicy = default_copy_policy>
class dbuffer
{
  private:
    struct snapshot_node
    {
        Container value;
    };

    struct retired_snapshot
    {
        snapshot_node *ptr;
        std::uint64_t  generation;
    };

    template <typename Policy, typename = void>
    struct has_custom_move : std::false_type
    {
    };

    template <typename Policy>
    struct has_custom_move<
        Policy,
        std::void_t<decltype(std::declval<Policy &>().move(
            std::declval<Container &&>(), std::declval<Container &>()))>>
        : std::true_type
    {
    };

  public:
    class read_guard
    {
      public:
        read_guard() noexcept = default;

        read_guard(const read_guard &)            = delete;
        read_guard &operator=(const read_guard &) = delete;

        read_guard(read_guard &&other) noexcept
            : _owner(other._owner)
            , _snapshot(other._snapshot)
        {
            other._owner    = nullptr;
            other._snapshot = nullptr;
        }

        read_guard &operator=(read_guard &&other) noexcept
        {
            if(this == &other)
                return *this;

            release();

            _owner    = other._owner;
            _snapshot = other._snapshot;

            other._owner    = nullptr;
            other._snapshot = nullptr;

            return *this;
        }

        ~read_guard() { release(); }

        const Container &get() const noexcept { return _snapshot->value; }

        const Container *operator->() const noexcept
        {
            return &_snapshot->value;
        }

        const Container &operator*() const noexcept { return _snapshot->value; }

        explicit operator bool() const noexcept { return _snapshot != nullptr; }

      private:
        friend class dbuffer;

        read_guard(const dbuffer *owner, snapshot_node *snapshot_ptr) noexcept
            : _owner(owner)
            , _snapshot(snapshot_ptr)
        {
        }

        void release() noexcept
        {
            if(_owner == nullptr)
                return;

            _owner->_active_readers.fetch_sub(1, std::memory_order_seq_cst);

            _owner    = nullptr;
            _snapshot = nullptr;
        }

      private:
        const dbuffer *_owner    = nullptr;
        snapshot_node *_snapshot = nullptr;
    };

  public:
    explicit dbuffer(CopyPolicy copy_policy = CopyPolicy{})
        : _copy_policy(std::move(copy_policy))
    {
        auto *initial = new snapshot_node{};

        _current.store(initial, std::memory_order_seq_cst);
    }

    ~dbuffer()
    {
        if(_active_readers.load(std::memory_order_seq_cst) != 0)
            std::terminate();

        snapshot_node *current = _current.load(std::memory_order_seq_cst);

        delete current;

        for(auto &entry : _retired)
            delete entry.ptr;
    }

    dbuffer(const dbuffer &)            = delete;
    dbuffer &operator=(const dbuffer &) = delete;

    dbuffer(dbuffer &&)            = delete;
    dbuffer &operator=(dbuffer &&) = delete;

    bool write(const Container &value)
    {
        std::lock_guard<std::mutex> lock(_write_mutex);

        snapshot_node *next = make_copy(value);

        if(next == nullptr)
            return false;

        publish(next);

        return true;
    }

    bool write(Container &&value)
    {
        std::lock_guard<std::mutex> lock(_write_mutex);

        snapshot_node *next = make_move(std::move(value));

        if(next == nullptr)
            return false;

        publish(next);

        return true;
    }

    read_guard read_guarded() const noexcept
    {
        _active_readers.fetch_add(1, std::memory_order_seq_cst);
        snapshot_node *current = _current.load(std::memory_order_seq_cst);
        return read_guard(this, current);
    }

    bool read(Container &value) const
    {
        auto guard = read_guarded();
        return _copy_policy(guard.get(), value);
    }

    bool snapshot(Container &value) const { return read(value); }

    std::size_t retired_count() const
    {
        std::lock_guard<std::mutex> lock(_write_mutex);
        return _retired.size();
    }

    std::uint64_t active_readers() const noexcept
    {
        return _active_readers.load(std::memory_order_seq_cst);
    }

    std::uint64_t generation() const noexcept
    {
        return _generation.load(std::memory_order_seq_cst);
    }

    void reclaim() const
    {
        std::lock_guard<std::mutex> lock(_write_mutex);
        reclaim_locked();
    }

  private:
    snapshot_node *make_copy(const Container &value)
    {
        try
        {
            auto *next = new snapshot_node{};

            if(!_copy_policy(value, next->value))
            {
                delete next;
                return nullptr;
            }

            return next;
        }
        catch(...)
        {
            return nullptr;
        }
    }

    snapshot_node *make_move(Container &&value)
    {
        try
        {
            auto *next = new snapshot_node{};

            if constexpr(has_custom_move<CopyPolicy>::value)
            {
                if(!_copy_policy.move(std::move(value), next->value))
                {
                    delete next;
                    return nullptr;
                }
            } else
            {
                next->value = std::move(value);
            }

            return next;
        }
        catch(...)
        {
            return nullptr;
        }
    }

    void publish(snapshot_node *next)
    {
        const std::uint64_t retired_generation =
            _generation.fetch_add(1, std::memory_order_seq_cst);
        snapshot_node *old = _current.exchange(next, std::memory_order_seq_cst);
        _retired.push_back(retired_snapshot{old, retired_generation});
        reclaim_locked();
    }

    void reclaim_locked() const
    {
        if(_active_readers.load(std::memory_order_seq_cst) != 0)
        {
            return;
        }

        for(auto &entry : _retired)
            delete entry.ptr;

        _retired.clear();
    }

  private:
    mutable std::atomic<snapshot_node *>  _current{nullptr};
    mutable std::atomic<std::uint64_t>    _active_readers{0};
    mutable std::atomic<std::uint64_t>    _generation{1};
    mutable std::mutex                    _write_mutex;
    mutable std::vector<retired_snapshot> _retired;
    CopyPolicy                            _copy_policy;
};

} // namespace hj

#endif // HJ_DBUFFER_HPP
