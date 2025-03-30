#include <gtest/gtest.h>
#include <libcpp/net/tcp.hpp>
#include <thread>
#include <string>

// class my_message : public libcpp::message
// {
// public:
//     my_message() : _name{std::string()} {}
//     my_message(const char* str) : _name{str} {}
//     ~my_message() {}

//     std::size_t size()
//     {
//         return _name.size() + 1;
//     }

//     std::size_t encode(unsigned char* buf, const std::size_t len)
//     {
//         memcpy(buf, _name.c_str(), len);
//         return len;
//     }

//     std::size_t decode(const unsigned char* buf, const std::size_t len)
//     {
//         if (len < 2)
//             return 0;

//         char tmp[len];
//         memcpy(tmp, buf, len);
//         _name = std::string(tmp);
//         return _name.size();
//     }

//     std::string name() { return _name; }

// private:
//     std::string _name;
// };

// TEST(tcp_conn, tcp_conn)
// {
//     libcpp::tcp_conn::io_t io;
//     std::thread([&](){
//         libcpp::tcp_conn::io_work_t work{io};
//         io.run();
//     }).detach();
//     std::thread([&]() {
//         libcpp::tcp_listener li{io};
//         // conn1
//         char buf[1024] = {0};
//         auto sock = li.accept(10091);
//         ASSERT_EQ(sock == nullptr, false);
//         std::size_t sz = sock->recv(buf, 1024);
//         std::string str(buf, sz - 1);
//         ASSERT_EQ(sz == 6, true);
//         ASSERT_EQ(str == std::string("harry"), true);
//         sock->send(std::string("potter").c_str(), 7);
//         sock->close();
//         delete sock;

//         // conn2
//         char buf2[1024] = {0};
//         auto sock2 = li.accept();
//         ASSERT_EQ(sock2 == nullptr, false);
//         std::size_t sz2 = sock2->recv(buf2, 1024);
//         std::string str2(buf2, sz2 - 1);
//         ASSERT_EQ(sz2 == 6, true);
//         ASSERT_EQ(str2 == std::string("hello"), true);
//         std::this_thread::sleep_for(std::chrono::milliseconds(20));
//         sock2->send(std::string("world").c_str(), 6);
//         std::this_thread::sleep_for(std::chrono::milliseconds(20));
//         sock2->close();
//         delete sock2;
//     }).detach();

//     // sync
//     std::this_thread::sleep_for(std::chrono::milliseconds(50));
//     libcpp::tcp_conn conn1{io};
//     ASSERT_EQ(conn1.connect("127.0.0.1", 10091), true);

//     my_message* msg1 = new my_message("harry");
//     ASSERT_EQ(msg1->size() == 6, true);
//     ASSERT_EQ(conn1.send((libcpp::message*)(msg1)), true);

//     my_message* msg2 = new my_message();
//     ASSERT_EQ(conn1.recv((libcpp::message*)(msg2)), true);
//     ASSERT_EQ(msg2->name() == std::string("potter"), true);

//     std::this_thread::sleep_for(std::chrono::milliseconds(100));
//     ASSERT_EQ(conn1.disconnect(), true);
//     conn1.close();
//     delete msg1; delete msg2;
//     msg1 = nullptr; msg2 = nullptr;

//     // async
//     libcpp::tcp_conn conn2{io};
//     static bool conn_async_entered = false;
//     my_message* msg3 = new my_message("hello");
//     my_message* msg4 = new my_message();
//     ASSERT_EQ(conn2.async_connect("127.0.0.1", 10091, 
//         [&](libcpp::tcp_conn::conn_ptr_t conn, libcpp::tcp_conn::err_t err){
//             conn_async_entered = true;
//             ASSERT_EQ(err.failed(), false);
//             ASSERT_EQ(conn != nullptr, true);

//             ASSERT_EQ(msg3->size() == 6, true);
//             ASSERT_EQ(conn->async_send(msg3, [](libcpp::tcp_conn::conn_ptr_t conn, libcpp::tcp_conn::msg_ptr_t msg){
//                 auto real = (my_message*)(msg);
//                 ASSERT_EQ(real->name() == std::string("hello"), true);
//             }), true);

//             ASSERT_EQ(conn->async_recv(msg4, [](libcpp::tcp_conn::conn_ptr_t, libcpp::tcp_conn::msg_ptr_t msg){
//                 auto real = (my_message*)(msg);
//                 ASSERT_EQ(real->name() == std::string("world"), true);
//             }), true);
//         }), true);

//     std::this_thread::sleep_for(std::chrono::milliseconds(100));
//     ASSERT_EQ(conn_async_entered, true);
//     ASSERT_EQ(conn2.disconnect(), true);
//     conn2.close();
//     delete msg3; delete msg4;
//     msg3 = nullptr; msg4 = nullptr;
//     io.stop();
// }

// TEST(tcp_conn, set_disconnect_handler)
// {
//     std::thread t([]() {
//         libcpp::tcp_conn::io_t io;
//         libcpp::tcp_listener li{io};
//         for (int i = 0; i < 2; i++)
//         {
//             auto sock = li.accept(10091);
//             ASSERT_EQ(sock == nullptr, false);
//             sock->close();
//             delete sock;
//         }
//     });

//     std::this_thread::sleep_for(std::chrono::milliseconds(50));
//     libcpp::tcp_conn::io_t io;
    
//     libcpp::tcp_conn conn1{io};
//     conn1.set_disconnect_handler([&](libcpp::tcp_conn::conn_ptr_t conn){
//         ASSERT_EQ(conn != nullptr, true);
//         ASSERT_EQ(conn == (&conn1), true);
//     });
//     ASSERT_EQ(conn1.connect("127.0.0.1", 10091), true);

//     libcpp::tcp_conn conn2{io};
//     conn2.set_disconnect_handler([](libcpp::tcp_conn::conn_ptr_t conn){
//         ASSERT_EQ(conn != nullptr, true);
//     });
//     ASSERT_EQ(conn2.connect("127.0.0.1", 10091), true);
//     t.join();
// }

// TEST(tcp_conn, get_flag)
// {
//     libcpp::tcp_conn::io_t io;
//     libcpp::tcp_conn conn1{io};
// }

// TEST(tcp_conn, set_flag)
// {
//     libcpp::tcp_conn::io_t io;
//     libcpp::tcp_conn conn1{io};

//     libcpp::tcp_conn::flag_t flag1 = 0x1;
//     libcpp::tcp_conn::flag_t flag2 = 0x2;

//     conn1.set_flag(conn1.get_flag() | flag1);
//     conn1.set_flag(conn1.get_flag() & flag1);
//     ASSERT_EQ(conn1.get_flag() == flag1, true);

//     conn1.set_flag(conn1.get_flag() | flag2);
//     ASSERT_EQ(conn1.get_flag() == (flag1 | flag2), true);

//     conn1.set_flag(conn1.get_flag() & flag2);
//     ASSERT_EQ(conn1.get_flag() == flag2, true);
// }

// TEST(tcp_conn, connect)
// {
//     std::thread t([]() {
//         libcpp::tcp_conn::io_t io;
//         libcpp::tcp_listener li{io};
//         for (int i = 0; i < 2; i++)
//         {
//             if (i == 0) {
//                 std::this_thread::sleep_for(std::chrono::milliseconds(3000));
//                 continue;
//             }

//             auto sock = li.accept(10091);
//             ASSERT_EQ(sock != nullptr, true);
//             sock->close();
//             delete sock;
//         }
//     });

//     std::this_thread::sleep_for(std::chrono::milliseconds(20));
//     libcpp::tcp_conn::io_t io;
    
//     libcpp::tcp_conn conn1{io};
//     conn1.set_disconnect_handler([&](libcpp::tcp_conn::conn_ptr_t conn){
//         ASSERT_EQ(conn != nullptr, true);
//         ASSERT_EQ(conn == (&conn1), true);
//     });
//     ASSERT_EQ(conn1.connect("127.0.0.1", 10091), false);
    
//     ASSERT_EQ(conn1.connect("127.0.0.1", 10091, 2), true);
// }

// TEST(tcp_conn, async_connect)
// {

// }

// TEST(tcp_conn, is_connected)
// {

// }

// TEST(tcp_conn, disconnect)
// {

// }

// TEST(tcp_conn, send)
// {

// }

// TEST(tcp_conn, async_send)
// {

// }

// TEST(tcp_conn, recv)
// {

// }

// TEST(tcp_conn, async_recv)
// {

// }

// TEST(tcp_conn, close)
// {

// }

// TEST(tcp_conn, read_all)
// {

// }

// TEST(tcp_conn, write_all)
// {

// }