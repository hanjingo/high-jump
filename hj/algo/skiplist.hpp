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

// for more information, see:
// https://github.com/redis/redis/blob/unstable/src/t_zset.c

#ifndef SKIPLIST_HPP
#define SKIPLIST_HPP

#include <random>
#include <vector>
#include <functional>
#include <memory>
#include <cassert>
#include <type_traits>

#define SKIPLIST_MAXLEVEL 32

#define SKIPLIST_P 0.25

namespace hj
{
template <typename T>
class skiplist
{
  public:
    using comparable = std::function<bool(const T &, const T &)>;

    struct node_t
    {
        T              obj;
        double         score;
        struct node_t *backward;

        struct level
        {
            struct node_t *forward;
            unsigned long  span;
        } _level[];

        node_t() = delete;
        node_t(int _level, double score, const T &obj)
            : obj(obj)
            , score(score)
            , backward(nullptr)
        {
        }
    };

    class iterator
    {
      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = T *;
        using reference         = T &;

      public:
        iterator(node_t *node)
            : current(node)
        {
        }

        T       &operator*() { return current->obj; }
        const T &operator*() const { return current->obj; }

        T       *operator->() { return &current->obj; }
        const T *operator->() const { return &current->obj; }

        iterator &operator++()
        {
            if(current)
                current = current->_level[0].forward;
            return *this;
        }

        iterator operator++(int)
        {
            iterator temp = *this;
            ++(*this);
            return temp;
        }

        iterator &operator--()
        {
            if(current)
                current = current->backward;
            return *this;
        }

        iterator operator--(int)
        {
            iterator temp = *this;
            --(*this);
            return temp;
        }

        bool operator==(const iterator &other) const
        {
            return current == other.current;
        }

        bool operator!=(const iterator &other) const
        {
            return current != other.current;
        }

        inline double  score() const { return current ? current->score : 0.0; }
        inline node_t *node() const { return current; }

      private:
        node_t *current;
    };

    class const_iterator
    {
      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = T *;
        using reference         = T &;

      public:
        const_iterator(const node_t *node)
            : current(node)
        {
        }

        const_iterator(const iterator &it)
            : current(it.node())
        {
        }

        const T &operator*() const { return current->obj; }
        const T *operator->() const { return &current->obj; }

        const_iterator &operator++()
        {
            if(current)
                current = current->_level[0].forward;
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
                current = current->backward;
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

        bool operator==(const iterator &other) const
        {
            return current == other.node();
        }

        bool operator!=(const iterator &other) const
        {
            return current != other.node();
        }

        inline double score() const { return current ? current->score : 0.0; }
        inline const node_t *node() const { return current; }

      private:
        const node_t *current;
    };

  public:
    explicit skiplist(comparable cmp = std::less<T>())
        : _tail(nullptr)
        , _length(0)
        , _level(1)
        , _gen(_rd())
        , _dis(0.0, 1.0)
        , _compare(cmp)
    {
        _header = _create_node(SKIPLIST_MAXLEVEL, 0, T{});
        for(int j = 0; j < SKIPLIST_MAXLEVEL; j++)
        {
            _header->_level[j].forward = nullptr;
            _header->_level[j].span    = 0;
        }
        _header->backward = nullptr;
    }

    ~skiplist() { release(); }

    skiplist(const skiplist &)            = delete;
    skiplist &operator=(const skiplist &) = delete;
    skiplist(skiplist &&)                 = delete;
    skiplist &operator=(skiplist &&)      = delete;

    inline unsigned long  size() const { return _length; }
    inline bool           empty() const { return _length == 0; }
    inline node_t        *first() const { return _header->_level[0].forward; }
    inline node_t        *last() const { return _tail; }
    inline iterator       begin() { return iterator(first()); }
    inline iterator       end() { return iterator(nullptr); }
    inline const_iterator begin() const { return const_iterator(first()); }
    inline const_iterator end() const { return const_iterator(nullptr); }
    inline const_iterator cbegin() const { return const_iterator(first()); }
    inline const_iterator cend() const { return const_iterator(nullptr); }

    // Insert element with score
    node_t *insert(double score, const T &obj)
    {
        node_t       *update[SKIPLIST_MAXLEVEL];
        unsigned long rank[SKIPLIST_MAXLEVEL];
        node_t       *x = _header;

        for(int i = _level - 1; i >= 0; i--)
        {
            rank[i] = (i == (_level - 1)) ? 0 : rank[i + 1];
            while(x->_level[i].forward
                  && (x->_level[i].forward->score < score
                      || (x->_level[i].forward->score == score
                          && _compare(x->_level[i].forward->obj, obj))))
            {
                rank[i] += x->_level[i].span;
                x = x->_level[i].forward;
            }
            update[i] = x;
        }

        int new_level = _random_level();
        if(new_level > _level)
        {
            for(int i = _level; i < new_level; i++)
            {
                rank[i]                   = 0;
                update[i]                 = _header;
                update[i]->_level[i].span = _length;
            }
            _level = new_level;
        }

        x = _create_node(new_level, score, obj);
        for(int i = 0; i < new_level; i++)
        {
            x->_level[i].forward         = update[i]->_level[i].forward;
            update[i]->_level[i].forward = x;

            x->_level[i].span = update[i]->_level[i].span - (rank[0] - rank[i]);
            update[i]->_level[i].span = (rank[0] - rank[i]) + 1;
        }

        for(int i = new_level; i < _level; i++)
            update[i]->_level[i].span++;

        x->backward = (update[0] == _header) ? nullptr : update[0];
        if(x->_level[0].forward)
            x->_level[0].forward->backward = x;
        else
            _tail = x;

        _length++;
        return x;
    }

    bool delete_node(double score, const T &obj)
    {
        node_t *update[SKIPLIST_MAXLEVEL];
        node_t *x = _header;
        for(int i = _level - 1; i >= 0; i--)
        {
            while(x->_level[i].forward
                  && (x->_level[i].forward->score < score
                      || (x->_level[i].forward->score == score
                          && _compare(x->_level[i].forward->obj, obj))))
            {
                x = x->_level[i].forward;
            }
            update[i] = x;
        }

        x = x->_level[0].forward;
        if(x && x->score == score && !_compare(x->obj, obj)
           && !_compare(obj, x->obj))
        {
            _delete_node_internal(x, update);
            return true;
        }

        return false;
    }

    node_t *get_element_by_rank(unsigned long rank) const
    {
        if(rank >= _length)
            return nullptr;

        node_t       *x         = _header;
        unsigned long traversed = 0;

        for(int i = _level - 1; i >= 0; i--)
        {
            while(x->_level[i].forward
                  && (traversed + x->_level[i].span - 1) < rank)
            {
                traversed += x->_level[i].span;
                x = x->_level[i].forward;
            }
        }

        x = x->_level[0].forward;
        return x;
    }

    unsigned long get_rank(double score, const T &obj) const
    {
        node_t       *x    = _header;
        unsigned long rank = 0;
        for(int i = _level - 1; i >= 0; i--)
        {
            while(x->_level[i].forward
                  && (x->_level[i].forward->score < score
                      || (x->_level[i].forward->score == score
                          && _compare(x->_level[i].forward->obj, obj))))
            {
                rank += x->_level[i].span;
                x = x->_level[i].forward;
            }

            if(x->_level[i].forward && x->_level[i].forward->score == score
               && !_compare(x->_level[i].forward->obj, obj)
               && !_compare(obj, x->_level[i].forward->obj))
            {
                rank += x->_level[i].span;
                return rank - 1;
            }
        }

        return _length; // Not found
    }

    node_t *first_in_range(double min_score, double max_score) const
    {
        if(!_is_in_range(min_score, max_score))
            return nullptr;

        node_t *x = _header;
        for(int i = _level - 1; i >= 0; i--)
        {
            while(x->_level[i].forward
                  && x->_level[i].forward->score < min_score)
                x = x->_level[i].forward;
        }

        x = x->_level[0].forward;
        if(!x || x->score > max_score)
            return nullptr;

        return x;
    }

    node_t *last_in_range(double min_score, double max_score) const
    {
        if(!_is_in_range(min_score, max_score))
            return nullptr;

        node_t *x = _header;
        for(int i = _level - 1; i >= 0; i--)
        {
            while(x->_level[i].forward
                  && x->_level[i].forward->score <= max_score)
                x = x->_level[i].forward;
        }

        if(!x || x->score < min_score)
            return nullptr;

        return x;
    }

    unsigned long delete_range_by_score(double min_score, double max_score)
    {
        node_t       *update[SKIPLIST_MAXLEVEL];
        node_t       *x       = _header;
        unsigned long removed = 0;
        for(int i = _level - 1; i >= 0; i--)
        {
            while(x->_level[i].forward
                  && x->_level[i].forward->score < min_score)
                x = x->_level[i].forward;

            update[i] = x;
        }

        x = x->_level[0].forward;
        while(x && x->score <= max_score)
        {
            node_t *next = x->_level[0].forward;
            _delete_node_internal(x, update);
            removed++;
            x = next;
        }

        return removed;
    }

    unsigned long delete_range_by_rank(unsigned long start, unsigned long end)
    {
        if(start > end || start >= _length)
            return 0;

        if(end >= _length)
            end = _length - 1;

        node_t       *update[SKIPLIST_MAXLEVEL];
        node_t       *x         = _header;
        unsigned long traversed = 0;
        unsigned long removed   = 0;
        for(int i = _level - 1; i >= 0; i--)
        {
            while(x->_level[i].forward
                  && traversed + x->_level[i].span <= start)
            {
                traversed += x->_level[i].span;
                x = x->_level[i].forward;
            }
            update[i] = x;
        }

        x                          = x->_level[0].forward;
        unsigned long current_rank = start;
        while(x && current_rank <= end)
        {
            node_t *next = x->_level[0].forward;
            _delete_node_internal(x, update);
            removed++;
            current_rank++;
            x = next;
        }

        return removed;
    }

    bool validate() const
    {
        node_t       *x     = _header;
        unsigned long count = 0;
        x                   = _header->_level[0].forward;
        while(x)
        {
            count++;
            if(x->_level[0].forward && x->score > x->_level[0].forward->score)
                return false;

            x = x->_level[0].forward;
        }

        return count == _length;
    }

    void release()
    {
        node_t *forward = _header->_level[0].forward;
        while(forward)
        {
            node_t *next = forward->_level[0].forward;
            _free_node(forward);
            forward = next;
        }
        _free_node(_header);
    }

  private:
    node_t *_create_node(int _level, double score, const T &obj)
    {
        size_t  size = sizeof(node_t) + _level * sizeof(typename node_t::level);
        void   *raw  = ::operator new(size);
        node_t *new_node = new(raw) node_t(_level, score, obj);
        return new_node;
    }

    void _free_node(node_t *node)
    {
        node->~node_t();
        ::operator delete(node);
    }

    int _random_level() const
    {
        int _level = 1;
        while(_dis(_gen) < SKIPLIST_P && _level < SKIPLIST_MAXLEVEL)
            _level++;

        return _level;
    }

    void _delete_node_internal(node_t *x, node_t **update)
    {
        for(int i = 0; i < _level; i++)
        {
            if(update[i]->_level[i].forward == x)
            {
                update[i]->_level[i].span += x->_level[i].span - 1;
                update[i]->_level[i].forward = x->_level[i].forward;
            } else
            {
                update[i]->_level[i].span -= 1;
            }
        }

        if(x->_level[0].forward)
            x->_level[0].forward->backward = x->backward;
        else
            _tail = x->backward;

        _free_node(x);
        while(_level > 1 && _header->_level[_level - 1].forward == nullptr)
            _level--;

        _length--;
    }

    bool _is_in_range(double min_score, double max_score) const
    {
        if(min_score > max_score)
            return false;

        node_t *x = _tail;
        if(x == nullptr || x->score < min_score)
            return false;

        x = _header->_level[0].forward;
        if(x == nullptr || x->score > max_score)
            return false;

        return true;
    }

  private:
    struct level
    {
        node_t       *forward;
        unsigned long span;
    };

    node_t                                        *_header;
    node_t                                        *_tail;
    unsigned long                                  _length;
    int                                            _level;
    mutable std::random_device                     _rd;
    mutable std::mt19937                           _gen;
    mutable std::uniform_real_distribution<double> _dis;
    comparable                                     _compare;
};

// Range result for range queries
template <typename T>
struct range_result
{
    std::vector<typename skiplist<T>::node_t *> nodes;
    unsigned long                               total_in_range;

    range_result()
        : total_in_range(0)
    {
    }
};

template <typename T>
inline range_result<T> range_by_score(const skiplist<T>  &sl,
                                      const double        min_score,
                                      const double        max_score,
                                      const unsigned long offset = 0,
                                      const long          limit  = -1)
{
    return detail::_range_by_score(sl, min_score, max_score, offset, limit);
}

template <typename T>
inline range_result<T> range_by_rank(const skiplist<T>  &sl,
                                     const unsigned long start,
                                     const unsigned long end_param,
                                     const bool          reverse = false)
{
    return detail::_range_by_rank(sl, start, end_param, reverse);
}

namespace detail
{
template <typename T>
range_result<T> _range_by_score(const skiplist<T>  &sl,
                                const double        min_score,
                                const double        max_score,
                                const unsigned long offset,
                                const long          limit)
{
    range_result<T> result;

    auto *x = sl.first_in_range(min_score, max_score);
    if(!x)
        return result;

    // Skip offset nodes
    for(unsigned long i = 0; i < offset && x && x->score <= max_score; i++)
    {
        x = x->_level[0].forward;
        result.total_in_range++;
    }

    // Collect nodes up to limit
    long collected = 0;
    while(x && x->score <= max_score && (limit == -1 || collected < limit))
    {
        result.nodes.push_back(x);
        result.total_in_range++;
        collected++;
        x = x->_level[0].forward;
    }

    // Count remaining nodes in range
    while(x && x->score <= max_score)
    {
        result.total_in_range++;
        x = x->_level[0].forward;
    }

    return result;
}

template <typename T>
range_result<T> _range_by_rank(const skiplist<T>  &sl,
                               const unsigned long start,
                               const unsigned long end_param,
                               const bool          reverse)
{
    range_result<T> result;

    if(start > end_param || start >= sl.size())
        return result;

    unsigned long end = end_param;
    if(end >= sl.size())
        end = sl.size() - 1;

    if(!reverse)
    {
        // Forward iteration
        for(unsigned long i = start; i <= end; i++)
        {
            auto *node = sl.get_element_by_rank(i);
            if(node)
            {
                result.nodes.push_back(node);
                result.total_in_range++;
            }
        }
    } else
    {
        // Reverse iteration
        for(unsigned long i = end + 1; i > start; i--)
        {
            auto *node = sl.get_element_by_rank(i - 1);
            if(node)
            {
                result.nodes.push_back(node);
                result.total_in_range++;
            }
        }
    }

    return result;
}

} // namespace detail
} // namespace hj

#endif // SKIPLIST_HPP