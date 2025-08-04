#include <string>

#include <gtest/gtest.h>
#include <libcpp/algo/striped_map.hpp>

TEST (striped_map, emplace)
{
    libcpp::striped_map<int, std::string> m{
      [] (const int &key) -> int { return key % 10; }, 10};
    m.emplace (1, "e1");
    m.emplace (11, "e11");
    m.emplace (123, "e123");
    std::string value;
    ASSERT_EQ (m.find (1, value), true);
    ASSERT_EQ (value == std::string ("e1"), true);

    m.emplace (1, "new e1");
    ASSERT_EQ (m.find (1, value), true);
    ASSERT_EQ (value == std::string ("e1"), true);
}

TEST (striped_map, replace)
{
    libcpp::striped_map<int, std::string> m{
      [] (const int &key) -> int { return key % 10; }, 10};
    m.emplace (1, "e1");
    m.emplace (11, "e11");
    m.emplace (123, "e123");
    std::string value;
    ASSERT_EQ (m.find (1, value), true);
    ASSERT_EQ (value == std::string ("e1"), true);

    m.replace (1, "new e1");
    std::string new_value;
    ASSERT_EQ (m.find (1, new_value), true);
    ASSERT_EQ (new_value == std::string ("new e1"), true);
}

TEST (striped_map, find)
{
    libcpp::striped_map<int, std::string> m{
      [] (const int &key) -> int { return key % 10; }, 10};
    m.emplace (1, "e1");
    m.emplace (11, "e11");
    m.emplace (123, "e123");
    std::string value;
    ASSERT_EQ (m.find (1, value), true);
    ASSERT_EQ (value == std::string ("e1"), true);
    ASSERT_EQ (m.find (11, value), true);
    ASSERT_EQ (value == std::string ("e11"), true);
    ASSERT_EQ (m.find (123, value), true);
    ASSERT_EQ (value == std::string ("e123"), true);
}

TEST (striped_map, erase)
{
    libcpp::striped_map<int, std::string> m{
      [] (const int &key) -> int { return key % 10; }, 10};
    m.emplace (1, "e1");
    m.emplace (11, "e11");
    m.emplace (123, "e123");
    ASSERT_EQ (m.erase (11), true);
    std::string value;
    ASSERT_EQ (m.find (11, value), false);
}

TEST (striped_map, range)
{
    libcpp::striped_map<int, std::string> m{
      [] (const int &key) -> int { return key % 10; }, 10};
    m.emplace (1, "e1");
    m.emplace (11, "e11");
    m.emplace (123, "e123");
    bool equal = false;
    m.range ([&] (const int &key, const std::string &value) -> bool {
        if (key == 1)
            equal = (value == std::string ("e1"));
        if (key == 11)
            equal = (value == std::string ("e11"));
        if (key == 123)
            equal = (value == std::string ("e123"));
        return equal;
    });
    ASSERT_EQ (equal, true);
}

TEST (striped_map, size)
{
}