#include <vector>

#include <gtest/gtest.h>
#include <libcpp/testing/exception.hpp>

TEST (exception, throw_if_false)
{
    try {
        libcpp::throw_if_false (false);
    }
    catch (std::exception &memo) {
        ASSERT_STREQ (memo.what (), "false");
    }

    try {
        libcpp::throw_if_false (false, "condition false");
    }
    catch (std::exception &memo) {
        ASSERT_STREQ (memo.what (), "condition false");
    }
}

TEST (exception, throw_if_not_false)
{
    try {
        libcpp::throw_if_not_false (true);
    }
    catch (std::exception &memo) {
        ASSERT_STREQ (memo.what (), "not false");
    }

    try {
        libcpp::throw_if_not_false (true, "condition true");
    }
    catch (std::exception &memo) {
        ASSERT_STREQ (memo.what (), "condition true");
    }
}

TEST (exception, throw_if_equal)
{
    try {
        libcpp::throw_if_equal (false, false);
    }
    catch (std::exception &memo) {
        ASSERT_STREQ (memo.what (), "equal");
    }

    try {
        libcpp::throw_if_equal (false, true, "equal panic");
    }
    catch (std::exception &memo) {
        ASSERT_STREQ (memo.what (), "equal panic");
    }
}

TEST (exception, throw_if_not_equal)
{
    try {
        libcpp::throw_if_not_equal (true, false);
    }
    catch (std::exception &memo) {
        ASSERT_STREQ (memo.what (), "not equal");
    }

    try {
        libcpp::throw_if_not_equal (true, true, "not equal panic");
    }
    catch (std::exception &memo) {
        ASSERT_STREQ (memo.what (), "not equal panic");
    }
}

TEST (exception, throw_if_empty)
{
    try {
        std::string str1{};
        libcpp::throw_if_empty (str1);
    }
    catch (std::exception &memo) {
        ASSERT_STREQ (memo.what (), "empty");
    }

    try {
        std::string str2{"hello"};
        libcpp::throw_if_empty (str2);
    }
    catch (std::exception &memo) {
        ASSERT_FALSE (false);
    }
}

TEST (exception, throw_if_not_empty)
{
    try {
        std::string str1{"hello"};
        libcpp::throw_if_not_empty (str1);
    }
    catch (std::exception &memo) {
        ASSERT_STREQ (memo.what (), "not empty");
    }

    try {
        std::string str2{};
        libcpp::throw_if_not_empty (str2);
    }
    catch (std::exception &memo) {
        ASSERT_FALSE (false);
    }
}

TEST (exception, throw_if_null)
{
    try {
        libcpp::throw_if_null (nullptr);
    }
    catch (std::exception &memo) {
        ASSERT_STREQ (memo.what (), "null");
    }

    try {
        libcpp::throw_if_null (nullptr, "world");
    }
    catch (std::exception &memo) {
        ASSERT_STREQ (memo.what (), "world");
    }
}

TEST (exception, throw_if_not_null)
{
    try {
        int i = 0;
        libcpp::throw_if_not_null (&i);
    }
    catch (std::exception &memo) {
        ASSERT_STREQ (memo.what (), "not null");
    }

    try {
        int i = 0;
        libcpp::throw_if_not_null (&i, "hello");
    }
    catch (std::exception &memo) {
        ASSERT_STREQ (memo.what (), "hello");
    }
}

TEST (exception, throw_if_exists)
{
    try {
        std::vector<int> ct{1, 2, 3};
        libcpp::throw_if_exists (ct, 2);
    }
    catch (std::exception &memo) {
        ASSERT_STREQ (memo.what (), "already exist");
    }
}

TEST (exception, throw_if_not_exists)
{
    try {
        std::vector<int> ct{1, 2, 3};
        libcpp::throw_if_not_exists (ct, 0);
    }
    catch (std::exception &memo) {
        ASSERT_STREQ (memo.what (), "not exist");
    }
}