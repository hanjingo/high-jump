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