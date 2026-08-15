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

#ifndef SKIPLIST_HPP
#define SKIPLIST_HPP

#include <cassert>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>
#include <unordered_map>

namespace hj
{

template <typename T,
          typename Score        = double,
          typename Compare      = std::less<T>,
          typename ScoreCompare = std::less<Score>,
          typename Allocator    = std::allocator<T>>
class skiplist
{
  private:
    struct node_t;

    struct level_t
    {
        node_t     *forward{nullptr};
        std::size_t span{0};
    };

    struct node_t
    {
        alignas(alignof(T)) std::byte obj_buf[sizeof(T)];
        alignas(alignof(Score)) std::byte score_buf[sizeof(Score)];
        node_t *backward{nullptr};
        int     level_count{0};

        level_t *levels() noexcept
        {
            return reinterpret_cast<level_t *>(this + 1);
        }

        const level_t *levels() const noexcept
        {
            return reinterpret_cast<const level_t *>(this + 1);
        }

        T       *value_ptr() noexcept { return reinterpret_cast<T *>(obj_buf); }
        const T *value_ptr() const noexcept
        {
            return reinterpret_cast<const T *>(obj_buf);
        }
        T       &value() noexcept { return *value_ptr(); }
        const T &value() const noexcept { return *value_ptr(); }

        Score *score_ptr() noexcept
        {
            return reinterpret_cast<Score *>(score_buf);
        }
        const Score *score_ptr() const noexcept
        {
            return reinterpret_cast<const Score *>(score_buf);
        }
        Score       &score() noexcept { return *score_ptr(); }
        const Score &score() const noexcept { return *score_ptr(); }
    };

    static_assert(alignof(node_t) >= alignof(level_t),
                  "node_t must be at least as aligned as level_t for trailing "
                  "array layout.");

  public:
    static constexpr int         kMaxLevel    = 32;
    static constexpr double      kProbability = 0.25;
    static constexpr std::size_t npos         = static_cast<std::size_t>(-1);

    class const_iterator
    {
      public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = const T *;
        using reference         = const T &;

      public:
        const_iterator()
            : current(nullptr)
            , list_ptr(nullptr)
        {
        }

        reference operator*() const { return current->value(); }
        pointer   operator->() const { return current->value_ptr(); }

        const_iterator &operator++()
        {
            if(current)
                current = current->levels()[0].forward;
            return *this;
        }

        const_iterator operator++(int)
        {
            const_iterator temp = *this;
            ++(*this);
            return temp;
        }

        const_iterator &operator--()
        {
            if(current)
            {
                current = current->backward;
            } else if(list_ptr)
            {
                current = list_ptr->_tail;
            }
            return *this;
        }

        const_iterator operator--(int)
        {
            const_iterator temp = *this;
            --(*this);
            return temp;
        }

        bool operator==(const const_iterator &other) const
        {
            return current == other.current;
        }

        bool operator!=(const const_iterator &other) const
        {
            return current != other.current;
        }

        const Score &score() const
        {
            assert(current != nullptr
                   && "Cannot call score() on end() iterator");
            return current->score();
        }

      private:
        friend class skiplist;

        explicit const_iterator(const node_t   *node,
                                const skiplist *sl = nullptr)
            : current(node)
            , list_ptr(sl)
        {
        }

        const node_t *node() const { return current; }

      private:
        const node_t   *current{nullptr};
        const skiplist *list_ptr{nullptr};
    };

    using iterator = const_iterator;

  public:
    explicit skiplist(const Compare      &cmp       = Compare(),
                      const ScoreCompare &score_cmp = ScoreCompare(),
                      const Allocator    &alloc     = Allocator())
        : _header(nullptr)
        , _tail(nullptr)
        , _length(0)
        , _level(1)
        , _compare(cmp)
        , _score_compare(score_cmp)
        , _alloc(alloc)
    {
        _init_empty();
    }

    ~skiplist() { _clear_and_release_header(); }

    skiplist(const skiplist &other)
        : _header(nullptr)
        , _tail(nullptr)
        , _length(0)
        , _level(1)
        , _compare(other._compare)
        , _score_compare(other._score_compare)
        , _alloc(std::allocator_traits<Allocator>::
                     select_on_container_copy_construction(other._alloc))
    {
        try
        {
            _init_empty();
            _copy_from(other);
        }
        catch(...)
        {
            _clear_and_release_header();
            throw;
        }
    }

    skiplist &operator=(const skiplist &other)
    {
        if(this != &other)
        {
            skiplist tmp(other);
            _swap(tmp);
        }
        return *this;
    }

    skiplist(skiplist &&other) noexcept
        : _header(other._header)
        , _tail(other._tail)
        , _length(other._length)
        , _level(other._level)
        , _compare(std::move(other._compare))
        , _score_compare(std::move(other._score_compare))
        , _alloc(std::move(other._alloc))
    {
        other._header = nullptr;
        other._tail   = nullptr;
        other._length = 0;
        other._level  = 1;
    }

    skiplist &operator=(skiplist &&other) noexcept
    {
        if(this != &other)
        {
            _clear_and_release_header();
            _header        = other._header;
            _tail          = other._tail;
            _length        = other._length;
            _level         = other._level;
            _compare       = std::move(other._compare);
            _score_compare = std::move(other._score_compare);
            _alloc         = std::move(other._alloc);

            other._header = nullptr;
            other._tail   = nullptr;
            other._length = 0;
            other._level  = 1;
        }
        return *this;
    }

    std::size_t size() const noexcept { return _length; }
    bool        empty() const noexcept { return _length == 0; }

    iterator first() const noexcept
    {
        return iterator(_header ? _header->levels()[0].forward : nullptr, this);
    }
    iterator last() const noexcept { return iterator(_tail, this); }

    const_iterator begin() const noexcept
    {
        return const_iterator(_header ? _header->levels()[0].forward : nullptr,
                              this);
    }
    const_iterator end() const noexcept
    {
        return const_iterator(nullptr, this);
    }

    const_iterator cbegin() const noexcept
    {
        return const_iterator(_header ? _header->levels()[0].forward : nullptr,
                              this);
    }
    const_iterator cend() const noexcept
    {
        return const_iterator(nullptr, this);
    }

    const Compare      &key_comp() const noexcept { return _compare; }
    const ScoreCompare &score_comp() const noexcept { return _score_compare; }

    template <typename... Args>
    iterator emplace(Score score, Args &&...args)
    {
        if(!_header)
            _init_empty();

        node_t     *update[kMaxLevel];
        std::size_t rank[kMaxLevel];
        int         new_level = _random_level();

        node_t *new_node =
            _create_node(new_level, score, std::forward<Args>(args)...);
        bool dismissed = false;
        auto guard     = [this, new_node, &dismissed]() {
            if(!dismissed)
            {
                this->_free_node(new_node, false);
            }
        };

        struct ScopeExit
        {
            decltype(guard) &f;
            ~ScopeExit() { f(); }
        } cleanup{guard};

        _find_predecessors(score, new_node->value(), update, rank);

        if(new_level > _level)
        {
            for(int i = _level; i < new_level; i++)
            {
                rank[i]                     = 0;
                update[i]                   = _header;
                update[i]->levels()[i].span = _length;
            }
            _level = new_level;
        }

        node_t *x = new_node;
        for(int i = 0; i < new_level; i++)
        {
            x->levels()[i].forward         = update[i]->levels()[i].forward;
            update[i]->levels()[i].forward = x;

            x->levels()[i].span =
                update[i]->levels()[i].span - (rank[0] - rank[i]);
            update[i]->levels()[i].span = (rank[0] - rank[i]) + 1;
        }

        for(int i = new_level; i < _level; i++)
        {
            update[i]->levels()[i].span++;
        }

        x->backward = (update[0] == _header) ? nullptr : update[0];
        if(x->levels()[0].forward)
            x->levels()[0].forward->backward = x;
        else
            _tail = x;

        _length++;
        dismissed = true;
        return iterator(x, this);
    }

    iterator insert(Score score, const T &obj) { return emplace(score, obj); }

    iterator insert(Score score, T &&obj)
    {
        return emplace(score, std::move(obj));
    }

    bool erase(Score score, const T &obj)
    {
        node_t *update[kMaxLevel];
        _find_predecessors(score, obj, update, nullptr);

        node_t *x = update[0]->levels()[0].forward;
        if(x && _score_eq(x->score(), score) && !_compare(x->value(), obj)
           && !_compare(obj, x->value()))
        {
            _erase_internal(x, update);
            return true;
        }

        return false;
    }

    bool contains(Score score, const T &obj) const
    {
        return _find_node(score, obj) != nullptr;
    }

    iterator find(Score score, const T &obj) const
    {
        return iterator(_find_node(score, obj), this);
    }

    iterator get_element_by_rank(std::size_t rank) const
    {
        return iterator(_element_by_rank(rank), this);
    }

    std::size_t get_rank(Score score, const T &obj) const
    {
        if(!_header)
            return npos;

        node_t     *x    = _header;
        std::size_t rank = 0;
        for(int i = _level - 1; i >= 0; i--)
        {
            while(x->levels()[i].forward
                  && _precedes(x->levels()[i].forward, score, obj))
            {
                rank += x->levels()[i].span;
                x = x->levels()[i].forward;
            }

            if(x->levels()[i].forward
               && _score_eq(x->levels()[i].forward->score(), score)
               && !_compare(x->levels()[i].forward->value(), obj)
               && !_compare(obj, x->levels()[i].forward->value()))
            {
                rank += x->levels()[i].span;
                return rank - 1;
            }
        }

        return npos;
    }

    std::size_t get_node_rank(const_iterator target_it) const
    {
        const node_t *target = target_it.node();
        if(!target)
            return npos;
        node_t     *x    = _header;
        std::size_t rank = 0;
        for(int i = _level - 1; i >= 0; i--)
        {
            while(x->levels()[i].forward
                  && _precedes(x->levels()[i].forward,
                               target->score(),
                               target->value()))
            {
                rank += x->levels()[i].span;
                x = x->levels()[i].forward;
            }

            if(x->levels()[i].forward == target)
            {
                rank += x->levels()[i].span;
                return rank - 1;
            }
        }
        return npos;
    }

    std::pair<iterator, std::size_t>
    first_in_range_with_rank(Score min_score, Score max_score) const
    {
        if(!_is_in_range(min_score, max_score))
            return {end(), npos};

        node_t     *x    = _header;
        std::size_t rank = 0;
        for(int i = _level - 1; i >= 0; i--)
        {
            while(x->levels()[i].forward
                  && _score_less(x->levels()[i].forward->score(), min_score))
            {
                rank += x->levels()[i].span;
                x = x->levels()[i].forward;
            }
        }

        x = x->levels()[0].forward;
        if(!x || _score_less(max_score, x->score()))
            return {end(), npos};

        return {iterator(x, this), rank};
    }

    std::pair<iterator, std::size_t>
    last_in_range_with_rank(Score min_score, Score max_score) const
    {
        if(!_is_in_range(min_score, max_score))
            return {end(), npos};

        node_t     *x    = _header;
        std::size_t rank = 0;
        for(int i = _level - 1; i >= 0; i--)
        {
            while(x->levels()[i].forward
                  && _score_le(x->levels()[i].forward->score(), max_score))
            {
                rank += x->levels()[i].span;
                x = x->levels()[i].forward;
            }
        }

        if(!x || x == _header || _score_less(x->score(), min_score))
            return {end(), npos};

        return {iterator(x, this), rank - 1};
    }

    std::size_t get_rank(const_iterator target_it) const
    {
        return get_node_rank(target_it);
    }

    iterator first_in_range(Score min_score, Score max_score) const
    {
        return iterator(_first_in_range(min_score, max_score), this);
    }

    iterator last_in_range(Score min_score, Score max_score) const
    {
        return iterator(_last_in_range(min_score, max_score), this);
    }

    std::size_t delete_range_by_score(Score min_score, Score max_score)
    {
        if(_score_less(max_score, min_score))
            return 0;

        node_t     *update[kMaxLevel];
        node_t     *x       = _header;
        std::size_t removed = 0;
        for(int i = _level - 1; i >= 0; i--)
        {
            while(x->levels()[i].forward
                  && _score_less(x->levels()[i].forward->score(), min_score))
                x = x->levels()[i].forward;

            update[i] = x;
        }

        x = x->levels()[0].forward;
        while(x && _score_le(x->score(), max_score))
        {
            node_t *next = x->levels()[0].forward;
            _erase_internal(x, update);
            removed++;
            x = next;
        }

        return removed;
    }

    std::size_t delete_range_by_rank(std::size_t start, std::size_t end)
    {
        if(start > end || start >= _length)
            return 0;

        if(end >= _length)
            end = _length - 1;

        node_t     *update[kMaxLevel];
        node_t     *x         = _header;
        std::size_t traversed = 0;
        std::size_t removed   = 0;
        for(int i = _level - 1; i >= 0; i--)
        {
            while(x->levels()[i].forward
                  && traversed + x->levels()[i].span <= start)
            {
                traversed += x->levels()[i].span;
                x = x->levels()[i].forward;
            }
            update[i] = x;
        }

        x                        = x->levels()[0].forward;
        std::size_t current_rank = start;
        while(x && current_rank <= end)
        {
            node_t *next = x->levels()[0].forward;
            _erase_internal(x, update);
            removed++;
            current_rank++;
            x = next;
        }

        return removed;
    }

    bool validate() const
    {
        if(!_header)
            return true;
        if(_level < 1 || _level > kMaxLevel)
            return false;
        if(_length == 0)
            return _validate_empty();

        std::unordered_map<const node_t *, std::size_t> node_ranks;

        if(!_validate_level0_and_build_ranks(node_ranks))
            return false;
        if(!_validate_active_levels(node_ranks))
            return false;
        if(!_validate_unused_levels())
            return false;

        return true;
    }

    void clear()
    {
        _clear_and_release_header();
        _init_empty();
    }

    void swap(skiplist &other) noexcept { _swap(other); }

  private:
    using NodeAlloc = typename std::allocator_traits<
        Allocator>::template rebind_alloc<node_t>;

    static constexpr std::size_t _calc_node_count(int level) noexcept
    {
        std::size_t total_bytes = sizeof(node_t) + level * sizeof(level_t);
        return (total_bytes + sizeof(node_t) - 1) / sizeof(node_t);
    }

    bool _score_less(const Score &a, const Score &b) const
    {
        return _score_compare(a, b);
    }

    bool _score_eq(const Score &a, const Score &b) const
    {
        return !_score_compare(a, b) && !_score_compare(b, a);
    }

    bool _score_le(const Score &a, const Score &b) const
    {
        return !_score_compare(b, a);
    }

    void _init_empty()
    {
        _header = _create_header_node(kMaxLevel);
        _tail   = nullptr;
        _length = 0;
        _level  = 1;
    }

    node_t *_create_header_node(int level)
    {
        std::size_t count = _calc_node_count(level);
        NodeAlloc   node_alloc(_alloc);
        node_t     *raw =
            std::allocator_traits<NodeAlloc>::allocate(node_alloc, count);

        node_t *node      = ::new(static_cast<void *>(raw)) node_t;
        node->backward    = nullptr;
        node->level_count = level;

        for(int i = 0; i < level; ++i)
        {
            ::new(static_cast<void *>(node->levels() + i)) level_t();
        }
        return node;
    }

    void _clear_and_release_header()
    {
        if(!_header)
            return;
        node_t *forward = _header->levels()[0].forward;
        while(forward)
        {
            node_t *next = forward->levels()[0].forward;
            _free_node(forward, false);
            forward = next;
        }
        _free_node(_header, true);
        _header = nullptr;
        _tail   = nullptr;
        _length = 0;
        _level  = 1;
    }

    void _copy_from(const skiplist &other)
    {
        for(auto it = other.begin(); it != other.end(); ++it)
            emplace(it.score(), *it);
    }

    void _swap(skiplist &other) noexcept
    {
        using std::swap;
        swap(_header, other._header);
        swap(_tail, other._tail);
        swap(_length, other._length);
        swap(_level, other._level);
        swap(_compare, other._compare);
        swap(_score_compare, other._score_compare);
        swap(_alloc, other._alloc);
    }

    bool _precedes(const node_t *node, Score score, const T &obj) const
    {
        if(_score_less(node->score(), score))
            return true;
        if(_score_less(score, node->score()))
            return false;
        return _compare(node->value(), obj);
    }

    void _find_predecessors(Score        score,
                            const T     &obj,
                            node_t     **update,
                            std::size_t *rank) const
    {
        node_t *x = _header;
        for(int i = _level - 1; i >= 0; i--)
        {
            if(rank)
                rank[i] = (i == (_level - 1)) ? 0 : rank[i + 1];
            while(x->levels()[i].forward
                  && _precedes(x->levels()[i].forward, score, obj))
            {
                if(rank)
                    rank[i] += x->levels()[i].span;
                x = x->levels()[i].forward;
            }
            update[i] = x;
        }
    }

    const node_t *_find_node(Score score, const T &obj) const
    {
        node_t *update[kMaxLevel];
        _find_predecessors(score, obj, update, nullptr);
        node_t *x = update[0]->levels()[0].forward;
        if(x && _score_eq(x->score(), score) && !_compare(x->value(), obj)
           && !_compare(obj, x->value()))
            return x;
        return nullptr;
    }

    const node_t *_element_by_rank(std::size_t rank) const
    {
        if(rank >= _length)
            return nullptr;

        node_t     *x         = _header;
        std::size_t traversed = 0;

        for(int i = _level - 1; i >= 0; i--)
        {
            while(x->levels()[i].forward
                  && (traversed + x->levels()[i].span - 1) < rank)
            {
                traversed += x->levels()[i].span;
                x = x->levels()[i].forward;
            }
        }

        return x->levels()[0].forward;
    }

    template <bool IsHeader = false, typename ScoreArg, typename... Args>
    node_t *_create_node(int level, ScoreArg &&score_val, Args &&...args)
    {
        std::size_t count = _calc_node_count(level);
        NodeAlloc   node_alloc(_alloc);
        node_t     *raw =
            std::allocator_traits<NodeAlloc>::allocate(node_alloc, count);

        node_t *node      = ::new(static_cast<void *>(raw)) node_t;
        node->backward    = nullptr;
        node->level_count = level;

        for(int i = 0; i < level; ++i)
        {
            ::new(static_cast<void *>(node->levels() + i)) level_t();
        }

        ::new(static_cast<void *>(node->score_ptr()))
            Score(std::forward<ScoreArg>(score_val));

        try
        {
            std::allocator_traits<Allocator>::construct(
                _alloc,
                node->value_ptr(),
                std::forward<Args>(args)...);
        }
        catch(...)
        {
            node->score_ptr()->~Score();
            for(int i = 0; i < level; ++i)
            {
                node->levels()[i].~level_t();
            }
            node->~node_t();
            std::allocator_traits<NodeAlloc>::deallocate(node_alloc,
                                                         raw,
                                                         count);
            throw;
        }

        return node;
    }

    void _free_node(node_t *node, bool is_header)
    {
        if(!node)
            return;

        if(!is_header)
        {
            std::allocator_traits<Allocator>::destroy(_alloc,
                                                      node->value_ptr());
            node->score_ptr()->~Score();
        }
        for(int i = 0; i < node->level_count; ++i)
        {
            node->levels()[i].~level_t();
        }
        std::size_t count = _calc_node_count(node->level_count);
        node->~node_t();

        NodeAlloc node_alloc(_alloc);
        std::allocator_traits<NodeAlloc>::deallocate(node_alloc, node, count);
    }

    static int _random_level()
    {
        thread_local std::mt19937 gen(std::random_device{}());
        thread_local std::uniform_real_distribution<double> dis(0.0, 1.0);

        int level = 1;
        while(dis(gen) < kProbability && level < kMaxLevel)
            level++;

        return level;
    }

    void _erase_internal(node_t *x, node_t **update)
    {
        for(int i = 0; i < _level; i++)
        {
            if(update[i]->levels()[i].forward == x)
            {
                update[i]->levels()[i].span += x->levels()[i].span - 1;
                update[i]->levels()[i].forward = x->levels()[i].forward;
            } else
            {
                update[i]->levels()[i].span -= 1;
            }
        }

        if(x->levels()[0].forward)
            x->levels()[0].forward->backward = x->backward;
        else
            _tail = x->backward;

        _free_node(x, false);
        while(_level > 1 && _header->levels()[_level - 1].forward == nullptr)
        {
            _header->levels()[_level - 1].span = 0;
            _level--;
        }

        _length--;
    }

    bool _is_in_range(Score min_score, Score max_score) const
    {
        if(!_header || _score_less(max_score, min_score))
            return false;

        node_t *x = _tail;
        if(x == nullptr || _score_less(x->score(), min_score))
            return false;

        x = _header->levels()[0].forward;
        if(x == nullptr || _score_less(max_score, x->score()))
            return false;

        return true;
    }

    const node_t *_first_in_range(Score min_score, Score max_score) const
    {
        if(!_is_in_range(min_score, max_score))
            return nullptr;

        node_t *x = _header;
        for(int i = _level - 1; i >= 0; i--)
        {
            while(x->levels()[i].forward
                  && _score_less(x->levels()[i].forward->score(), min_score))
                x = x->levels()[i].forward;
        }

        x = x->levels()[0].forward;
        if(!x || _score_less(max_score, x->score()))
            return nullptr;

        return x;
    }

    const node_t *_last_in_range(Score min_score, Score max_score) const
    {
        if(!_is_in_range(min_score, max_score))
            return nullptr;

        node_t *x = _header;
        for(int i = _level - 1; i >= 0; i--)
        {
            while(x->levels()[i].forward
                  && _score_le(x->levels()[i].forward->score(), max_score))
                x = x->levels()[i].forward;
        }

        if(!x || x == _header || _score_less(x->score(), min_score))
            return nullptr;

        return x;
    }

    bool _validate_empty() const
    {
        if(_tail != nullptr)
            return false;
        for(int i = 0; i < kMaxLevel; ++i)
        {
            if(_header->levels()[i].forward != nullptr
               || _header->levels()[i].span != 0)
                return false;
        }
        return true;
    }

    bool _validate_level0_and_build_ranks(
        std::unordered_map<const node_t *, std::size_t> &node_ranks) const
    {
        if(_header->levels()[0].span != 1)
            return false;

        const node_t *curr  = _header->levels()[0].forward;
        const node_t *prev  = nullptr;
        std::size_t   count = 0;
        while(curr)
        {
            if(curr->level_count < 1 || curr->level_count > kMaxLevel)
                return false;

            if(curr->backward != prev)
                return false;

            if(prev)
            {
                if(_score_less(curr->score(), prev->score()))
                    return false;
                if(_score_eq(curr->score(), prev->score())
                   && _compare(curr->value(), prev->value()))
                    return false;
            }

            std::size_t expected_span =
                (curr->levels()[0].forward != nullptr) ? 1 : 0;
            if(curr->levels()[0].span != expected_span)
                return false;

            node_ranks[curr] = count++;
            prev             = curr;
            curr             = curr->levels()[0].forward;
        }

        return count == _length && prev == _tail;
    }

    bool _validate_active_levels(
        const std::unordered_map<const node_t *, std::size_t> &node_ranks) const
    {
        auto get_rank = [&](const node_t *node) -> std::ptrdiff_t {
            if(node == _header)
                return -1;

            auto it = node_ranks.find(node);
            return (it != node_ranks.end())
                       ? static_cast<std::ptrdiff_t>(it->second)
                       : -2;
        };
        for(int i = 0; i < _level; ++i)
        {
            const node_t *x          = _header;
            std::size_t   total_span = 0;
            while(x)
            {
                const node_t *next = x->levels()[i].forward;
                std::size_t   span = x->levels()[i].span;
                total_span += span;
                std::ptrdiff_t x_idx = get_rank(x);
                if(x_idx == -2)
                    return false;

                if(next)
                {
                    if(next->level_count <= i)
                        return false;

                    std::ptrdiff_t next_idx = get_rank(next);
                    if(next_idx == -2 || next_idx <= x_idx)
                        return false;

                    if(span != static_cast<std::size_t>(next_idx - x_idx))
                        return false;
                } else
                {
                    if(x == _header)
                        return false;

                    std::size_t expected_span =
                        _length - 1 - static_cast<std::size_t>(x_idx);
                    if(span != expected_span)
                        return false;
                }

                x = next;
            }

            if(total_span != _length)
                return false;
        }

        return true;
    }

    bool _validate_unused_levels() const
    {
        for(int i = _level; i < kMaxLevel; ++i)
        {
            if(_header->levels()[i].forward != nullptr
               || _header->levels()[i].span != 0)
                return false;
        }
        return true;
    }

  private:
    node_t      *_header{nullptr};
    node_t      *_tail{nullptr};
    std::size_t  _length{0};
    int          _level{1};
    Compare      _compare;
    ScoreCompare _score_compare;
    Allocator    _alloc;
};

template <typename T,
          typename Score,
          typename Compare,
          typename ScoreCompare,
          typename Allocator>
inline void
swap(skiplist<T, Score, Compare, ScoreCompare, Allocator> &a,
     skiplist<T, Score, Compare, ScoreCompare, Allocator> &b) noexcept
{
    a.swap(b);
}

template <typename T,
          typename Score        = double,
          typename Compare      = std::less<T>,
          typename ScoreCompare = std::less<Score>,
          typename Allocator    = std::allocator<T>>
struct range_result
{
    using iterator =
        typename skiplist<T, Score, Compare, ScoreCompare, Allocator>::iterator;
    std::vector<iterator> iterators;
    std::size_t           total_in_range{0};
};

template <typename T,
          typename Score        = double,
          typename Compare      = std::less<T>,
          typename ScoreCompare = std::less<Score>,
          typename Alloc        = std::allocator<T>>
inline range_result<T, Score, Compare, ScoreCompare, Alloc>
range_by_score(const skiplist<T, Score, Compare, ScoreCompare, Alloc> &sl,
               const Score          min_score,
               const Score          max_score,
               const std::size_t    offset     = 0,
               const std::ptrdiff_t limit      = -1,
               const bool           with_total = true)
{
    range_result<T, Score, Compare, ScoreCompare, Alloc> result;

    typename skiplist<T, Score, Compare, ScoreCompare, Alloc>::iterator
        first_it;

    if(with_total)
    {
        auto [fit, rank_first] =
            sl.first_in_range_with_rank(min_score, max_score);
        if(fit == sl.end())
            return result;

        auto [lit, rank_last] =
            sl.last_in_range_with_rank(min_score, max_score);
        if(rank_last != skiplist<T, Score, Compare, ScoreCompare, Alloc>::npos
           && rank_last >= rank_first)
        {
            result.total_in_range = rank_last - rank_first + 1;
        }

        first_it = fit;
    } else
    {
        first_it = sl.first_in_range(min_score, max_score);
        if(first_it == sl.end())
            return result;

        result.total_in_range = 0;
    }

    auto score_cmp = sl.score_comp();
    auto is_le_max = [&](const Score &s) { return !score_cmp(max_score, s); };
    auto x         = first_it;
    for(std::size_t i = 0; i < offset && x != sl.end() && is_le_max(x.score());
        i++)
        ++x;

    std::ptrdiff_t collected = 0;
    while(x != sl.end() && is_le_max(x.score())
          && (limit == -1 || collected < limit))
    {
        result.iterators.push_back(x);
        collected++;
        ++x;
    }

    return result;
}

template <typename T,
          typename Score        = double,
          typename Compare      = std::less<T>,
          typename ScoreCompare = std::less<Score>,
          typename Alloc        = std::allocator<T>>
inline range_result<T, Score, Compare, ScoreCompare, Alloc>
range_by_rank(const skiplist<T, Score, Compare, ScoreCompare, Alloc> &sl,
              const std::size_t                                       start,
              const std::size_t                                       end_param,
              const bool reverse = false)
{
    range_result<T, Score, Compare, ScoreCompare, Alloc> result;

    if(start > end_param || start >= sl.size())
        return result;

    std::size_t end = end_param;
    if(end >= sl.size())
        end = sl.size() - 1;

    result.total_in_range = end - start + 1;

    if(!reverse)
    {
        auto        x     = sl.get_element_by_rank(start);
        std::size_t count = end - start + 1;
        while(x != sl.end() && count > 0)
        {
            result.iterators.push_back(x);
            ++x;
            count--;
        }
    } else
    {
        auto        x     = sl.get_element_by_rank(end);
        std::size_t count = end - start + 1;
        while(x != sl.end() && count > 0)
        {
            result.iterators.push_back(x);
            --x;
            count--;
        }
    }

    return result;
}

} // namespace hj

#endif // SKIPLIST_HPP