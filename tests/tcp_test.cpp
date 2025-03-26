#include <gtest/gtest.h>
#include <libcpp/net/tcp.hpp>
#include <thread>
#include <string>

class my_message : public libcpp::message
{
public:
    my_message() : _name{std::string()} {}
    my_message(const char* str) : _name{str} {}
    ~my_message() {}

    std::size_t size()
    {
        return _name.size() + 1;
    }

    std::size_t encode(unsigned char* buf, const std::size_t len)
    {
        memcpy(buf, _name.c_str(), len);
        return len;
    }

    std::size_t decode(const unsigned char* buf, const std::size_t len)
    {
        if (len < 2)
            return 0;

        char tmp[len];
        memcpy(tmp, buf, len);
        _name = std::string(tmp);
        return _name.size();
    }

    std::string name() { return _name; }

private:
    std::string _name;
};

TEST(tcp, tcp_socket)
{
    libcpp::tcp_socket::io_t io;
    std::thread([&]() {
        char buf1[1024] = {0};
        libcpp::tcp_listener listener{io};
        auto sock = listener.accept(10085);
        sock->recv(buf1, 1024);
        sock->send(std::string("world1").c_str(), 7);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        sock->close();
        delete sock;
    }).detach();

    char buf2[1024] = {0};
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    libcpp::tcp_socket sock1{io};
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
    libcpp::tcp_socket::io_t io;
    std::thread([&]() {
        char buf1[1024] = {0};
        libcpp::tcp_listener listener{io};
        auto sock = listener.accept(10086);
        sock->recv(buf1, 1024);
        sock->send(std::string("world2").c_str(), 7);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        sock->close();
        delete sock;
    }).detach();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    libcpp::tcp_socket sock2{io};
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
    libcpp::tcp_socket::io_t io;
    std::thread([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        libcpp::tcp_socket sock1{io};
        sock1.connect("127.0.0.1", 10090);
        sock1.close();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        libcpp::tcp_socket sock2{io};
        sock2.connect("127.0.0.1", 10090);
        sock2.close();
    }).detach();

    libcpp::tcp_listener listener{io};
    listener.async_accept(10090, [](const libcpp::tcp_listener::err_t& err, libcpp::tcp_socket* sock){
        ASSERT_EQ(sock != nullptr, true);
        sock->close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

TEST(tcp, tcp_conn)
{
    libcpp::tcp_conn::io_t io;
    std::thread([&](){
        libcpp::tcp_conn::io_work_t work{io};
        io.run();
    }).detach();
    std::thread([&]() {
        libcpp::tcp_listener li{io};
        // conn1
        char buf[1024] = {0};
        auto sock = li.accept(10091);
        ASSERT_EQ(sock == nullptr, false);
        std::size_t sz = sock->recv(buf, 1024);
        std::string str(buf, sz - 1);
        ASSERT_EQ(sz == 6, true);
        ASSERT_EQ(str == std::string("harry"), true);
        sock->send(std::string("potter").c_str(), 7);
        sock->close();
        delete sock;

        // conn2
        char buf2[1024] = {0};
        auto sock2 = li.accept();
        ASSERT_EQ(sock2 == nullptr, false);
        std::size_t sz2 = sock2->recv(buf2, 1024);
        std::string str2(buf2, sz2 - 1);
        ASSERT_EQ(sz2 == 6, true);
        ASSERT_EQ(str2 == std::string("hello"), true);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        sock2->send(std::string("world").c_str(), 6);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        sock2->close();
        delete sock2;
    }).detach();

    // sync
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    libcpp::tcp_conn conn1{io};
    ASSERT_EQ(conn1.connect("127.0.0.1", 10091), true);

    my_message* msg1 = new my_message("harry");
    ASSERT_EQ(msg1->size() == 6, true);
    ASSERT_EQ(conn1.send((libcpp::message*)(msg1)), true);

    my_message* msg2 = new my_message();
    ASSERT_EQ(conn1.recv((libcpp::message*)(msg2)), true);
    ASSERT_EQ(msg2->name() == std::string("potter"), true);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_EQ(conn1.disconnect(), true);
    conn1.close();
    delete msg1; delete msg2;
    msg1 = nullptr; msg2 = nullptr;

    // async
    libcpp::tcp_conn conn2{io};
    static bool conn_async_entered = false;
    my_message* msg3 = new my_message("hello");
    my_message* msg4 = new my_message();
    ASSERT_EQ(conn2.async_connect("127.0.0.1", 10091, 
        [&](libcpp::tcp_conn::conn_ptr_t conn, libcpp::tcp_conn::err_t err){
            conn_async_entered = true;
            ASSERT_EQ(err.failed(), false);
            ASSERT_EQ(conn != nullptr, true);

            ASSERT_EQ(msg3->size() == 6, true);
            ASSERT_EQ(conn->async_send(msg3, [](libcpp::tcp_conn::conn_ptr_t conn, libcpp::tcp_conn::msg_ptr_t msg){
                auto real = (my_message*)(msg);
                ASSERT_EQ(real->name() == std::string("hello"), true);
            }), true);

            ASSERT_EQ(conn->async_recv(msg4, [](libcpp::tcp_conn::conn_ptr_t, libcpp::tcp_conn::msg_ptr_t msg){
                auto real = (my_message*)(msg);
                ASSERT_EQ(real->name() == std::string("world"), true);
            }), true);
        }), true);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_EQ(conn_async_entered, true);
    ASSERT_EQ(conn2.disconnect(), true);
    conn2.close();
    delete msg3; delete msg4;
    msg3 = nullptr; msg4 = nullptr;
    io.stop();
}

// TEST(tcp, tcp_server)
// {
//     std::thread([]() {
//         libcpp::tcp_server<std::string> srv{};
//         ASSERT_EQ(srv.size() == 0, true);

//         auto conn = srv.listen("127.0.0.1", 10093);
//         ASSERT_EQ(conn != nullptr, true);
//         conn->set_recv_cb([](libcpp::tcp_conn* conn, libcpp::message* msg){
//             my_message* real = (my_message*)(msg);
//             ASSERT_EQ(real->name() == std::string("hello"), true);
//         });
//         conn->set_send_cb([](libcpp::tcp_conn* conn, libcpp::message* msg){
//             my_message* real = (my_message*)(msg);
//             ASSERT_EQ(real->name() == std::string("world"), true);
//         });

//         ASSERT_EQ(srv.add("k1", conn) == 1, true);
//         ASSERT_EQ(srv.size() == 1, true);
//         ASSERT_EQ(srv.get("k1") != nullptr, true);

//         my_message* msg1 = new my_message();
//         ASSERT_EQ(srv.recv(msg1, "k1", false), true);

//         my_message* msg2 = new my_message("world");
//         ASSERT_EQ(srv.send(msg2, "k1", false), true);

//         std::this_thread::sleep_for(std::chrono::milliseconds(200));
//         srv.close();
//         delete msg1; delete msg2;
//         msg1 = nullptr; msg2 = nullptr;
//     }).detach();

//     std::this_thread::sleep_for(std::chrono::milliseconds(100));
//     libcpp::tcp_conn conn{};
//     ASSERT_EQ(conn.connect("127.0.0.1", 10093), true);
//     std::this_thread::sleep_for(std::chrono::milliseconds(50));

//     my_message* msg1 = new my_message("hello");
//     ASSERT_EQ(conn.send(msg1), true);

//     my_message* msg2 = new my_message();
//     ASSERT_EQ(conn.recv(msg2), true);
//     ASSERT_EQ(msg2->name() == std::string("world"), true);

//     std::this_thread::sleep_for(std::chrono::milliseconds(200));
//     delete msg1; delete msg2;
//     msg1 = nullptr; msg2 = nullptr;
// }

// TEST(tcp, tcp_client)
// {
//     std::thread([](){
//         libcpp::tcp_server<std::string> srv{};
//         auto conn1 = srv.listen("127.0.0.1", 10094);
//         ASSERT_EQ(conn1 != nullptr, true);
//         conn1->set_recv_cb([](libcpp::tcp_conn* conn, libcpp::message* msg){
//             my_message* real = (my_message*)(msg);
//             ASSERT_EQ(real->name() == std::string("123"), true);
//             std::cout << "srv conn1 recv" << std::endl;

//             my_message* msg2 = new my_message("hello conn1");
//             conn->send(msg2);
//         });
//         conn1->set_send_cb([](libcpp::tcp_conn* conn, libcpp::message* msg){
//             my_message* real = (my_message*)(msg);
//             std::cout << "srv conn1 send" << std::endl;
//         });
//         srv.add("conn1", conn1);

//         auto conn2 = srv.listen("127.0.0.1", 10094);
//         ASSERT_EQ(conn2 != nullptr, true);
//         conn2->set_recv_cb([](libcpp::tcp_conn* conn, libcpp::message* msg){
//             my_message* real = (my_message*)(msg);
//             ASSERT_EQ(real->name() == std::string("123"), true);
//             std::cout << "srv conn2 recv" << std::endl;

//             my_message* msg2 = new my_message("hello conn2");
//             conn->send(msg2);
//         });
//         conn2->set_send_cb([](libcpp::tcp_conn* conn, libcpp::message* msg){
//             my_message* real = (my_message*)(msg);
//             std::cout << "srv conn2 send" << std::endl;
//         });
//         srv.add("conn2", conn2);

//         std::thread([&](){
//             std::cout << "loop start" << std::endl;
//             srv.loop_start();
//             std::cout << "loop end" << std::endl;
//         }).detach();

//         for (int i = 0; i < 5; i++)
//         {
//             my_message* msg1 = new my_message();
//             srv.recv(msg1);

//             std::this_thread::sleep_for(std::chrono::milliseconds(100));
//             delete msg1; msg1 = nullptr;
//         }

//         srv.close();
//     }).detach();

//     std::this_thread::sleep_for(std::chrono::milliseconds(100));
//     libcpp::tcp_client<int> cli{};
//     ASSERT_EQ(cli.size() == 0, true);
//     ASSERT_EQ(cli.get(1) == nullptr, true);
//     auto conn1 = cli.connect("127.0.0.1", 10094);
//     ASSERT_EQ(conn1 != nullptr, true);
//     conn1->set_recv_cb([](libcpp::tcp_conn* conn, libcpp::message* msg){
//         my_message* real = (my_message*)(msg);
//         ASSERT_EQ(real->name() == std::string("hello conn1"), true);
//     });
//     conn1->set_send_cb([](libcpp::tcp_conn* conn, libcpp::message* msg){
//         my_message* real = (my_message*)(msg);
//         ASSERT_EQ(real->name() == std::string("123"), true);
//     });
//     ASSERT_EQ(cli.add(1, conn1) == 1, true);
//     ASSERT_EQ(cli.add(1, conn1) == 1, true);
//     ASSERT_EQ(cli.get(1) != nullptr, true);
//     ASSERT_EQ(cli.size() == 1, true);

//     auto conn2 = cli.connect("127.0.0.1", 10094);
//     ASSERT_EQ(conn2 != nullptr, true);
//     conn2->set_recv_cb([](libcpp::tcp_conn* conn, libcpp::message* msg){
//         my_message* real = (my_message*)(msg);
//         ASSERT_EQ(real->name() == std::string("hello conn2"), true);
//     });
//     conn2->set_send_cb([](libcpp::tcp_conn* conn, libcpp::message* msg){
//         my_message* real = (my_message*)(msg);
//         ASSERT_EQ(real->name() == std::string("123"), true);
//     });
//     ASSERT_EQ(cli.add(2, conn2) == 2, true);
//     ASSERT_EQ(cli.add(2, conn2) == 2, true);
//     ASSERT_EQ(cli.get(2) != nullptr, true);
//     ASSERT_EQ(cli.size() == 2, true);

//     my_message* msg1 = new my_message("123");
//     ASSERT_EQ(cli.send(msg1, 1), true);

//     my_message* msg2 = new my_message();
//     ASSERT_EQ(cli.recv(msg2, 1), true);

//     my_message* msg3 = new my_message("123");
//     my_message* msg4 = new my_message();
//     ASSERT_EQ(cli.group_cast(msg3, {1, 2}) == 2, true);
//     std::this_thread::sleep_for(std::chrono::milliseconds(50));
//     ASSERT_EQ(cli.recv(msg4, 1), true);
//     ASSERT_EQ(cli.recv(msg4, 2), true);

//     my_message* msg5 = new my_message("123");
//     my_message* msg6 = new my_message();
//     ASSERT_EQ(cli.broad_cast(msg5) == 2, true);
//     std::this_thread::sleep_for(std::chrono::milliseconds(50));
//     ASSERT_EQ(cli.recv(msg6, 1), true);
//     ASSERT_EQ(cli.recv(msg6, 2), true);

//     my_message* msg7 = new my_message("123");
//     my_message* msg8 = new my_message();
//     std::this_thread::sleep_for(std::chrono::milliseconds(50));
//     ASSERT_EQ(cli.sub("hello", 1), true);
//     cli.pub("topic", msg7);
//     ASSERT_EQ(cli.recv(msg8, 1), true);
//     ASSERT_EQ(msg8->name() == std::string("hello conn1"), true);

//     my_message* msg9 = new my_message("123");
//     my_message* msg10 = new my_message();
//     std::this_thread::sleep_for(std::chrono::milliseconds(50));
//     ASSERT_EQ(cli.req_resp(msg9, msg10, 1), true);
//     ASSERT_EQ(msg9->name() == std::string("123"), true);
//     ASSERT_EQ(msg10->name() == std::string("hello conn1"), true);

//     std::this_thread::sleep_for(std::chrono::milliseconds(50));
//     delete msg1; delete msg2; delete msg3; delete msg4; delete msg5; 
//     delete msg6; delete msg7; delete msg8; delete msg9; delete msg10; 
//     msg1 = nullptr; msg2 = nullptr; msg3 = nullptr; msg4 = nullptr; msg5 = nullptr;
//     msg6 = nullptr; msg7 = nullptr; msg8 = nullptr; msg9 = nullptr; msg10 = nullptr;
//     cli.close();
// }