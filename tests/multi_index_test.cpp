#include <gtest/gtest.h>
#include <hj/algo/multi_index.hpp>
#include <string>

HJ_INDEX_TAG(id_tag)
HJ_INDEX_TAG(name_tag)

struct person
{
    int         id;
    std::string name;
    int         age;
};

using person_index =
    hj::multi_index<person,
                    HJ_UNIQUE_INDEX(int, &person::id, id_tag)
                        HJ_NON_UNIQUE_INDEX(
                            std::string, &person::name, name_tag)>;

TEST(multi_index, basic_insert_find)
{
    person_index idx;
    idx.insert({1, "Tom", 20});
    idx.insert({2, "Jerry", 22});
    idx.insert({3, "Tom", 25});

    auto &id_view = idx.get<id_tag>();
    auto  it      = id_view.find(2);
    ASSERT_NE(it, id_view.end());
    EXPECT_EQ(it->name, "Jerry");
    EXPECT_EQ(it->age, 22);

    auto &name_view = idx.get<name_tag>();
    auto  range     = name_view.equal_range("Tom");
    int   count     = 0;
    for(auto itr = range.first; itr != range.second; ++itr)
    {
        EXPECT_EQ(itr->name, "Tom");
        ++count;
    }
    EXPECT_EQ(count, 2);
}

TEST(multi_index, unique_constraint)
{
    person_index idx;
    idx.insert({1, "A", 10});
    auto res = idx.insert({1, "B", 20});
    EXPECT_FALSE(res.second);
}

TEST(multi_index, erase_and_modify)
{
    person_index idx;
    idx.insert({1, "A", 10});
    idx.insert({2, "B", 20});
    auto &id_view = idx.get<id_tag>();
    id_view.erase(1);
    EXPECT_EQ(id_view.size(), 1);
    auto it = id_view.find(2);
    ASSERT_NE(it, id_view.end());
    id_view.modify(it, [](person &p) { p.age = 99; });
    EXPECT_EQ(it->age, 99);
}
