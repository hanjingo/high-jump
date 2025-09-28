#include <gtest/gtest.h>
#include <hj/types/reflect.hpp>

struct TestStruct
{
    int    a;
    double b;
    char   c;
};

TEST(reflect, type_name)
{
    ASSERT_NE(hj::reflect::type_name(int(1)).find("int"), std::string::npos);
    ASSERT_NE(hj::reflect::type_name(TestStruct()).find("TestStruct"),
              std::string::npos);
}

TEST(reflect, is_pod)
{
    ASSERT_TRUE(hj::reflect::is_pod(TestStruct()));
    ASSERT_TRUE(hj::reflect::is_pod(int(1)));
}

TEST(reflect, copy_clone)
{
    TestStruct t1{1, 2.5, 'x'};
    auto       t2 = hj::reflect::copy(t1);
    auto       t3 = hj::reflect::clone(t1);
    ASSERT_EQ(t2.a, 1);
    ASSERT_DOUBLE_EQ(t2.b, 2.5);
    ASSERT_EQ(t2.c, 'x');
    ASSERT_EQ(t3.a, 1);
    ASSERT_DOUBLE_EQ(t3.b, 2.5);
    ASSERT_EQ(t3.c, 'x');
}

TEST(reflect, serialize_unserialize)
{
    TestStruct t1{42, 3.14, 'z'};
    auto       buf = hj::reflect::serialize(t1);
    auto       t2  = hj::reflect::unserialize<TestStruct>(buf.data());
    ASSERT_EQ(t2.a, 42);
    ASSERT_DOUBLE_EQ(t2.b, 3.14);
    ASSERT_EQ(t2.c, 'z');
}

TEST(reflect, offset_size_align_of)
{
    ASSERT_EQ(hj::reflect::offset_of(&TestStruct::a),
              static_cast<std::size_t>(offsetof(TestStruct, a)));
    ASSERT_EQ(hj::reflect::size_of(&TestStruct::a),
              sizeof(((TestStruct *) 0)->a));
    // ASSERT_GE(hj::reflect::align_of(&TestStruct::a), alignof(((TestStruct*)0)->a));
}

TEST(reflect, is_same_type)
{
    int    a = 1;
    double b = 2.0;
    char   c = 'x';
    ASSERT_TRUE(hj::reflect::is_same_type(a, int(2)));
    ASSERT_FALSE(hj::reflect::is_same_type(a, b));
    ASSERT_TRUE(hj::reflect::is_same_type(a, int(2), int(3)));
    ASSERT_FALSE(hj::reflect::is_same_type(a, b, c));
    ASSERT_TRUE(hj::reflect::is_same_type(a, int(2), int(3), int(4)));
    ASSERT_TRUE(hj::reflect::is_same_type(a, int(2), int(3), int(4), int(5)));
    ASSERT_TRUE(
        hj::reflect::is_same_type(a, int(2), int(3), int(4), int(5), int(6)));
    ASSERT_TRUE(hj::reflect::is_same_type(a,
                                          int(2),
                                          int(3),
                                          int(4),
                                          int(5),
                                          int(6),
                                          int(7)));
}
