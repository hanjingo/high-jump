#include <gtest/gtest.h>
#include <libcpp/net/tcp.hpp>
#include <thread>
#include <string>

class message1 : public libcpp::message
{
public:
    message1() : _name{std::string()} {}
    message1(const char* str) : _name{str} {}
    ~message1() {}

    std::size_t size()
    {
        return _name.size() + 1;
    }

    std::size_t encode(unsigned char* buf, const std::size_t len)
    {
        memcpy(buf, _name.c_str(), len);
        return len;
    }

    bool decode(const unsigned char* buf, const std::size_t len)
    {
        char tmp[len];
        memcpy(tmp, buf, len);
        _name = std::string(tmp);
        return true;
    }

    std::string name() { return _name; }

private:
    std::string _name;
};

TEST(tcp, tcp_socket)
{
    std::thread([]() {
        char buf1[1024] = {0};
        libcpp::tcp_listener listener{};
        auto sock = listener.accept(10085);
        sock->recv(buf1, 1024);
        sock->send(std::string("world1").c_str(), 7);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        sock->close();
        delete sock;
    }).detach();

    char buf2[1024] = {0};
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    libcpp::tcp_socket sock1{};
    ASSERT_EQ(sock1.connect("127.0.0.1", 10085), true);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_EQ(sock1.send(std::string("hello").c_str(), 6) == 6, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_EQ(sock1.recv(buf2, 1024) == 7, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_EQ(sock1.is_connected(), true);
    sock1.close();
    ASSERT_EQ(sock1.is_connected(), false);
}

TEST(tcp, tcp_socket_async)
{
    std::thread([]() {
        char buf1[1024] = {0};
        libcpp::tcp_listener listener{};
        auto sock = listener.accept(10086);
        sock->recv(buf1, 1024);
        sock->send(std::string("world2").c_str(), 7);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        sock->close();
        delete sock;
    }).detach();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    libcpp::tcp_socket sock2{};
    sock2.async_connect(
        "127.0.0.1", 
        10086, 
        [&](const libcpp::tcp_socket::err_t& err, libcpp::tcp_socket* sock){
            ASSERT_EQ(!err.failed(), true);
            ASSERT_EQ(sock != nullptr, true);

            sock2.async_send(
                std::string("hello").c_str(), 
                6,
                [](const libcpp::tcp_socket::err_t& err, std::size_t sz) {
                    ASSERT_EQ(!err.failed(), true);
                    ASSERT_EQ(sz == 6, true);
                }
            );
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            char buf[1024] = {0};
            sock2.async_recv(
                buf, 
                1024,
                [](const libcpp::tcp_socket::err_t& err, std::size_t sz) {
                ASSERT_EQ(!err.failed(), true);
                ASSERT_EQ(sz == 7, true);
            });
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    sock2.close();
    ASSERT_EQ(sock2.is_connected(), false);
}

TEST(tcp, tcp_listener)
{
    std::thread([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        libcpp::tcp_socket sock1{};
        sock1.connect("127.0.0.1", 10090);
        sock1.close();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        libcpp::tcp_socket sock2{};
        sock2.connect("127.0.0.1", 10090);
        sock2.close();
    }).detach();

    libcpp::tcp_listener listener{};
    auto sock1 = listener.accept(10090);
    ASSERT_EQ(sock1 != nullptr, true);
    sock1->close();

    listener.async_accept(10090, [](const err_t& err, tcp_socket* sock){
        ASSERT_EQ(sock != nullptr, true);
        sock->close();
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

TEST(tcp, tcp_conn)
{
    std::thread([]() {
        std::string str;
        char buf1[1024] = {0};
        libcpp::tcp_listener listener{};
        auto sock = listener.accept(10087);
        std::size_t sz = sock->recv(buf1, 1024);
        str.copy(buf1, sz);
        ASSERT_EQ(str == std::string("harry"), true);

        sock->send(std::string("potter").c_str(), 7);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        sock->close();
        delete sock;
    }).detach();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    libcpp::tcp_conn conn1{};
    ASSERT_EQ(conn1.connect("127.0.0.1", 10087), true);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    message1* msg1 = new message1("harry");
    ASSERT_EQ(msg1.size() == 6, true);
    conn1.send((libcpp::message*)(msg1));

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    message1* msg2 = new message1();
    conn1.recv((libcpp::message*)(msg2));
    ASSERT_EQ(msg2.name() == std::string("potter"), true);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_EQ(conn1.disconnect(), true);

    ASSERT_EQ(conn1.close(), true);
    delete msg1; delete msg2;
    msg1 = nullptr; msg2 = nullptr;
}

TEST(tcp, tcp_conn_async)
{
    std::thread([]() {
        std::string str;
        char buf1[1024] = {0};
        libcpp::tcp_listener listener{};
        auto sock = listener.accept(10088);
        std::size_t sz = sock->recv(buf1, 1024);
        str.copy(buf1, sz);
        ASSERT_EQ(str == std::string("harry"), true);

        sock->send(std::string("potter").c_str(), 7);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        sock->close();
        delete sock;
    }).detach();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    libcpp::tcp_conn conn1{};
    conn1.set_send_cb([&](libcpp::tcp_conn* conn, libcpp::message* msg){
        message1* real = (message1*)(msg); 
        ASSERT_EQ(real->name() == std::string("harry"), true);
    });
    conn1.set_recv_cb([&](libcpp::tcp_conn* conn, libcpp::message* msg){
        message1* real = (message1*)(msg); 
        ASSERT_EQ(real->name() == std::string("potter"), true);
    });
    ASSERT_EQ(conn1.connect("127.0.0.1", 10088), true);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    message1* msg1 = new message1("harry");
    conn1.async_send((libcpp::message*)(&msg1));

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    message1 msg2 = new message1();
    conn1.async_recv((libcpp::message*)(&msg2));

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_EQ(conn1.disconnect(), true);
}

TEST(tcp, tcp_server)
{
    std::thread([]() {
        libcpp::tcp_server<std::string> srv{};
        ASSERT_EQ(srv.conn_size() == 0, true);

        auto conn = srv.listen("127.0.0.1", 10089);
        ASSERT_EQ(conn != nullptr, true);
        conn.set_recv_cb([](tcp_conn* conn, message* msg){
            message1* real = (message1*)(msg);
            ASSERT_EQ(real->name() == std::string("hello"), true);
            delete msg;
            msg = nullptr;
        });
        conn.set_send_cb([](tcp_conn* conn, message* msg){
            message1* real = (message1*)(msg);
            ASSERT_EQ(real->name() == std::string("world"), true);
            delete msg;
            msg = nullptr;
        });

        ASSERT_EQ(srv.add("k1", conn) == 1, true);
        ASSERT_EQ(srv.conn_size() == 1, true);
        ASSERT_EQ(srv.get("k1") != nullptr, true);

        message1* msg1 = new message1();
        ASSERT_EQ(conn.recv(msg1), true);

        message1* msg2 = new message1();
        ASSERT_EQ(conn.send(msg2), true);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    libcpp::tcp_conn conn{};
    ASSERT_EQ(conn.connect("127.0.0.1", 10089), true);
    conn.set_send_cb([](tcp_conn* conn, message* msg){
        delete msg;
        msg = nullptr;
    });
    conn.set_recv_cb([](tcp_conn* conn, message* msg){
        delete msg;
        msg = nullptr;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    message1* msg1 = new message1("hello");
    ASSERT_EQ(conn.send(msg1), true);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    message1* msg2 = new message1();
    ASSERT_EQ(conn.recv(msg2), true);
    ASSERT_EQ(msg2->name() == std::string("world"), true);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

TEST(tcp, tcp_client)
{
    static int srv_conn1_recv_count = 0;
    static int srv_conn2_recv_count = 0;
    std::thread([](){
        libcpp::tcp_server<std::string> srv{};
        auto conn1 = srv.listen("127.0.0.1", 10091);
        ASSERT_EQ(conn1 != nullptr, true);
        conn1.set_recv_cb([](tcp_conn* conn, message* msg){
            message1* real = (message1*)(msg);
            ASSERT_EQ(real->name() == std::string("123"), true);
            srv_conn1_recv_count++;

            message1* msg2 = new message1("hello conn1");
            conn->send(msg2);
        });
        conn1.set_send_cb([](tcp_conn* conn, message* msg){
            message1* real = (message1*)(msg);
        });
        srv.add("conn1", conn1);

        auto conn2 = srv.listen("127.0.0.1", 10091);
        ASSERT_EQ(conn2 != nullptr, true);
        conn2.set_recv_cb([](tcp_conn* conn, message* msg){
            message1* real = (message1*)(msg);
            ASSERT_EQ(real->name() == std::string("123"), true);
            srv_conn2_recv_count++;

            message1* msg2 = new message1("hello conn2");
            conn->send(msg2);
        });
        conn2.set_send_cb([](tcp_conn* conn, message* msg){
            message1* real = (message1*)(msg);
        });
        srv.add("conn2", conn2);

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        srv.close();
    }).detach();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    libcpp::tcp_client cli{};
    ASSERT_EQ(cli.size() == 0, true);
    ASSERT_EQ(cli.get("k1") == nullptr, true);
    auto conn1 = cli.connect("127.0.0.1", 10091);
    ASSERT_EQ(conn1 != nullptr, true);
    conn1.set_recv_cb([](tcp_conn* conn, message* msg){
        message1* real = (message1*)(msg);
        ASSERT_EQ(real->name() == std::string("hello conn1"), true);
    });
    conn1.set_send_cb([](tcp_conn* conn, message* msg){
        message1* real = (message1*)(msg);
        ASSERT_EQ(real->name() == std::string("conn1"), true);
    });
    ASSERT_EQ(cli.add(1, conn1) == 1, true);
    ASSERT_EQ(cli.add(1, conn1) == 1, true);
    ASSERT_EQ(cli.get(1) != nullptr, true);
    ASSERT_EQ(cli.size() == 1, true);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    cli.async_connect("127.0.0.1", 10091, )
    auto conn2 = cli.connect();
    ASSERT_EQ(conn2 != nullptr, true);
    conn2.set_recv_cb([](tcp_conn* conn, message* msg){
        message1* real = (message1*)(msg);
        ASSERT_EQ(real->name() == std::string("hello conn2"), true);
    });
    conn2.set_send_cb([](tcp_conn* conn, message* msg){
        message1* real = (message1*)(msg);
        ASSERT_EQ(real->name() == std::string("conn2"), true);
    });
    ASSERT_EQ(cli.add(2, conn2) == 2, true);
    ASSERT_EQ(cli.add(2, conn2) == 2, true);
    ASSERT_EQ(cli.get(2) != nullptr, true);
    ASSERT_EQ(cli.size() == 2, true);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    message1* msg1 = new message1("123");
    ASSERT_EQ(cli.send(msg1, 1));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_EQ(srv_conn1_recv_count == 1, true);

    cli.group_cast(msg1, {1, 2});
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_EQ(srv_conn1_recv_count == 2, true);
    ASSERT_EQ(srv_conn2_recv_count == 1, true);

    cli.broad_cast(msg1);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_EQ(srv_conn1_recv_count == 3, true);
    ASSERT_EQ(srv_conn2_recv_count == 2, true);

    delete msg1; delete msg2;
    msg1 = nullptr; msg2 = nullptr;
    cli.close();
}