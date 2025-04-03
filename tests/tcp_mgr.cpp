#include <gtest/gtest.h>
#include <libcpp/net/tcp.hpp>

TEST(tcp_mgr, emplace)
{
    libcpp::tcp_mgr<std::string, libcpp::tcp_conn*> mgr{};
    libcpp::tcp_conn::io_t io;
    libcpp::tcp_conn* conn1 = new libcpp::tcp_conn(io);
    mgr.emplace("conn1", std::move(conn1));

    libcpp::tcp_conn* conn2 = new libcpp::tcp_conn(io);
    mgr.emplace("conn2", std::move(conn2));
}

TEST(tcp_mgr, replace)
{
    libcpp::tcp_mgr<std::string, libcpp::tcp_conn*> mgr{};
    libcpp::tcp_conn::io_t io;
    libcpp::tcp_conn* conn1 = new libcpp::tcp_conn(io);
    mgr.emplace("conn1", std::move(conn1));

    libcpp::tcp_conn* val = nullptr;
    ASSERT_EQ(mgr.find("conn1", val), true);
    ASSERT_EQ(val == conn1, true);

    libcpp::tcp_conn* conn3 = new libcpp::tcp_conn(io);
    mgr.replace("conn1", std::move(conn3), val);
    ASSERT_EQ(val == conn1, true);
    ASSERT_EQ(mgr.find("conn1", val), true);
    ASSERT_EQ(val == conn3, true);
}

TEST(tcp_mgr, find)
{
    libcpp::tcp_mgr<std::string, libcpp::tcp_conn*> mgr{};
    libcpp::tcp_conn::io_t io;
    libcpp::tcp_conn* conn1 = new libcpp::tcp_conn(io);
    mgr.emplace("conn1", std::move(conn1));

    libcpp::tcp_conn* conn2 = new libcpp::tcp_conn(io);
    mgr.emplace("conn2", std::move(conn2));

    libcpp::tcp_conn* val = nullptr;
    ASSERT_EQ(mgr.find("conn1", val), true);
    ASSERT_EQ(val != nullptr, true);
}

TEST(tcp_mgr, erase)
{
    libcpp::tcp_mgr<std::string, libcpp::tcp_conn*> mgr{};
    libcpp::tcp_conn::io_t io;
    libcpp::tcp_conn* conn1 = new libcpp::tcp_conn(io);
    mgr.emplace("conn1", std::move(conn1));

    ASSERT_EQ(mgr.erase("conn1"), true);
    ASSERT_EQ(mgr.erase("conn1"), false);
}

TEST(tcp_mgr, range)
{
    libcpp::tcp_mgr<std::string, libcpp::tcp_conn*> mgr{};
    libcpp::tcp_conn::io_t io;
    libcpp::tcp_conn* conn1 = new libcpp::tcp_conn(io);
    mgr.emplace("conn1", std::move(conn1));

    libcpp::tcp_conn* conn2 = new libcpp::tcp_conn(io);
    mgr.emplace("conn2", std::move(conn2));

    int nrange1 = 0;
    mgr.range([&nrange1](const std::string& key, libcpp::tcp_conn* value)->bool{
        nrange1++;
        std::cout << "key:" << key << ", value:" << value << std::endl;
        return false;
    });
    ASSERT_EQ(nrange1 == 1, true);

    int nrange2 = 0;
    mgr.range([&nrange2](const std::string& key, libcpp::tcp_conn* value)->bool{
        nrange2++;
        std::cout << "key:" << key << ", value:" << value << std::endl;
        return true;
    });
    ASSERT_EQ(nrange2 == 2, true);
    
}

TEST(tcp_mgr, count)
{
    libcpp::tcp_mgr<std::string, libcpp::tcp_conn*> mgr{};
    libcpp::tcp_conn::io_t io;
    libcpp::tcp_conn* conn1 = new libcpp::tcp_conn(io);
    mgr.emplace("conn1", std::move(conn1));
    ASSERT_EQ(mgr.count("conn1") == 1, true);

    libcpp::tcp_conn* conn2 = new libcpp::tcp_conn(io);
    mgr.emplace("conn2", std::move(conn2));
    ASSERT_EQ(mgr.count("conn2") == 1, true);
}