#include <gtest/gtest.h>
#include <hj/types/reflect.hpp>
#include <vector>
#include <string>

struct TestStruct
{
    int    a;
    double b;
    char   c;
};

enum class Color : int
{
    Red,
    Green,
    Blue
};
enum OldEnum
{
    Val1,
    Val2
};

union MyUnion
{
    int   i;
    float f;
};

struct NonTrivialStruct
{
    std::string s;
    ~NonTrivialStruct() {}
};

class NonStandardLayoutBase
{
  public:
    int x;
};
class NonStandardLayoutDerived : public NonStandardLayoutBase
{
  public:
    int y;
};

struct VirtualStruct
{
    virtual ~VirtualStruct() = default;
    int x;
};

TEST(reflect, type_name)
{
    ASSERT_NE(hj::reflect::type_name(int(1)).find("int"), std::string::npos);
    ASSERT_NE(hj::reflect::type_name(TestStruct()).find("TestStruct"),
              std::string::npos);
}

TEST(reflect, type_category_matrix)
{
    EXPECT_TRUE(hj::reflect::is_simple(int(0)));
    EXPECT_TRUE(hj::reflect::is_simple(double(0.0)));
    EXPECT_TRUE(hj::reflect::is_simple(char('a')));
    EXPECT_TRUE(hj::reflect::is_simple(Color::Red));
    EXPECT_TRUE(hj::reflect::is_simple(OldEnum::Val1));
    EXPECT_TRUE(hj::reflect::is_simple(MyUnion{0}));
    EXPECT_TRUE(hj::reflect::is_simple(TestStruct{}));

    int arr[5] = {1, 2, 3, 4, 5};
    EXPECT_TRUE(hj::reflect::is_simple(arr));

    int *ptr = nullptr;
    EXPECT_TRUE(hj::reflect::is_simple(ptr));

    EXPECT_FALSE(hj::reflect::is_simple(NonTrivialStruct{"test"}));
    EXPECT_FALSE(hj::reflect::is_simple(VirtualStruct{}));
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

TEST(reflect, serialize_unserialize_happy_path)
{
    TestStruct t1{42, 3.14, 'z'};
    auto       bytes = hj::reflect::dump_binary(t1);
    auto t2 = hj::reflect::load_binary<TestStruct>(bytes.data(), bytes.size());
    ASSERT_EQ(t2.a, 42);
    ASSERT_DOUBLE_EQ(t2.b, 3.14);
    ASSERT_EQ(t2.c, 'z');
}

TEST(reflect, load_binary_negative_tests)
{
    TestStruct t1{100, 2.718, 'k'};
    auto       bytes = hj::reflect::dump_binary(t1);

    EXPECT_THROW(hj::reflect::load_binary<TestStruct>(
                     static_cast<const std::byte *>(nullptr),
                     bytes.size()),
                 std::invalid_argument);
    EXPECT_THROW(hj::reflect::load_binary<TestStruct>(
                     static_cast<const unsigned char *>(nullptr),
                     bytes.size()),
                 std::invalid_argument);

    EXPECT_THROW(hj::reflect::load_binary<TestStruct>(bytes.data(),
                                                      sizeof(TestStruct) - 1),
                 std::invalid_argument);
    EXPECT_THROW(hj::reflect::load_binary<TestStruct>(bytes.data(), 0),
                 std::invalid_argument);

    auto t_exact =
        hj::reflect::load_binary<TestStruct>(bytes.data(), sizeof(TestStruct));
    ASSERT_EQ(t_exact.a, 100);

    std::vector<std::byte> larger_buf(bytes.size() + 10, std::byte{0});
    std::memcpy(larger_buf.data(), bytes.data(), bytes.size());
    auto t_larger = hj::reflect::load_binary<TestStruct>(larger_buf.data(),
                                                         larger_buf.size());
    ASSERT_EQ(t_larger.a, 100);

    // static_assert(std::is_trivially_copyable_v<NonTrivialStruct> == false);
}

TEST(reflect, offset_size_align_of)
{
    ASSERT_EQ(hj::reflect::offset_of(&TestStruct::a),
              static_cast<std::size_t>(offsetof(TestStruct, a)));
    ASSERT_EQ(hj::reflect::size_of(&TestStruct::a),
              sizeof(((TestStruct *) 0)->a));
    ASSERT_EQ(hj::reflect::align_of(&TestStruct::a), alignof(int));
    ASSERT_EQ(hj::reflect::align_of(&TestStruct::b), alignof(double));
    ASSERT_EQ(hj::reflect::align_of(&TestStruct::c), alignof(char));
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