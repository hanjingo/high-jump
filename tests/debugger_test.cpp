
#include <gtest/gtest.h>
#include <hj/testing/debugger.hpp>
#include <vector>
#include <string>
#include <boost/asio.hpp>

#include <sstream>

TEST(debugger, fmt_vector)
{
    std::vector<uint8_t> buf = {0x12, 0x34, 0x56, 0x78};
    std::string          out = hj::debugger::instance().fmt("{:02x}", buf);
    EXPECT_NE(out.find("12"), std::string::npos);
    EXPECT_NE(out.find("34"), std::string::npos);
}

TEST(debugger, fmt_char_ptr)
{
    const char *buf = "ABCD";
    std::string out = hj::debugger::instance().fmt("{:02x}", buf);
    EXPECT_NE(out.find("41"), std::string::npos); // 'A' = 0x41
    EXPECT_NE(out.find("42"), std::string::npos); // 'B' = 0x42
}

TEST(debugger, fmt_unsigned_char_ptr)
{
    unsigned char buf[3] = {0xAA, 0xBB, 0xCC};
    std::string   out    = hj::debugger::instance().fmt("{:02x}", buf, 3);
    EXPECT_NE(out.find("aa"), std::string::npos);
    EXPECT_NE(out.find("bb"), std::string::npos);
    EXPECT_NE(out.find("cc"), std::string::npos);
}

TEST(debugger, fmt_streambuf)
{
    boost::asio::streambuf buf;
    std::ostream           os(&buf);
    os << "XYZ";
    std::string out = hj::debugger::instance().fmt("{:02x}", buf);
    EXPECT_NE(out.find("58"), std::string::npos); // 'X' = 0x58
    EXPECT_NE(out.find("59"), std::string::npos); // 'Y' = 0x59
    EXPECT_NE(out.find("5a"), std::string::npos); // 'Z' = 0x5A
}

TEST(debugger, fmt_bufsize_limit)
{
    std::vector<uint8_t> buf(DEBUG_BUF_SIZE + 10, 0xFF);
    std::string          out = hj::debugger::instance().fmt("{:02x}", buf);
    EXPECT_NE(out.find("..."), std::string::npos);
}

// TEST(debugger, print_vector)
// {
//     std::vector<uint8_t> buf = {0x12, 0x34, 0x56, 0x78};
//     std::ostringstream   oss;
//     hj::debugger::instance().set_ostream(oss);
//     PRINT("{:02x}", buf);
//     std::string out = oss.str();
//     EXPECT_NE(out.find("12"), std::string::npos);
//     EXPECT_NE(out.find("34"), std::string::npos);
// }

// TEST(debugger, print_char_ptr)
// {
//     const char        *buf = "ABCD";
//     std::ostringstream oss;
//     hj::debugger::instance().set_ostream(oss);
//     PRINT("{:02x}", buf);
//     std::string out = oss.str();
//     EXPECT_NE(out.find("41"), std::string::npos);
//     EXPECT_NE(out.find("42"), std::string::npos);
// }

// TEST(debugger, print_unsigned_char_ptr)
// {
//     unsigned char      buf[3] = {0xAA, 0xBB, 0xCC};
//     std::ostringstream oss;
//     hj::debugger::instance().set_ostream(oss);
//     PRINT("{:02x}", buf, 3);
//     std::string out = oss.str();
//     EXPECT_NE(out.find("aa"), std::string::npos);
//     EXPECT_NE(out.find("bb"), std::string::npos);
//     EXPECT_NE(out.find("cc"), std::string::npos);
// }

// TEST(debugger, print_streambuf)
// {
//     boost::asio::streambuf buf;
//     std::ostream           os(&buf);
//     os << "XYZ";
//     std::ostringstream oss;
//     hj::debugger::instance().set_ostream(oss);
//     PRINT("{:02x}", buf);
//     std::string out = oss.str();
//     EXPECT_NE(out.find("58"), std::string::npos);
//     EXPECT_NE(out.find("59"), std::string::npos);
//     EXPECT_NE(out.find("5a"), std::string::npos);
// }

// TEST(debugger, print_bufsize_limit)
// {
//     std::vector<uint8_t> buf(DEBUG_BUF_SIZE + 10, 0xFF);
//     std::ostringstream   oss;
//     hj::debugger::instance().set_ostream(oss);
//     PRINT("{:02x}", buf);
//     std::string out = oss.str();
//     EXPECT_NE(out.find("..."), std::string::npos);
// }
