#include <gtest/gtest.h>
#include <hj/algo/multi_index.hpp>
#include <boost/tuple/tuple.hpp>
#include <string>
#include <type_traits>
#include <vector>

// 1. 定义测试用 Tag
HJ_INDEX_TAG(id_tag);
HJ_INDEX_TAG(name_tag);
HJ_INDEX_TAG(code_tag);
HJ_INDEX_TAG(score_tag);
HJ_INDEX_TAG(dept_name_tag);

// 2. 定义测试用数据结构
struct item
{
    int         id;
    std::string name;
    std::string code;
    double      score;
    std::string dept;
};

// 3. 容器定义：ordered_unique / ordered_non_unique / hashed_unique /
//    hashed_non_unique / composite(ordered_unique on two fields)
//
// NOTE: macro argument order is now (tag, member_ptr...) — this changed
// from the previous (member_ptr, tag) to support composite keys.
using item_container =
    hj::multi_index<item,
                    HJ_UNIQUE_INDEX(id_tag, &item::id),
                    HJ_NON_UNIQUE_INDEX(name_tag, &item::name),
                    HJ_HASHED_UNIQUE_INDEX(code_tag, &item::code),
                    HJ_HASHED_NON_UNIQUE_INDEX(score_tag, &item::score),
                    HJ_UNIQUE_INDEX(dept_name_tag, &item::dept, &item::name)>;

// ============================================================================
// 元编程与类型萃取（Compile-time Unit Tests）
// ============================================================================

TEST(multi_index_meta, member_pointer_traits)
{
    using traits = hj::detail::member_pointer_traits<decltype(&item::id)>;
    static_assert(std::is_same_v<traits::class_type, item>,
                  "Class type mismatch");
    static_assert(std::is_same_v<traits::member_type, int>,
                  "Member type mismatch");

    using name_traits =
        hj::detail::member_pointer_traits<decltype(&item::name)>;
    static_assert(std::is_same_v<name_traits::member_type, std::string>,
                  "Member type mismatch");
}

TEST(multi_index_meta, index_config_properties)
{
    using unique_cfg = hj::unique_index<id_tag, &item::id>;
    EXPECT_EQ(unique_cfg::kind, hj::detail::index_kind::ordered_unique);
    EXPECT_EQ(unique_cfg::member_ptr, &item::id);

    using nonunique_cfg = hj::non_unique_index<name_tag, &item::name>;
    EXPECT_EQ(nonunique_cfg::kind, hj::detail::index_kind::ordered_non_unique);

    using hashed_cfg = hj::hashed_unique_index<code_tag, &item::code>;
    EXPECT_EQ(hashed_cfg::kind, hj::detail::index_kind::hashed_unique);

    using hashed_nonunique_cfg =
        hj::hashed_non_unique_index<score_tag, &item::score>;
    EXPECT_EQ(hashed_nonunique_cfg::kind,
              hj::detail::index_kind::hashed_non_unique);
}

TEST(multi_index_meta, composite_key_extractor_selection)
{
    // single member pointer -> plain member<> extractor
    using single_extractor = hj::detail::key_extractor<item, &item::id>;
    static_assert(single_extractor::count == 1, "count mismatch");

    // two member pointers -> composite_key<> extractor
    using composite_extractor =
        hj::detail::key_extractor<item, &item::dept, &item::name>;
    static_assert(composite_extractor::count == 2, "count mismatch");
    static_assert(
        !std::is_same_v<composite_extractor::type, single_extractor::type>,
        "single- and multi-field extractors must differ");
}

// ============================================================================
// 运行期功能与边界条件测试
// ============================================================================

// 1. 基础插入与多索引查找测试
TEST(multi_index, basic_insert_and_lookup)
{
    item_container container;

    auto [it1, inserted1] = container.insert({1, "Apple", "A01", 9.5, "Sales"});
    EXPECT_TRUE(inserted1);
    EXPECT_EQ(it1->id, 1);

    auto [it2, inserted2] =
        container.insert({2, "Banana", "B01", 8.0, "Sales"});
    EXPECT_TRUE(inserted2);

    // 有序唯一索引查找 (ordered_unique)
    auto &id_view = container.get<id_tag>();
    auto  id_it   = id_view.find(1);
    ASSERT_NE(id_it, id_view.end());
    EXPECT_EQ(id_it->name, "Apple");

    // 哈希唯一索引查找 (hashed_unique)
    auto &code_view = container.get<code_tag>();
    auto  code_it   = code_view.find("B01");
    ASSERT_NE(code_it, code_view.end());
    EXPECT_EQ(code_it->id, 2);

    // 哈希非唯一索引查找 (hashed_non_unique)
    auto &score_view = container.get<score_tag>();
    EXPECT_EQ(score_view.count(9.5), 1u);
}

// 2. 约束条件测试（唯一性与非唯一性冲突）
TEST(multi_index, unique_and_nonunique_constraints)
{
    item_container container;
    container.insert({1, "Apple", "A01", 9.0, "Sales"});

    // 2.1 主键(ordered_unique)重复插入失败
    auto res1 = container.insert({1, "Orange", "A02", 7.0, "IT"});
    EXPECT_FALSE(res1.second);
    EXPECT_EQ(container.size(), 1u);

    // 2.2 哈希键(hashed_unique)重复插入失败
    auto res2 = container.insert({2, "Orange", "A01", 7.0, "IT"});
    EXPECT_FALSE(res2.second);
    EXPECT_EQ(container.size(), 1u);

    // 2.3 非唯一索引(ordered_non_unique)允许重复插入
    auto res3 = container.insert({2, "Apple", "A02", 8.0, "IT"});
    EXPECT_TRUE(res3.second);
    EXPECT_EQ(container.size(), 2u);

    // 验证 nonunique 范围检索
    auto &name_view = container.get<name_tag>();
    EXPECT_EQ(name_view.count("Apple"), 2u);
}

// 3. 删除与修改（Modify）边界测试
TEST(multi_index, erase_and_modify_operations)
{
    item_container container;
    container.insert({1, "Apple", "A01", 9.0, "Sales"});
    container.insert({2, "Banana", "B01", 8.0, "Sales"});
    container.insert({3, "Cherry", "C01", 8.5, "IT"});

    auto &id_view = container.get<id_tag>();

    // 3.1 删除存在的元素
    size_t erased_count = id_view.erase(2);
    EXPECT_EQ(erased_count, 1u);
    EXPECT_EQ(container.size(), 2u);
    EXPECT_EQ(id_view.find(2), id_view.end());

    // 3.2 删除不存在的元素（边界条件）
    erased_count = id_view.erase(999);
    EXPECT_EQ(erased_count, 0u);

    // 3.3 安全修改非索引或不打破唯一性的字段
    auto it_modify = id_view.find(1);
    ASSERT_NE(it_modify, id_view.end());
    bool modified = id_view.modify(it_modify, [](item &i) { i.score = 9.9; });
    EXPECT_TRUE(modified);
    EXPECT_EQ(it_modify->score, 9.9);

    // 3.4 修改引发索引冲突（边界条件：尝试将 code 改为已存在的 "C01"）
    auto it_conflict = id_view.find(1);
    bool fail_modify =
        id_view.modify(it_conflict, [](item &i) { i.code = "C01"; });
    // modify 失败时元素会被自动移除，容器大小改变（Boost Multi-Index 默认行为）
    EXPECT_FALSE(fail_modify);
    EXPECT_EQ(container.size(), 1u);
}

// 4. 边界条件：空容器与不存在值的查找
TEST(multi_index, empty_and_non_existent_lookup)
{
    item_container container;
    EXPECT_TRUE(container.empty());
    EXPECT_EQ(container.size(), 0u);

    auto &id_view = container.get<id_tag>();
    EXPECT_EQ(id_view.find(1), id_view.end());

    auto &code_view = container.get<code_tag>();
    EXPECT_EQ(code_view.find("NOT_FOUND"), code_view.end());

    auto &name_view = container.get<name_tag>();
    auto  range     = name_view.equal_range("NOT_FOUND");
    EXPECT_EQ(range.first, range.second);
}

// 5. 排序特性验证（ordered_unique 按 key 自动排序，含范围查询）
TEST(multi_index, ordered_sorting_and_range_query)
{
    item_container container;
    container.insert({3, "C", "03", 1.0, "X"});
    container.insert({1, "A", "01", 1.0, "X"});
    container.insert({4, "D", "04", 1.0, "X"});
    container.insert({2, "B", "02", 1.0, "X"});

    auto            &id_view      = container.get<id_tag>();
    std::vector<int> expected_ids = {1, 2, 3, 4};
    std::vector<int> actual_ids;
    for(const auto &it : id_view)
        actual_ids.push_back(it.id);
    EXPECT_EQ(actual_ids, expected_ids);

    auto             lower = id_view.lower_bound(2);
    auto             upper = id_view.upper_bound(3);
    std::vector<int> range_ids;
    for(auto it = lower; it != upper; ++it)
        range_ids.push_back(it->id);
    std::vector<int> expected_range = {2, 3};
    EXPECT_EQ(range_ids, expected_range);
}

// 6. 组合键（composite_key）测试：按 (dept, name) 联合唯一
TEST(multi_index, composite_key_unique_constraint)
{
    item_container container;
    container.insert({1, "Alice", "A01", 9.0, "Sales"});
    container.insert({2, "Alice", "A02", 8.0, "IT"}); // 同名不同部门，允许

    auto &dept_name_view = container.get<dept_name_tag>();

    // 组合键查询需要用 boost::make_tuple 按索引字段顺序传入
    auto it = dept_name_view.find(
        boost::make_tuple(std::string("Sales"), std::string("Alice")));
    ASSERT_NE(it, dept_name_view.end());
    EXPECT_EQ(it->id, 1);

    // 同一 (dept, name) 组合重复插入应失败
    auto res = container.insert({3, "Alice", "A03", 7.0, "Sales"});
    EXPECT_FALSE(res.second);
    EXPECT_EQ(container.size(), 2u);

    // 不存在的组合查找不到
    auto missing = dept_name_view.find(
        boost::make_tuple(std::string("HR"), std::string("Alice")));
    EXPECT_EQ(missing, dept_name_view.end());
}

// 7. hashed_non_unique 独立行为验证（此前版本声明了但从未实现/测试）
TEST(multi_index, hashed_non_unique_allows_duplicates)
{
    item_container container;
    container.insert({1, "Apple", "A01", 9.0, "Sales"});
    container.insert({2, "Banana", "B01", 9.0, "IT"}); // 分数重复，允许
    container.insert({3, "Cherry", "C01", 7.5, "IT"});

    auto &score_view = container.get<score_tag>();
    EXPECT_EQ(score_view.count(9.0), 2u);
    EXPECT_EQ(score_view.count(7.5), 1u);
    EXPECT_EQ(score_view.count(100.0), 0u);
}