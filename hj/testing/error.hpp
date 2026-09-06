/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#ifndef ERROR_HPP
#define ERROR_HPP

#include <cstdint>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>

namespace hj
{

namespace detail
{
template <typename T, bool IsEnum = std::is_enum_v<T>>
struct safe_underlying_type
{
    static_assert(std::is_integral_v<T> || std::is_enum_v<T>,
                  "hj error utility API only accepts integral or enum types!");
    using type = T;
};

template <typename T>
struct safe_underlying_type<T, true>
{
    using type = std::underlying_type_t<T>;
};

template <typename T>
using safe_underlying_type_t = typename safe_underlying_type<T>::type;
} // namespace detail


enum class generic_errc
{
    success = 0,
    network_failure,
    io_failure,
    permission_denied,
    resource_unavailable
};

class generic_category_impl final : public std::error_category
{
  public:
    const char *name() const noexcept override { return "hj_generic"; }

    std::string message(int ev) const override
    {
        switch(static_cast<generic_errc>(ev))
        {
            case generic_errc::success:
                return "success";
            case generic_errc::network_failure:
                return "network failure";
            case generic_errc::io_failure:
                return "io failure";
            case generic_errc::permission_denied:
                return "permission denied";
            case generic_errc::resource_unavailable:
                return "resource unavailable";
            default:
                return "unknown generic error";
        }
    }
};

inline const std::error_category &generic_category() noexcept
{
    static generic_category_impl instance;
    return instance;
}

inline std::error_condition make_error_condition(generic_errc e) noexcept
{
    return {static_cast<int>(e), generic_category()};
}


struct nested_error
{
    std::error_code               ec;
    std::shared_ptr<nested_error> cause{nullptr};

    nested_error() noexcept = default;

    /* implicit */ nested_error(
        std::error_code e, std::shared_ptr<nested_error> c = nullptr) noexcept
        : ec(e)
        , cause(std::move(c))
    {
    }

    nested_error(std::error_code e, std::error_code cause_ec)
        : ec(e)
        , cause(std::make_shared<nested_error>(cause_ec))
    {
    }

    explicit operator bool() const noexcept { return static_cast<bool>(ec); }

    [[nodiscard]] std::string to_string() const
    {
        std::string result = ec.category().name();
        result += " error " + std::to_string(ec.value()) + ": " + ec.message();
        if(cause)
        {
            result += " (caused by: " + cause->to_string() + ")";
        }
        return result;
    }
};

inline nested_error
make_nested_error(std::error_code               ec,
                  std::shared_ptr<nested_error> cause = nullptr)
{
    return nested_error{ec, std::move(cause)};
}

inline nested_error make_nested_error(std::error_code ec,
                                      std::error_code cause_ec)
{
    return nested_error{ec, cause_ec};
}


template <typename T>
constexpr auto to_underlying(T err) noexcept
{
    using Underlying = detail::safe_underlying_type_t<T>;
    return static_cast<Underlying>(err);
}

template <typename T>
constexpr int ec_to_int(T err) noexcept
{
    using Underlying = detail::safe_underlying_type_t<T>;

    static_assert(sizeof(Underlying) <= sizeof(int),
                  "ec_to_int: underlying type size exceeds sizeof(int), use "
                  "hj::to_underlying() instead.");

    return static_cast<int>(static_cast<Underlying>(err));
}

template <typename T>
std::string
ec_to_hex(T err, bool upper_case = true, std::string_view prefix = "0x")
{
    using Underlying         = detail::safe_underlying_type_t<T>;
    using UnsignedUnderlying = std::make_unsigned_t<Underlying>;

    auto u_val = static_cast<UnsignedUnderlying>(err);

    std::ostringstream ss;
    if(upper_case)
        ss << std::uppercase;

    if constexpr(sizeof(Underlying) < 4)
    {
        ss << std::hex << static_cast<uint32_t>(u_val);
    } else
    {
        ss << std::hex << u_val;
    }

    std::string res(prefix);
    res.append(ss.str());
    return res;
}

} // namespace hj


namespace std
{
template <>
struct is_error_condition_enum<hj::generic_errc> : true_type
{
};
} // namespace std


#define HJ_REG_ERR_CONDITION(EnumType, ConditionClassName, ConditionName)      \
    static_assert(sizeof(std::underlying_type_t<EnumType>) <= sizeof(int),     \
                  #EnumType " size exceeds sizeof(int), cannot be converted "  \
                            "to std::error_condition safely!");                \
    class ConditionClassName final : public std::error_category                \
    {                                                                          \
      public:                                                                  \
        const char *name() const noexcept override { return ConditionName; }   \
        std::string message(int ev) const override;                            \
    };                                                                         \
    inline const std::error_category &get_##ConditionClassName() noexcept      \
    {                                                                          \
        static ConditionClassName instance;                                    \
        return instance;                                                       \
    }                                                                          \
    inline std::error_condition make_error_condition(EnumType e) noexcept      \
    {                                                                          \
        return std::error_condition(static_cast<int>(e),                       \
                                    get_##ConditionClassName());               \
    }                                                                          \
    namespace std                                                              \
    {                                                                          \
    template <>                                                                \
    struct is_error_condition_enum<EnumType> : true_type                       \
    {                                                                          \
    };                                                                         \
    }


#define HJ_REG_ERR_CATEGORY(EnumType, CategoryClassName, CategoryName)         \
    static_assert(sizeof(std::underlying_type_t<EnumType>) <= sizeof(int),     \
                  #EnumType " size exceeds sizeof(int), cannot be converted "  \
                            "to std::error_code safely!");                     \
    class CategoryClassName : public std::error_category                       \
    {                                                                          \
      public:                                                                  \
        const char *name() const noexcept override { return CategoryName; }    \
        std::string message(int ev) const override;                            \
        std::error_condition                                                   \
        default_error_condition(int ev) const noexcept override;               \
    };                                                                         \
    inline const std::error_category &get_##CategoryClassName() noexcept       \
    {                                                                          \
        static CategoryClassName instance;                                     \
        return instance;                                                       \
    }                                                                          \
    inline std::error_code make_error_code(EnumType e) noexcept                \
    {                                                                          \
        return std::error_code(static_cast<int>(e),                            \
                               get_##CategoryClassName());                     \
    }                                                                          \
    namespace std                                                              \
    {                                                                          \
    template <>                                                                \
    struct is_error_code_enum<EnumType> : true_type                            \
    {                                                                          \
    };                                                                         \
    }

#endif // ERROR_HPP