#include <gtest/gtest.h>
#include <hj/testing/debugger.hpp>

#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include <set>
#include <boost/asio.hpp>

class DebuggerTest : public ::testing::Test
{
  protected:
    void TearDown() override { hj::debugger::instance().reset_ostream(); }
};

TEST_F(DebuggerTest, empty_vector)
{
    std::vector<uint8_t> empty_buf;
    std::string out = hj::debugger::instance().fmt("{:02x}", empty_buf);
    EXPECT_EQ(out, "");
}

TEST_F(DebuggerTest, empty_streambuf)
{
    boost::asio::streambuf empty_buf;
    std::string out = hj::debugger::instance().fmt("{:02x}", empty_buf);
    EXPECT_EQ(out, "");
}

TEST_F(DebuggerTest, null_char_ptr)
{
    const char *p   = nullptr;
    std::string out = hj::debugger::instance().fmt("{:02x}", p);
    EXPECT_EQ(out, "<null>");
}

TEST_F(DebuggerTest, null_bytes_view)
{
    hj::bytes_view view(nullptr, 100);
    std::string    out = hj::debugger::instance().fmt("{:02x}", view);
    EXPECT_EQ(out, "<null>");
}

TEST_F(DebuggerTest, exact_4096_bytes)
{
    std::vector<uint8_t> buf(hj::debugger::buf_sz, 0xAB);
    std::string          out = hj::debugger::instance().fmt("{:02x}", buf);
    EXPECT_EQ(out.find("..."), std::string::npos);
}

TEST_F(DebuggerTest, exact_4097_bytes)
{
    std::vector<uint8_t> buf(hj::debugger::buf_sz + 1, 0xAB);
    std::string          out = hj::debugger::instance().fmt("{:02x}", buf);
    EXPECT_NE(out.find("..."), std::string::npos);
}

TEST_F(DebuggerTest, ultra_large_buffer)
{
    std::vector<uint8_t> huge_buf(1'000'000, 0x55);
    std::string          out = hj::debugger::instance().fmt("{:02x}", huge_buf);

    EXPECT_NE(out.find("55"), std::string::npos);
    EXPECT_NE(out.find("..."), std::string::npos);
    EXPECT_LT(out.size(), hj::debugger::buf_sz * 4);
}

TEST_F(DebuggerTest, concurrent_print)
{
    std::ostringstream oss;
    {
        hj::ostream_guard guard(oss);

        constexpr int            thread_count     = 50;
        constexpr int            print_per_thread = 200;
        std::vector<std::thread> threads;
        threads.reserve(thread_count);

        for(int i = 0; i < thread_count; ++i)
        {
            threads.emplace_back([i, print_per_thread]() {
                for(int j = 0; j < print_per_thread; ++j)
                {
                    hj::debugger::instance().print("[TAG_{:03d}_{:04d}]", i, j);
                }
            });
        }

        for(auto &t : threads)
        {
            t.join();
        }
    }

    std::istringstream    iss(oss.str());
    std::string           line;
    int                   total_lines = 0;
    std::set<std::string> unique_lines;

    while(std::getline(iss, line))
    {
        if(!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        if(line.empty())
            continue;

        ++total_lines;

        EXPECT_EQ(line.front(), '[');
        EXPECT_EQ(line.back(), ']');
        EXPECT_EQ(line.size(), 14);

        unique_lines.insert(line);
    }

    EXPECT_EQ(total_lines, 50 * 200);
    EXPECT_EQ(unique_lines.size(), 50 * 200);
}

TEST_F(DebuggerTest, ostream_guard_scope_isolation)
{
    std::ostringstream local_oss;
    {
        hj::ostream_guard guard(local_oss);
        hj::debugger::instance().print("Inside Scope");
    }

    EXPECT_NE(local_oss.str().find("Inside Scope"), std::string::npos);

    std::ostringstream dummy_oss;
    hj::debugger::instance().set_ostream(dummy_oss);
    hj::debugger::instance().print("Outside Scope");

    EXPECT_EQ(local_oss.str().find("Outside Scope"), std::string::npos);
    EXPECT_NE(dummy_oss.str().find("Outside Scope"), std::string::npos);
}