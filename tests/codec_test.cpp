#include <gtest/gtest.h>
#include <libcpp/net/comm/codec.hpp>

unsigned long encode_str(const void* obj, unsigned char* data, const unsigned long len)
{
    auto str = reinterpret_cast<const std::string*>(obj);
    if (len < str->size())
        return 0;

    memcpy(data, reinterpret_cast<const unsigned char*>(str->data()), str->size());
    return str->size();
}

unsigned long decode_str(const unsigned char* data, const unsigned long len, void* obj)
{
    auto str = reinterpret_cast<std::string*>(obj);
    if (len < str->size())
        return 0;

    str->resize(len);
    memcpy(const_cast<char*>(str->data()), data, len);
    return str->size();
}

TEST(codec, size)
{
    libcpp::codec coder1{0};
    ASSERT_EQ(coder1.size(), 1);

    libcpp::codec coder2{1};
    ASSERT_EQ(coder2.size(), 1);

    libcpp::codec coder3{};
    ASSERT_EQ(coder3.size() > 0, true);
}

TEST(codec, set_encode_handler)
{
    libcpp::codec coder{1};
    std::string str{"hello"};
    unsigned char buf[1024];
    unsigned long buf_sz = 1024;
    auto ret1 = coder.encode(str, buf, buf_sz);
    ASSERT_EQ(ret1.valid(), false);

    coder.set_encode_handler(std::bind(encode_str, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    auto ret2 = coder.encode(str, buf, buf_sz);
    ASSERT_EQ(ret2.valid(), true);
    ASSERT_EQ(ret2.get(), 5);
}

TEST(codec, encode)
{
    libcpp::codec coder{1};
    coder.set_encode_handler(std::bind(encode_str, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    std::string str{"hello"};
    unsigned char buf[1024];
    unsigned long buf_sz = 1024;
    auto ret = coder.encode(str, buf, buf_sz);
    ASSERT_EQ(ret.get(), 5);
}

TEST(codec, bind_core_)
{
    libcpp::codec coder{ {1, 2, 3} };
    coder.set_encode_handler(std::bind(encode_str, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    std::string str{ "hello" };
    unsigned char buf[1024];
    unsigned long buf_sz = 1024;
    auto ret = coder.encode(str, buf, buf_sz);
    ASSERT_EQ(ret.get(), 5);
}

TEST(codec, decode)
{
    libcpp::codec coder{1};
    coder.set_decode_handler(std::bind(decode_str, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    std::string str{};
    unsigned char buf[] = {'w', 'o', 'r', 'l', 'd'};
    unsigned long buf_sz = 5;
    auto ret = coder.decode(buf, buf_sz, str);
    ASSERT_EQ(ret.get(), 5);
    ASSERT_STREQ(str.c_str(), "world");
}

TEST(codec, set_decode_handler)
{
    libcpp::codec coder{ 1 };
    std::string str{};
    unsigned char buf[] = { 'w', 'o', 'r', 'l', 'd' };
    unsigned long buf_sz = 5;
    auto ret1 = coder.decode(buf, buf_sz, str);
    ASSERT_EQ(ret1.valid(), false);

    coder.set_decode_handler(std::bind(decode_str, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    auto ret2 = coder.decode(buf, buf_sz, str);
    ASSERT_EQ(ret2.valid(), true);
    ASSERT_EQ(ret2.get(), 5);
    ASSERT_STREQ(str.c_str(), "world");
}