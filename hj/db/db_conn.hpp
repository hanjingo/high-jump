/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2026 hanjingo <hehehunanchina@live.com>
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

#ifndef DB_CONN_HPP
#define DB_CONN_HPP

#include <cassert>
#include <variant>
#include <string>
#include <string_view>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <utility>
#include <type_traits>
#include <tuple>
#include <functional>
#include <system_error>

namespace hj
{

class db_conn
{
  public:
    using err_t  = std::error_code;
    using blob_t = std::vector<uint8_t>;

    struct blob_view
    {
        const uint8_t *data{nullptr};
        std::size_t    size{0};
    };

    using val_in_t = std::
        variant<std::nullptr_t, int64_t, double, std::string_view, blob_view>;

    using out_val_t =
        std::variant<std::nullptr_t, int64_t, double, std::string, blob_t>;

    struct exec_result
    {
        err_t   ec;
        int64_t affected_rows{0};
        int64_t last_insert_id{0};
    };

    class ret_t
    {
      public:
        ret_t() = default;
        ret_t(std::size_t rows, std::size_t cols)
            : _rows(rows)
            , _cols(cols)
        {
            _data.reserve(rows * cols);
        }

        class row_view
        {
            const out_val_t *begin_;
            size_t           cols_;

          public:
            row_view(const out_val_t *begin, size_t cols)
                : begin_(begin)
                , cols_(cols)
            {
            }
            const out_val_t &operator[](size_t col) const
            {
                return begin_[col];
            }
        };

        row_view operator[](size_t row) const
        {
            assert(row < _rows);
            return row_view(&_data[row * _cols], _cols);
        }

        inline std::size_t rows() const { return _rows; }
        inline std::size_t cols() const { return _cols; }
        inline bool        empty() const { return _data.empty(); }

        inline void set_dim(std::size_t rows, std::size_t cols)
        {
            _rows = rows;
            _cols = cols;
        }

        inline void reserve(std::size_t capacity) { _data.reserve(capacity); }

        inline const out_val_t &get(std::size_t r, std::size_t c) const
        {
            assert(r < _rows && c < _cols && "ret_t index out of range!");
            return _data[r * _cols + c];
        }

        inline void emplace_back(out_val_t val)
        {
            _data.emplace_back(std::move(val));
        }

        inline bool is_null(std::size_t r, std::size_t c) const
        {
            return std::holds_alternative<std::nullptr_t>(get(r, c));
        }

        template <typename T>
        inline const T *get_if(std::size_t r, std::size_t c) const
        {
            return std::get_if<T>(&get(r, c));
        }

        template <typename T>
        inline T get_or(std::size_t r, std::size_t c, T default_val) const
        {
            if(auto p = get_if<T>(r, c))
                return *p;
            return default_val;
        }

      private:
        std::size_t            _rows{0};
        std::size_t            _cols{0};
        std::vector<out_val_t> _data;
    };

    using exec_cb_t  = std::function<void(exec_result res)>;
    using query_cb_t = std::function<void(err_t ec, ret_t outs)>;

    template <typename... Args>
    static constexpr bool is_first_arg_special = [] {
        if constexpr(sizeof...(Args) == 0)
            return false;
        else
        {
            using First = std::decay_t<
                std::tuple_element_t<0, std::tuple<Args..., void>>>;
            return std::is_convertible_v<First, const val_in_t *>
                   || std::is_convertible_v<First, const out_val_t *>
                   || std::is_same_v<First, std::vector<out_val_t>>;
        }
    }();

  public:
    virtual ~db_conn() = default;

    virtual std::string id() const = 0;
    virtual err_t       begin()    = 0;
    virtual err_t       commit()   = 0;
    virtual err_t       rollback() = 0;

    exec_result
    exec(std::string_view sql, const val_in_t *args, std::size_t count)
    {
        return exec_impl(sql, args, count);
    }

    template <typename... Args,
              typename = std::enable_if_t<!is_first_arg_special<Args...>>>
    exec_result exec(std::string_view sql, Args &&...args)
    {
        if constexpr(sizeof...(Args) == 0)
        {
            return exec_impl(sql, nullptr, 0);
        } else
        {
            const val_in_t stack_args[] = {
                val_in_t(std::forward<Args>(args))...};
            return exec_impl(sql, stack_args, sizeof...(Args));
        }
    }

    err_t query(ret_t           &outs,
                std::string_view sql,
                const val_in_t  *args,
                std::size_t      count)
    {
        return query_impl(outs, sql, args, count);
    }

    template <typename... Args,
              typename = std::enable_if_t<!is_first_arg_special<Args...>>>
    err_t query(ret_t &outs, std::string_view sql, Args &&...args)
    {
        if constexpr(sizeof...(Args) == 0)
        {
            return query_impl(outs, sql, nullptr, 0);
        } else
        {
            const val_in_t stack_args[] = {
                val_in_t(std::forward<Args>(args))...};
            return query_impl(outs, sql, stack_args, sizeof...(Args));
        }
    }

    void exec_async(exec_cb_t cb, std::string sql, std::vector<out_val_t> args)
    {
        exec_async_impl(std::move(cb), std::move(sql), std::move(args));
    }

    template <typename... Args,
              typename = std::enable_if_t<!is_first_arg_special<Args...>>>
    void exec_async(exec_cb_t cb, std::string sql, Args &&...args)
    {
        if constexpr(sizeof...(Args) == 0)
        {
            exec_async_impl(std::move(cb), std::move(sql), {});
        } else
        {
            std::vector<out_val_t> heap_args;
            heap_args.reserve(sizeof...(Args));
            (heap_args.emplace_back(to_out_val(std::forward<Args>(args))), ...);
            exec_async_impl(std::move(cb),
                            std::move(sql),
                            std::move(heap_args));
        }
    }

    void
    query_async(query_cb_t cb, std::string sql, std::vector<out_val_t> args)
    {
        query_async_impl(std::move(cb), std::move(sql), std::move(args));
    }

    template <typename... Args,
              typename = std::enable_if_t<!is_first_arg_special<Args...>>>
    void query_async(query_cb_t cb, std::string sql, Args &&...args)
    {
        if constexpr(sizeof...(Args) == 0)
        {
            query_async_impl(std::move(cb), std::move(sql), {});
        } else
        {
            std::vector<out_val_t> heap_args;
            heap_args.reserve(sizeof...(Args));
            (heap_args.emplace_back(to_out_val(std::forward<Args>(args))), ...);
            query_async_impl(std::move(cb),
                             std::move(sql),
                             std::move(heap_args));
        }
    }

  protected:
    virtual exec_result exec_impl(std::string_view sql,
                                  const val_in_t  *args,
                                  std::size_t      count)            = 0;
    virtual void        exec_async_impl(exec_cb_t              cb,
                                        std::string            sql,
                                        std::vector<out_val_t> args) = 0;

    virtual err_t query_impl(ret_t           &outs,
                             std::string_view sql,
                             const val_in_t  *args,
                             std::size_t      count)            = 0;
    virtual void  query_async_impl(query_cb_t             cb,
                                   std::string            sql,
                                   std::vector<out_val_t> args) = 0;

  private:
    template <typename T>
    static out_val_t to_out_val(T &&arg)
    {
        using Decayed = std::decay_t<T>;
        if constexpr(std::is_convertible_v<Decayed, std::string_view>)
        {
            return std::string(arg);
        } else if constexpr(std::is_same_v<Decayed, blob_view>)
        {
            return blob_t(arg.data, arg.data + arg.size);
        } else
        {
            return out_val_t(std::forward<T>(arg));
        }
    }
};

} // namespace hj

#endif // DB_CONN_HPP