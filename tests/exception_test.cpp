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

#include <gtest/gtest.h>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <hj/testing/exception.hpp>

class FooWithBoolOperator
{
  public:
    explicit operator bool() const { return true; }
};

TEST(exception, custom_exception_class)
{
    try
    {
        throw hj::Exception("custom error message");
    }
    catch(const hj::Exception &e)
    {
        EXPECT_STREQ(e.what(), "custom error message");
        EXPECT_NO_THROW({
            auto trace_obj = e.trace();
            static_cast<void>(trace_obj);
        });
    }

    EXPECT_THROW(
        { throw hj::NotFoundException("not found exact type"); },
        hj::NotFoundException);

    try
    {
        throw hj::NotFoundException("resource not found");
    }
    catch(const hj::NotFoundException &e)
    {
        EXPECT_STREQ(e.what(), "resource not found");
    }
}

TEST(exception, empty_message)
{
    try
    {
        throw hj::Exception("");
    }
    catch(const hj::Exception &e)
    {
        EXPECT_STREQ(e.what(), "");
    }

    EXPECT_THROW(hj::throw_if_false(false, {}), std::logic_error);
    try
    {
        hj::throw_if_false(false, {});
    }
    catch(const std::logic_error &e)
    {
        EXPECT_STREQ(e.what(), "");
    }
}

TEST(exception, custom_stacktrace_preservation)
{
    boost::stacktrace::stacktrace custom_trace  = hj::current_stacktrace();
    std::size_t                   expected_size = custom_trace.size();

    hj::Exception e("error with custom trace", custom_trace);

    EXPECT_STREQ(e.what(), "error with custom trace");
    EXPECT_EQ(e.trace().size(), expected_size);
}

TEST(exception, compile_time_nullable_traits)
{
    static_assert(!hj::detail::is_nullable_v<int>, "int is not nullable");
    static_assert(!hj::detail::is_nullable_v<double>, "double is not nullable");
    static_assert(!hj::detail::is_nullable_v<std::string>,
                  "string is not nullable");
    static_assert(!hj::detail::is_nullable_v<FooWithBoolOperator>,
                  "Foo is not nullable");

    static_assert(hj::detail::is_nullable_v<int *>, "int* is nullable");
    static_assert(hj::detail::is_nullable_v<const char *>,
                  "const char* is nullable");
    static_assert(hj::detail::is_nullable_v<std::nullptr_t>,
                  "nullptr_t is nullable");
    static_assert(hj::detail::is_nullable_v<std::shared_ptr<int>>,
                  "shared_ptr is nullable");
    static_assert(hj::detail::is_nullable_v<std::unique_ptr<int>>,
                  "unique_ptr is nullable");
    static_assert(hj::detail::is_nullable_v<std::weak_ptr<int>>,
                  "weak_ptr is nullable");
    static_assert(hj::detail::is_nullable_v<std::optional<int>>,
                  "optional is nullable");
}

TEST(exception, throw_if_null)
{
    int *null_ptr = nullptr;
    int  val      = 100;

    EXPECT_THROW(hj::throw_if_null(null_ptr), std::invalid_argument);
    EXPECT_NO_THROW(hj::throw_if_null(&val));

    std::shared_ptr<int> empty_shared;
    auto                 valid_shared = std::make_shared<int>(42);

    EXPECT_THROW(hj::throw_if_null(empty_shared, "shared null"),
                 std::invalid_argument);
    EXPECT_NO_THROW(hj::throw_if_null(valid_shared));

    std::unique_ptr<int> empty_unique;
    auto                 valid_unique = std::make_unique<int>(42);

    EXPECT_THROW(hj::throw_if_null(empty_unique, "unique null"),
                 std::invalid_argument);
    EXPECT_NO_THROW(hj::throw_if_null(valid_unique));

    std::optional<int> empty_opt;
    std::optional<int> valid_opt = 42;

    EXPECT_THROW(hj::throw_if_null(empty_opt, "optional null"),
                 std::invalid_argument);
    EXPECT_NO_THROW(hj::throw_if_null(valid_opt));

    std::weak_ptr<int> expired_weak;
    {
        auto temp_shared = std::make_shared<int>(99);
        expired_weak     = temp_shared;
    }
    EXPECT_THROW(hj::throw_if_null(expired_weak, "weak expired"),
                 std::invalid_argument);
}

TEST(exception, throw_if_not_null)
{
    int *null_ptr = nullptr;
    int  val      = 100;

    EXPECT_THROW(hj::throw_if_not_null(&val, "not null"),
                 std::invalid_argument);
    EXPECT_NO_THROW(hj::throw_if_not_null(null_ptr));

    auto                 valid_shared = std::make_shared<int>(42);
    std::shared_ptr<int> empty_shared;

    EXPECT_THROW(hj::throw_if_not_null(valid_shared), std::invalid_argument);
    EXPECT_NO_THROW(hj::throw_if_not_null(empty_shared));

    auto                 valid_unique = std::make_unique<int>(123);
    std::unique_ptr<int> empty_unique;

    EXPECT_THROW(hj::throw_if_not_null(valid_unique, "unique not null"),
                 std::invalid_argument);
    EXPECT_NO_THROW(hj::throw_if_not_null(empty_unique));

    std::optional<int> valid_opt = 456;
    std::optional<int> empty_opt = std::nullopt;

    EXPECT_THROW(hj::throw_if_not_null(valid_opt, "optional not null"),
                 std::invalid_argument);
    EXPECT_NO_THROW(hj::throw_if_not_null(empty_opt));
}

TEST(exception, string_view_non_null_terminated)
{
    std::string      value = "abcdef";
    std::string_view view(value.data(), 3);

    EXPECT_THROW(hj::throw_if_false(false, view), std::logic_error);

    try
    {
        hj::throw_if_false(false, view);
    }
    catch(const std::logic_error &e)
    {
        EXPECT_STREQ(e.what(), "abc");
    }
}

TEST(exception, throw_if_false)
{
    try
    {
        hj::throw_if_false(false);
    }
    catch(const std::logic_error &e)
    {
        EXPECT_STREQ(e.what(), "false");
    }

    try
    {
        hj::throw_if_false(false, "condition false");
    }
    catch(const std::logic_error &e)
    {
        EXPECT_STREQ(e.what(), "condition false");
    }

    EXPECT_NO_THROW(hj::throw_if_false(true));
}

TEST(exception, throw_if_not_false)
{
    try
    {
        hj::throw_if_not_false(true);
    }
    catch(const std::logic_error &e)
    {
        EXPECT_STREQ(e.what(), "not false");
    }

    EXPECT_NO_THROW(hj::throw_if_not_false(false));
}

TEST(exception, throw_if_equal)
{
    EXPECT_THROW(hj::throw_if_equal(10, 10), std::logic_error);
    EXPECT_NO_THROW(hj::throw_if_equal(10, 20, "equal panic"));
}

TEST(exception, throw_if_not_equal)
{
    EXPECT_THROW(hj::throw_if_not_equal(10, 20), std::logic_error);
    EXPECT_NO_THROW(hj::throw_if_not_equal(10, 10));
}

TEST(exception, throw_if_empty)
{
    std::string str1{};
    std::string str2{"hello"};

    EXPECT_THROW(hj::throw_if_empty(str1), std::invalid_argument);
    EXPECT_NO_THROW(hj::throw_if_empty(str2));
}

TEST(exception, throw_if_not_empty)
{
    std::string str1{"hello"};
    std::string str2{};

    EXPECT_THROW(hj::throw_if_not_empty(str1), std::invalid_argument);
    EXPECT_NO_THROW(hj::throw_if_not_empty(str2));
}

TEST(exception, throw_if_exists)
{
    std::vector<int> vec{1, 2, 3};
    EXPECT_THROW(hj::throw_if_exists(vec, 2), std::invalid_argument);
    EXPECT_NO_THROW(hj::throw_if_exists(vec, 5));

    std::set<int> set_ct{10, 20, 30};
    EXPECT_THROW(hj::throw_if_exists(set_ct, 20), std::invalid_argument);
    EXPECT_NO_THROW(hj::throw_if_exists(set_ct, 50));
}

TEST(exception, throw_if_not_exists)
{
    std::vector<int> vec{1, 2, 3};
    EXPECT_THROW(hj::throw_if_not_exists(vec, 0), std::invalid_argument);
    EXPECT_NO_THROW(hj::throw_if_not_exists(vec, 2));

    std::set<int> set_ct{10, 20, 30};
    EXPECT_THROW(hj::throw_if_not_exists(set_ct, 99), std::invalid_argument);
    EXPECT_NO_THROW(hj::throw_if_not_exists(set_ct, 10));
}

TEST(exception, recover_function_with_return_value)
{
    bool handler_called = false;

    auto res1 = hj::recover([]() -> int { return 42; }, [&](auto, auto) {});
    ASSERT_TRUE(res1.has_value());
    EXPECT_EQ(res1.value(), 42);

    auto res2 = hj::recover(
        []() -> int {
            throw hj::Exception("error in recover");
            return 0;
        },
        [&](const std::exception_ptr            &ep,
            const boost::stacktrace::stacktrace &st) {
            handler_called = true;
            EXPECT_NE(ep, nullptr);
            EXPECT_GT(st.size(), 0);
        });

    EXPECT_FALSE(res2.has_value());
    EXPECT_TRUE(handler_called);
}

TEST(exception, recover_or_rethrow_function)
{
    bool handler_called = false;

    int val =
        hj::recover_or_rethrow([]() -> int { return 100; }, [&](auto, auto) {});
    EXPECT_EQ(val, 100);

    EXPECT_THROW(
        {
            hj::recover_or_rethrow(
                []() -> int {
                    throw hj::Exception("rethrow test error");
                    return 0;
                },
                [&](const std::exception_ptr            &ep,
                    const boost::stacktrace::stacktrace &st) {
                    handler_called = true;
                    EXPECT_NE(ep, nullptr);
                    EXPECT_GT(st.size(), 0);
                });
        },
        hj::Exception);

    EXPECT_TRUE(handler_called);
}

TEST(exception, recover_unknown_exception)
{
    bool handler_called = false;

    hj::recover([]() { throw 42; },
                [&](const std::exception_ptr            &ep,
                    const boost::stacktrace::stacktrace &st) {
                    handler_called = true;
                    EXPECT_NE(ep, nullptr);
                    EXPECT_GT(st.size(), 0);
                });

    EXPECT_TRUE(handler_called);
}

TEST(exception, recover_handler_throws_exception)
{
    EXPECT_THROW(
        {
            hj::recover([]() { throw std::runtime_error("primary error"); },
                        [](const std::exception_ptr &,
                           const boost::stacktrace::stacktrace &) {
                            throw std::runtime_error("handler error");
                        });
        },
        std::runtime_error);

    try
    {
        hj::recover([]() { throw std::runtime_error("primary error"); },
                    [](const std::exception_ptr &,
                       const boost::stacktrace::stacktrace &) {
                        throw std::runtime_error("handler error");
                    });
    }
    catch(const std::runtime_error &e)
    {
        EXPECT_STREQ(e.what(), "handler error");
    }
}

TEST(exception, recover_macro)
{
    std::ostringstream ss;

    HJ_RECOVER_WITH_LOG(hj::throw_if_false(false, "macro capture test"), ss);

    std::string log_output = ss.str();
    EXPECT_NE(log_output.find("Exception caught: macro capture test"),
              std::string::npos);
    EXPECT_NE(log_output.find("Stacktrace:"), std::string::npos);
}

TEST(exception, recover_nested_optional)
{
    bool handler_called = false;

    auto res1 = hj::recover([]() -> std::optional<int> { return 42; },
                            [&](auto, auto) {});

    ASSERT_TRUE(res1.has_value());
    ASSERT_TRUE(res1.value().has_value());
    EXPECT_EQ(res1.value().value(), 42);

    auto res2 = hj::recover([]() -> std::optional<int> { return std::nullopt; },
                            [&](auto, auto) {});

    ASSERT_TRUE(res2.has_value());
    EXPECT_FALSE(res2.value().has_value());

    auto res3 = hj::recover(
        []() -> std::optional<int> {
            throw std::runtime_error("inner failure");
            return 42;
        },
        [&](const std::exception_ptr &, const boost::stacktrace::stacktrace &) {
            handler_called = true;
        });

    EXPECT_FALSE(res3.has_value());
    EXPECT_TRUE(handler_called);
}