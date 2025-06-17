#include <gtest/gtest.h>
#include <libcpp/net/tcp.hpp>
#include <thread>
#include <string>

class my_message : public libcpp::message
{
public:
    my_message() : text_{std::string()} {}
    my_message(const char* str) : text_{ str } {}
    ~my_message() {}

    std::size_t size()
    {
        return text_.size(); // ignore '\n'
    }

    std::size_t encode(unsigned char* buf, const std::size_t len)
    {
        memcpy(buf, text_.c_str(), len);
        return len;
    }

    std::size_t decode(const unsigned char* buf, const std::size_t len)
    {
        if (len < 5)
            return 0;

        text_ = std::string(reinterpret_cast<const char*>(buf), 5);
        return 5; // ignore '\n'
    }

    std::string text() { return text_; }

private:
    std::string text_;
};

TEST(tcp_conn, get_flag)
{
    libcpp::tcp_conn::io_t io;
    libcpp::tcp_conn conn1{io};
    ASSERT_EQ(conn1.get_flag() == 0, true);
}

TEST(tcp_conn, set_flag)
{
    libcpp::tcp_conn::io_t io;
    libcpp::tcp_conn conn1{io};

    libcpp::tcp_conn::flag_t flag1 = 0x1;
    libcpp::tcp_conn::flag_t flag2 = 0x2;

    conn1.set_flag(flag1);
    ASSERT_EQ(conn1.get_flag() == flag1, true);

    conn1.set_flag(flag2);
    ASSERT_EQ(conn1.get_flag() == (flag1 | flag2), true);
}

TEST(tcp_conn, unset_flag)
{
    libcpp::tcp_conn::io_t io;
    libcpp::tcp_conn conn1{io};

    libcpp::tcp_conn::flag_t flag1 = 0x1;
    libcpp::tcp_conn::flag_t flag2 = 0x2;

    conn1.set_flag(flag1);
    ASSERT_EQ(conn1.get_flag() == flag1, true);

    conn1.set_flag(flag2);
    ASSERT_EQ(conn1.get_flag() == (flag1 | flag2), true);

    conn1.unset_flag(flag2);
    ASSERT_EQ(conn1.get_flag() == flag1, true);
}

TEST(tcp_conn, is_connected)
{
    libcpp::tcp_conn::io_t io;
    libcpp::tcp_conn conn1{io};

    ASSERT_EQ(conn1.is_connected(), false);
}

TEST(tcp_conn, is_w_closed)
{
    libcpp::tcp_conn::io_t io;
    libcpp::tcp_conn conn1{io};

    ASSERT_EQ(conn1.is_w_closed(), false);
}

TEST(tcp_conn, is_r_closed)
{
    libcpp::tcp_conn::io_t io;
    libcpp::tcp_conn conn1{io};

    ASSERT_EQ(conn1.is_r_closed(), false);
}

TEST(tcp_conn, set_w_closed)
{
    std::thread t1([]() {
        libcpp::tcp_conn::io_t io;
        libcpp::tcp_listener li{io};
        auto sock = li.accept(10008);
        ASSERT_EQ(sock == nullptr, false);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        li.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    libcpp::tcp_conn::io_t io;
    libcpp::tcp_conn conn1{io};
    ASSERT_EQ(conn1.is_w_closed(), false);
    conn1.set_w_closed(true);
    ASSERT_EQ(conn1.is_w_closed(), true);
    conn1.set_w_closed(false);
    ASSERT_EQ(conn1.is_w_closed(), false);

    ASSERT_EQ(conn1.connect("127.0.0.1", 10008), true);
    conn1.set_r_closed(true);
    ASSERT_EQ(conn1.is_connected(), true);
    conn1.set_w_closed(true);
    ASSERT_EQ(conn1.is_connected(), false);

    t1.join();
}

TEST(tcp_conn, set_r_closed)
{
    std::thread t1([]() {
        libcpp::tcp_conn::io_t io;
        libcpp::tcp_listener li{io};
        auto sock = li.accept(10009);
        ASSERT_EQ(sock == nullptr, false);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        li.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    libcpp::tcp_conn::io_t io;
    libcpp::tcp_conn conn1{io};
    ASSERT_EQ(conn1.is_r_closed(), false);
    conn1.set_r_closed(true);
    ASSERT_EQ(conn1.is_r_closed(), true);
    conn1.set_r_closed(false);
    ASSERT_EQ(conn1.is_r_closed(), false);

    ASSERT_EQ(conn1.connect("127.0.0.1", 10009), true);
    conn1.set_w_closed(true);
    ASSERT_EQ(conn1.is_connected(), true);
    conn1.set_r_closed(true);
    ASSERT_EQ(conn1.is_connected(), false);

    t1.join();
}

TEST(tcp_conn, connect)
{
    std::thread t1([]() {
        libcpp::tcp_conn::io_t io;
        libcpp::tcp_listener li{io};
        for (int i = 0; i < 3; i++)
        {
            auto sock = li.accept(10010);
            ASSERT_EQ(sock == nullptr, false);
            sock->close();
            delete sock;
        }
        li.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    libcpp::tcp_conn::io_t io;
    libcpp::tcp_conn conn1{io};
    ASSERT_EQ(conn1.connect("127.0.0.1", 10010), true);
    ASSERT_EQ(conn1.connect("127.0.0.1", 10010), false);

    libcpp::tcp_conn conn2{io};
    ASSERT_EQ(conn2.connect("127.0.0.1", 10010), true);
    ASSERT_EQ(conn2.disconnect(), true);
    ASSERT_EQ(conn2.connect("127.0.0.1", 10010), true);

    t1.join();
}

TEST(tcp_conn, async_connect)
{
    std::thread t1([]() {
        libcpp::tcp_conn::io_t io;
        libcpp::tcp_listener li{io};
        for (int i = 0; i < 2; i++)
        {
            auto sock = li.accept(10011);
            ASSERT_EQ(sock == nullptr, false);
            sock->close();
            delete sock;
        }
        li.close();
    });

    libcpp::tcp_conn::io_t io;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    int connect_times = 0;
    libcpp::tcp_conn conn1{io};
    ASSERT_EQ(conn1.async_connect("127.0.0.1", 10011,
        [&connect_times](libcpp::tcp_conn::conn_ptr_t conn, const libcpp::tcp_conn::err_t& err){
            ASSERT_EQ(conn != nullptr, true);
            ASSERT_EQ(err.failed(), false);
            connect_times++;

            conn->disconnect();
        }), true);

    libcpp::tcp_conn conn2{io};
    ASSERT_EQ(conn2.async_connect("127.0.0.1", 10011,
        [&connect_times](libcpp::tcp_conn::conn_ptr_t conn, const libcpp::tcp_conn::err_t& err){
            ASSERT_EQ(conn != nullptr, true);
            ASSERT_EQ(err.failed(), false);
            connect_times++;

            conn->disconnect();
        }), true);
    
    io.run();
    t1.join();
    ASSERT_EQ(connect_times == 2, true);
}

TEST(tcp_conn, disconnect)
{
    std::thread t1([]() {
        libcpp::tcp_conn::io_t io;
        libcpp::tcp_listener li{io};
        for (int i = 0; i < 2; i++)
        {
            auto sock = li.accept(10012);
            ASSERT_EQ(sock == nullptr, false);
            sock->close();
            delete sock;
        }
        li.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    libcpp::tcp_conn::io_t io;
    libcpp::tcp_conn conn1{io};
    ASSERT_EQ(conn1.disconnect(), false);
    ASSERT_EQ(conn1.connect("127.0.0.1", 10012), true);
    ASSERT_EQ(conn1.disconnect(), true);

    libcpp::tcp_conn conn2{io};
    ASSERT_EQ(conn2.connect("127.0.0.1", 10012), true);
    ASSERT_EQ(conn2.disconnect(), true);
    ASSERT_EQ(conn2.disconnect(), false);

    t1.join();
}

TEST(tcp_conn, send)
{
    std::thread t1([]() {
        libcpp::tcp_conn::io_t io;
        libcpp::tcp_listener li{io};
        unsigned char buf[1024] = {0};
        std::size_t sz = 0;
        for (int i = 0; i < 2; i++)
        {
            auto base = li.accept(10013);
            my_message* msg1 = new my_message();
            my_message* msg2 = new my_message();
            libcpp::tcp_conn sock{io, base};
            ASSERT_EQ(sock.recv(msg1), true);
            ASSERT_EQ(msg1->text() == std::string("hello"), true);

            ASSERT_EQ(sock.recv(msg2), true);
            ASSERT_EQ(msg2->text() == std::string("world"), true);
            sock.close();
        }
        li.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    my_message* msg1 = new my_message("hello");
    my_message* msg2 = new my_message("world");

    libcpp::tcp_conn::io_t io;
    libcpp::tcp_conn conn1{io};
    ASSERT_EQ(conn1.connect("127.0.0.1", 10013), true);
    ASSERT_EQ(conn1.send(msg1), true);
    ASSERT_EQ(conn1.send(msg2), true);

    libcpp::tcp_conn conn2{io};
    ASSERT_EQ(conn2.connect("127.0.0.1", 10013), true);
    ASSERT_EQ(conn2.send(msg1), true);
    ASSERT_EQ(conn2.send(msg2), true);

    t1.join();
}

TEST(tcp_conn, recv)
{
    std::thread t1([]() {
        libcpp::tcp_conn::io_t io;
        libcpp::tcp_listener li{io};
        my_message* msg1 = new my_message("hello");
        my_message* msg2 = new my_message("world");
        for (int i = 0; i < 2; i++)
        {
            auto base = li.accept(10014);
            libcpp::tcp_conn sock{io, base};
            ASSERT_EQ(sock.send(msg1), true);
            ASSERT_EQ(sock.send(msg2), true);
        }
        li.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    my_message* msg1 = new my_message();
    my_message* msg2 = new my_message();

    libcpp::tcp_conn::io_t io;
    libcpp::tcp_conn conn1{io};
    ASSERT_EQ(conn1.connect("127.0.0.1", 10014), true);
    ASSERT_EQ(conn1.recv(msg1), true);
    ASSERT_EQ(msg1->size() == 5, true);
    ASSERT_EQ(msg1->text() == "hello", true);
    ASSERT_EQ(conn1.recv(msg2), true);
    ASSERT_EQ(msg2->text() == "world", true);
    
    libcpp::tcp_conn conn2{io};
    ASSERT_EQ(conn2.connect("127.0.0.1", 10014), true);
    ASSERT_EQ(conn2.recv(msg1), true);
    ASSERT_EQ(msg1->text() == "hello", true);
    ASSERT_EQ(conn2.recv(msg2), true);
    ASSERT_EQ(msg2->text() == "world", true);

    t1.join();
}

TEST(tcp_conn, async_send)
{
    std::thread t1([]() {
        libcpp::tcp_conn::io_t io;
        libcpp::tcp_listener li{io};
        for (int i = 0; i < 2; i++)
        {
            auto base = li.accept(10015);
            my_message* msg1 = new my_message();
            my_message* msg2 = new my_message();
            libcpp::tcp_conn sock{io, base};
            ASSERT_EQ(sock.recv(msg1), true);
            //ASSERT_EQ(msg1->text() == std::string("hello"), true);

            ASSERT_EQ(sock.recv(msg2), true);
            //ASSERT_EQ(msg2->text() == std::string("world"), true);
        }
        li.close();
    });

    int nsend = 0;
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    libcpp::tcp_conn::io_t io;
    libcpp::tcp_conn conn1{
        io,
        [](libcpp::tcp_conn::conn_ptr_t conn) {},
        [&nsend](libcpp::tcp_conn::conn_ptr_t conn, libcpp::tcp_conn::msg_ptr_t msg){
            nsend++;
        }
    };
    ASSERT_EQ(conn1.async_connect("127.0.0.1", 10015,
        [](libcpp::tcp_conn::conn_ptr_t conn, const libcpp::tcp_conn::err_t& err){
            ASSERT_EQ(err.failed(), false);
            my_message* msg1 = new my_message("hello");
            conn->async_send(msg1);
            my_message* msg2 = new my_message("world");
            conn->async_send(msg2);
        }), true);

     libcpp::tcp_conn conn2{
          io,
          [](libcpp::tcp_conn::conn_ptr_t conn) {},
          [&nsend](libcpp::tcp_conn::conn_ptr_t conn, libcpp::tcp_conn::msg_ptr_t msg){
              nsend++;
          }
     };
     ASSERT_EQ(conn2.async_connect("127.0.0.1", 10015,
          [](libcpp::tcp_conn::conn_ptr_t conn, const libcpp::tcp_conn::err_t& err){
              ASSERT_EQ(err.failed(), false);
              my_message* msg1 = new my_message("hello");
              conn->async_send(msg1);
              my_message* msg2 = new my_message("world");
              conn->async_send(msg2);
          }), true);

    io.run();
    t1.join();
    ASSERT_EQ(nsend == 4, true);
}

TEST(tcp_conn, async_recv)
{
    std::thread t1([]() {
        libcpp::tcp_conn::io_t io;
        libcpp::tcp_listener li{io};
        my_message* msg1 = new my_message("hello");
        my_message* msg2 = new my_message("world");
        for (int i = 0; i < 2; i++)
        {
            auto base = li.accept(10016);
            libcpp::tcp_conn sock{io, base};
            ASSERT_EQ(sock.send(msg1), true);
            ASSERT_EQ(sock.send(msg2), true);
        }
        li.close();
    });

    int nrecv = 0;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    libcpp::tcp_conn::io_t io;
    libcpp::tcp_conn conn1{
        io,
        [](libcpp::tcp_conn::conn_ptr_t) {},
        [](libcpp::tcp_conn::conn_ptr_t, libcpp::tcp_conn::msg_ptr_t) {},
        [&nrecv](libcpp::tcp_conn::conn_ptr_t conn, libcpp::tcp_conn::msg_ptr_t arg){
            nrecv++;

            auto msg = static_cast<my_message*>(arg);
            ASSERT_EQ(msg != nullptr, true);
            // if (nrecv == 1)
            //     ASSERT_EQ(msg->text() == std::string("hello"), true);
            // else
            //     ASSERT_EQ(msg->text() == std::string("world"), true);
        }
    };
    ASSERT_EQ(conn1.async_connect("127.0.0.1", 10016,
        [](libcpp::tcp_conn::conn_ptr_t conn, const libcpp::tcp_conn::err_t& err){
            ASSERT_EQ(err.failed(), false);
            my_message* msg1 = new my_message();
            conn->async_recv(msg1);
            my_message* msg2 = new my_message();
            conn->async_recv(msg2);
        }), true);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    libcpp::tcp_conn conn2{
       io,
       [](libcpp::tcp_conn::conn_ptr_t conn) {},
       [](libcpp::tcp_conn::conn_ptr_t conn, libcpp::tcp_conn::msg_ptr_t arg) {},
       [&nrecv](libcpp::tcp_conn::conn_ptr_t conn, libcpp::tcp_conn::msg_ptr_t arg){
           nrecv++;

           auto msg = static_cast<my_message*>(arg);
           ASSERT_EQ(msg != nullptr, true);
        //    if (nrecv == 3)
        //        ASSERT_EQ(msg->text() == "hello", true);
        //    else
        //        ASSERT_EQ(msg->text() == "world", true);
       }
    };
    ASSERT_EQ(conn2.async_connect("127.0.0.1", 10016,
       [](libcpp::tcp_conn::conn_ptr_t conn, const libcpp::tcp_conn::err_t& err){
           ASSERT_EQ(err.failed(), false);
           my_message* msg1 = new my_message();
           conn->async_recv(msg1);
           my_message* msg2 = new my_message();
           conn->async_recv(msg2);
       }), true);

    io.run();
    t1.join();
    ASSERT_EQ(nrecv == 4, true);
}

TEST(tcp_conn, close)
{
    libcpp::tcp_conn::io_t io;
    libcpp::tcp_conn conn1{io};
    ASSERT_EQ(conn1.is_w_closed(), false);
    ASSERT_EQ(conn1.is_r_closed(), false);

    conn1.close();
    ASSERT_EQ(conn1.is_w_closed(), true);
    ASSERT_EQ(conn1.is_r_closed(), true);

    conn1.close();
    ASSERT_EQ(conn1.is_w_closed(), true);
    ASSERT_EQ(conn1.is_r_closed(), true);
}