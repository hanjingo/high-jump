#include <gtest/gtest.h>
#include <hj/testing/debugger.hpp>

#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include <set>
#include <stdexcept>
#include <boost/asio.hpp>

class DebuggerTest : public ::testing::Test
{
  protected:
    void TearDown() override { hj::debugger::instance().reset_ostream(); }
};

TEST_F(DebuggerTest, raw_char_array_without_null_terminator)
{
    char        raw_array[3] = {0x01, 0x02, 0x03};
    std::string out = hj::debugger::instance().fmt("{:02x}", raw_array);
    EXPECT_EQ(out, "01 02 03");
}

TEST_F(DebuggerTest, raw_char_array_with_embedded_zero)
{
    char        raw_array[5] = {0x01, 0x00, 0x02, 0x00, 0x03};
    std::string out = hj::debugger::instance().fmt("{:02x}", raw_array);
    EXPECT_EQ(out, "01 00 02 00 03");
}

TEST_F(DebuggerTest, bytes_view_normal_data)
{
    uint8_t        data[] = {0x12, 0x34, 0x56, 0x78};
    hj::bytes_view view(data, 4);

    std::string out = hj::debugger::instance().fmt("{:02x}", view);
    EXPECT_EQ(out, "12 34 56 78");
}

TEST_F(DebuggerTest, streambuf_exceed_buf_sz)
{
    boost::asio::streambuf buf;
    std::ostream           os(&buf);

    std::vector<uint8_t> data(hj::debugger::buf_sz + 1, 0xAB);
    os.write(reinterpret_cast<const char *>(data.data()), data.size());

    std::string out = hj::debugger::instance().fmt("{:02x}", buf);

    EXPECT_NE(out.find("ab"), std::string::npos);
    EXPECT_NE(out.find("..."), std::string::npos);
}

TEST_F(DebuggerTest, set_ostream_guard_exception_safety)
{
    std::ostringstream oss;

    EXPECT_THROW(
        {
            [[maybe_unused]] auto guard =
                hj::debugger::instance().set_ostream(oss);
            hj::debugger::instance().print("Before Exception");
            throw std::runtime_error("simulated error");
        },
        std::runtime_error);

    EXPECT_NE(oss.str().find("Before Exception"), std::string::npos);

    std::ostringstream    dummy_oss;
    [[maybe_unused]] auto dummy_guard =
        hj::debugger::instance().set_ostream(dummy_oss);
    hj::debugger::instance().print("After Exception");

    EXPECT_EQ(oss.str().find("After Exception"), std::string::npos);
    EXPECT_NE(dummy_oss.str().find("After Exception"), std::string::npos);
}

TEST_F(DebuggerTest, set_ostream_factory_pattern)
{
    std::ostringstream oss;
    {
        [[maybe_unused]] auto guard = hj::debugger::instance().set_ostream(oss);
        hj::debugger::instance().print("Inside Factory Guard Scope");
    }

    EXPECT_NE(oss.str().find("Inside Factory Guard Scope"), std::string::npos);

    std::ostringstream    dummy_oss;
    [[maybe_unused]] auto dummy_guard =
        hj::debugger::instance().set_ostream(dummy_oss);
    hj::debugger::instance().print("Outside Scope");

    EXPECT_EQ(oss.str().find("Outside Scope"), std::string::npos);
    EXPECT_NE(dummy_oss.str().find("Outside Scope"), std::string::npos);
}

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

TEST_F(DebuggerTest, concurrent_print)
{
    std::ostringstream oss;
    {
        [[maybe_unused]] auto guard = hj::debugger::instance().set_ostream(oss);

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