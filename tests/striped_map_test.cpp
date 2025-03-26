#include <gtest/gtest.h>
#include <libcpp/sync/striped_map.hpp>
#include <string>

TEST(striped_map, insert)
{
    libcpp::striped_map<int, std::string> m{[](const int& key)->int{ return key % 10; }, 10};
    m.insert(1, "e1");
    m.insert(11, "e11");
    m.insert(123, "e123");
    std::string value;
    ASSERT_EQ(m.find(1, value), true);
    ASSERT_EQ(value == std::string("e1"), true);

    m.insert(11, "new e11");
    ASSERT_EQ(m.find(11, value), true);
    ASSERT_EQ(value == std::string("new e11"), true);
}

TEST(striped_map, find)
{
    libcpp::striped_map<int, std::string> m{[](const int& key)->int{ return key % 10; }, 10};
    m.insert(1, "e1");
    m.insert(11, "e11");
    m.insert(123, "e123");
    std::string value;
    ASSERT_EQ(m.find(1, value), true);
    ASSERT_EQ(value == std::string("e1"), true);

    m.insert(11, "new e11");
    ASSERT_EQ(m.find(11, value), true);
    ASSERT_EQ(value == std::string("new e11"), true);
}

TEST(striped_map, erase)
{
    libcpp::striped_map<int, std::string> m{[](const int& key)->int{ return key % 10; }, 10};
    m.insert(1, "e1");
    m.insert(11, "e11");
    m.insert(123, "e123");
    ASSERT_EQ(m.erase(11), true);
}

TEST(striped_map, range)
{
    libcpp::striped_map<int, std::string> m{[](const int& key)->int{ return key % 10; }, 10};
    m.insert(1, "e1");
    m.insert(11, "e11");
    m.insert(123, "e123");
    m.range([](const int& key, const std::string value)->bool{
        if (key == 1)
            ASSERT_EQ(value == std::string("e1"), true);
        if (key == 11)
            ASSERT_EQ(value == std::string("e11"), true);
        if (key == 123)
            ASSERT_EQ(value == std::string("e123"), true);
        return true;
    });
}

TEST(striped_map, size)
{
    
}