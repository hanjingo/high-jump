#include <gtest/gtest.h>
#include <vector>
#include <libcpp/testing/exception.hpp>

TEST(exception, throw_if_null)
{
    try {
        libcpp::throw_if_null(nullptr);
    } catch(const char* memo) {
        ASSERT_STREQ(memo, "null");
    }

    try {
        libcpp::throw_if_null(nullptr, "world");
    } catch(const char* memo) {
        ASSERT_STREQ(memo, "world");
    }
}

TEST(exception, throw_if_not_null)
{
    try {
        int i = 0;
        libcpp::throw_if_not_null(&i);
    } catch(const char* memo) {
        ASSERT_STREQ(memo, "not null");
    }

    try {
        int i = 0;
        libcpp::throw_if_not_null(&i, "hello");
    } catch(const char* memo) {
        ASSERT_STREQ(memo, "hello");
    }
}

TEST(exception, throw_if_exists)
{
    try {
        std::vector<int> ct{1, 2, 3};
        libcpp::throw_if_exists(ct, 2);
    } catch(const char* memo) {
        ASSERT_STREQ(memo, "already exist");
    }
}

TEST(exception, throw_if_not_exists)
{
    try {
        std::vector<int> ct{1, 2, 3};
        libcpp::throw_if_not_exists(ct, 0);
    } catch(const char* memo) {
        ASSERT_STREQ(memo, "not exist");
    }
}