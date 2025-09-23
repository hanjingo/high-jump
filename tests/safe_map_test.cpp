#include <gtest/gtest.h>
#include <hj/sync/safe_map.hpp>

TEST(safe_map, insert)
{
    hj::safe_map<int, std::string> map;
    ASSERT_EQ(map.insert(1, "one"), true);
    ASSERT_EQ(map.insert(1, "one"), false);

    ASSERT_EQ(map.insert(2, "two"), true);
    ASSERT_EQ(map.insert(3, "three"), true);
    ASSERT_EQ(map.insert(4, "four"), true);
    ASSERT_EQ(map.insert(5, "five"), true);
    ASSERT_EQ(map.insert(6, "six"), true);
    ASSERT_EQ(map.insert(7, "seven"), true);
    ASSERT_EQ(map.insert(8, "eight"), true);
    ASSERT_EQ(map.insert(9, "nine"), true);
    ASSERT_EQ(map.insert(10, "ten"), true);
}

TEST(safe_map, emplace)
{
    hj::safe_map<int, std::string> map;
    ASSERT_EQ(map.emplace(1, "one"), true);
    ASSERT_EQ(map.emplace(1, "one"), false);

    ASSERT_EQ(map.emplace(2, "two"), true);
    ASSERT_EQ(map.emplace(3, "three"), true);
    ASSERT_EQ(map.emplace(4, "four"), true);
    ASSERT_EQ(map.emplace(5, "five"), true);
    ASSERT_EQ(map.emplace(6, "six"), true);
    ASSERT_EQ(map.emplace(7, "seven"), true);
    ASSERT_EQ(map.emplace(8, "eight"), true);
    ASSERT_EQ(map.emplace(9, "nine"), true);
    ASSERT_EQ(map.emplace(10, "ten"), true);
}
TEST(safe_map, replace)
{
    hj::safe_map<int, std::string> map;
    ASSERT_EQ(map.emplace(1, "one"), true);
    ASSERT_EQ(map.emplace(2, "two"), true);
    ASSERT_EQ(map.emplace(3, "three"), true);

    std::string old_value;
    map.replace(1, "uno", old_value);
    ASSERT_EQ(old_value, "one");
    map.replace(2, "dos", old_value);
    ASSERT_EQ(old_value, "two");
    map.replace(3, "tres", old_value);
    ASSERT_EQ(old_value, "three");
}
TEST(safe_map, find)
{
    hj::safe_map<int, std::string> map;
    ASSERT_EQ(map.emplace(1, "one"), true);
    ASSERT_EQ(map.emplace(2, "two"), true);
    ASSERT_EQ(map.emplace(3, "three"), true);
    ASSERT_EQ(map.emplace(4, "four"), true);
    ASSERT_EQ(map.emplace(5, "five"), true);
    ASSERT_EQ(map.emplace(6, "six"), true);
    ASSERT_EQ(map.emplace(7, "seven"), true);
    ASSERT_EQ(map.emplace(8, "eight"), true);
    ASSERT_EQ(map.emplace(9, "nine"), true);
    ASSERT_EQ(map.emplace(10, "ten"), true);

    std::string value;
    ASSERT_EQ(map.find(1, value), true);
    ASSERT_EQ(value, "one");
}
TEST(safe_map, erase)
{
    hj::safe_map<int, std::string> map;
    ASSERT_EQ(map.emplace(1, "one"), true);
    ASSERT_EQ(map.emplace(2, "two"), true);
    ASSERT_EQ(map.emplace(3, "three"), true);
    ASSERT_EQ(map.emplace(4, "four"), true);
    ASSERT_EQ(map.emplace(5, "five"), true);
    ASSERT_EQ(map.emplace(6, "six"), true);
    ASSERT_EQ(map.emplace(7, "seven"), true);
    ASSERT_EQ(map.emplace(8, "eight"), true);
    ASSERT_EQ(map.emplace(9, "nine"), true);
    ASSERT_EQ(map.emplace(10, "ten"), true);

    ASSERT_EQ(map.erase(1), true);
}
TEST(safe_map, range)
{
    hj::safe_map<int, std::string> map;
    ASSERT_EQ(map.emplace(1, "one"), true);
    ASSERT_EQ(map.emplace(2, "two"), true);
    ASSERT_EQ(map.emplace(3, "three"), true);
    ASSERT_EQ(map.emplace(4, "four"), true);
    ASSERT_EQ(map.emplace(5, "five"), true);
    ASSERT_EQ(map.emplace(6, "six"), true);
    ASSERT_EQ(map.emplace(7, "seven"), true);
    ASSERT_EQ(map.emplace(8, "eight"), true);
    ASSERT_EQ(map.emplace(9, "nine"), true);
    ASSERT_EQ(map.emplace(10, "ten"), true);

    map.range([](const int& key, std::string& value) {
        (void) key;
        (void) value;
        return true;
    });
}
TEST(safe_map, count)
{
    hj::safe_map<int, std::string> map;
    ASSERT_EQ(map.emplace(1, "one"), true);
    ASSERT_EQ(map.emplace(2, "two"), true);
    ASSERT_EQ(map.emplace(3, "three"), true);
    ASSERT_EQ(map.emplace(4, "four"), true);
    ASSERT_EQ(map.emplace(5, "five"), true);
    ASSERT_EQ(map.emplace(6, "six"), true);
    ASSERT_EQ(map.emplace(7, "seven"), true);
    ASSERT_EQ(map.emplace(8, "eight"), true);
    ASSERT_EQ(map.emplace(9, "nine"), true);
    ASSERT_EQ(map.emplace(10, "ten"), true);

    ASSERT_EQ(map.count(1), 1);
}
TEST(safe_map, size)
{
    hj::safe_map<int, std::string> map;
    ASSERT_EQ(map.emplace(1, "one"), true);
    ASSERT_EQ(map.emplace(2, "two"), true);
    ASSERT_EQ(map.emplace(3, "three"), true);
    ASSERT_EQ(map.emplace(4, "four"), true);
    ASSERT_EQ(map.emplace(5, "five"), true);
    ASSERT_EQ(map.emplace(6, "six"), true);
    ASSERT_EQ(map.emplace(7, "seven"), true);
    ASSERT_EQ(map.emplace(8, "eight"), true);
    ASSERT_EQ(map.emplace(9, "nine"), true);
    ASSERT_EQ(map.emplace(10, "ten"), true);

    ASSERT_EQ(map.size(), 10);
}
TEST(safe_map, empty)
{
    hj::safe_map<int, std::string> map;
    ASSERT_EQ(map.empty(), true);
    ASSERT_EQ(map.emplace(1, "one"), true);
    ASSERT_EQ(map.empty(), false);
}
TEST(safe_map, clear)
{
    hj::safe_map<int, std::string> map;
    ASSERT_EQ(map.emplace(1, "one"), true);
    ASSERT_EQ(map.emplace(2, "two"), true);
    ASSERT_EQ(map.emplace(3, "three"), true);
    ASSERT_EQ(map.emplace(4, "four"), true);
    ASSERT_EQ(map.emplace(5, "five"), true);
    ASSERT_EQ(map.emplace(6, "six"), true);
    ASSERT_EQ(map.emplace(7, "seven"), true);
    ASSERT_EQ(map.emplace(8, "eight"), true);
    ASSERT_EQ(map.emplace(9, "nine"), true);
    ASSERT_EQ(map.emplace(10, "ten"), true);

    map.clear();
}
TEST(safe_map, swap)
{
    hj::safe_map<int, std::string> map1;
    hj::safe_map<int, std::string> map2;
    ASSERT_EQ(map1.emplace(1, "one"), true);
    ASSERT_EQ(map1.emplace(2, "two"), true);
    ASSERT_EQ(map1.emplace(3, "three"), true);
    ASSERT_EQ(map1.emplace(4, "four"), true);
    ASSERT_EQ(map1.emplace(5, "five"), true);
    ASSERT_EQ(map1.emplace(6, "six"), true);
    ASSERT_EQ(map1.emplace(7, "seven"), true);
    ASSERT_EQ(map1.emplace(8, "eight"), true);
    ASSERT_EQ(map1.emplace(9, "nine"), true);
    ASSERT_EQ(map1.emplace(10, "ten"), true);

    map2.swap(map1);
}
TEST(safe_map, swap2)
{
    hj::safe_map<int, std::string> map1;
    hj::safe_map<int, std::string> map2;
    ASSERT_EQ(map1.emplace(1, "one"), true);
    ASSERT_EQ(map1.emplace(2, "two"), true);
    ASSERT_EQ(map1.emplace(3, "three"), true);
    ASSERT_EQ(map1.emplace(4, "four"), true);
    ASSERT_EQ(map1.emplace(5, "five"), true);
    ASSERT_EQ(map1.emplace(6, "six"), true);
    ASSERT_EQ(map1.emplace(7, "seven"), true);
    ASSERT_EQ(map1.emplace(8, "eight"), true);
    ASSERT_EQ(map1.emplace(9, "nine"), true);
    ASSERT_EQ(map1.emplace(10, "ten"), true);

    map2.swap(std::move(map1));
}