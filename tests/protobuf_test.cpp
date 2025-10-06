#include <gtest/gtest.h>
#include <hj/encoding/protobuf.hpp>
#include "person.pb.h"
#include <cstdio>
#include <string>

TEST(ProtobufTest, SerializeParseString)
{
    test::Person p;
    p.set_name("test");
    p.set_id(100);
    p.set_email("hehehunanchina@live.com");
    std::string buf;
    auto        serr = hj::pb::serialize(buf, p);
    ASSERT_EQ(serr, hj::pb::error_code::ok);
    test::Person p1;
    auto         derr = hj::pb::deserialize(p1, buf);
    ASSERT_EQ(derr, hj::pb::error_code::ok);
    EXPECT_EQ(p1.name(), "test");
    EXPECT_EQ(p1.email(), "hehehunanchina@live.com");
    EXPECT_EQ(p1.id(), 100);
}

TEST(ProtobufTest, SerializeParseFile)
{
    test::Person p;
    p.set_name("file");
    p.set_id(42);
    p.set_email("file@example.com");
    std::ofstream fout("person_test.pb", std::ios::binary);
    auto          serr = hj::pb::serialize(fout, p);
    ASSERT_EQ(serr, hj::pb::error_code::ok);
    fout.close();

    test::Person  p2;
    std::ifstream fin("person_test.pb", std::ios::binary);
    auto          derr = hj::pb::deserialize(p2, fin);
    ASSERT_EQ(derr, hj::pb::error_code::ok);
    EXPECT_EQ(p2.name(), "file");
    EXPECT_EQ(p2.id(), 42);
    EXPECT_EQ(p2.email(), "file@example.com");
    fin.close();
}