#include <gtest/gtest.h>
#include <hj/encoding/xml.hpp>
#include <string>
#include <cstdio>

using namespace hj;

TEST(XmlTest, LoadFromString) {
	const char* text = "<root><item>42</item></root>";
	xml x;
	ASSERT_TRUE(x.load(text));
	auto item = x.child("item");
	EXPECT_EQ(item.value(), "42");
}

TEST(XmlTest, LoadSaveFile) {
	const char* filename = "test.xml";
	xml x;
	x.load("<root><foo>bar</foo></root>");
	ASSERT_TRUE(x.save_file(filename));
	xml y;
	ASSERT_TRUE(y.load_file(filename));
	auto foo = y.child("foo");
	EXPECT_EQ(foo.value(), "bar");
	std::remove(filename);
}

TEST(XmlTest, NodeAndAttr) {
	xml x;
	x.load("<root></root>");
	auto child = x.append_child("item");
	child.set_value("abc");
	child.set_attr("id", "123");
	EXPECT_EQ(child.value(), "abc");
	EXPECT_EQ(child.attr("id"), "123");
	EXPECT_EQ(child.name(), "item");
	child.set_name("item2");
	EXPECT_EQ(child.name(), "item2");
}

