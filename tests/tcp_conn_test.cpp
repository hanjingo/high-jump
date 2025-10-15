#include <gtest/gtest.h>
#include <hj/net/tcp.hpp>
#include <thread>
#include <string>

struct message
{
    message(std::string str)
        : text{str} {};
    ~message() {};

    std::string text;
};

class my_codec
{
  public:
    my_codec()  = delete;
    ~my_codec() = delete;

    static std::size_t encode(unsigned char          *buf,
                              const std::size_t       len,
                              hj::tcp_conn::msg_ptr_t msg)
    {
        message *p = (message *) msg;
        if(p->text.size() > len)
            return 0;

        memcpy(buf, p->text.data(), p->text.size());
        return p->text.size();
    }

    static std::size_t decode(hj::tcp_conn::msg_ptr_t msg,
                              const unsigned char    *buf,
                              const std::size_t       len)
    {
        if(len < 5)
            return 0;

        message *p = (message *) msg;
        p->text    = std::string(reinterpret_cast<const char *>(buf), 5);
        return 5; // ignore '\n'
    }
};

TEST(tcp_conn, get_flag)
{
    hj::tcp_conn::io_t io;
    hj::tcp_conn       conn1{io};
    ASSERT_EQ(conn1.get_flag() == 0, true);
}

TEST(tcp_conn, set_flag)
{
    hj::tcp_conn::io_t io;
    hj::tcp_conn       conn1{io};

    hj::tcp_conn::flag_t flag1 = 0x1;
    hj::tcp_conn::flag_t flag2 = 0x2;

    conn1.set_flag(flag1);
    ASSERT_EQ(conn1.get_flag() == flag1, true);

    conn1.set_flag(flag2);
    ASSERT_EQ(conn1.get_flag() == (flag1 | flag2), true);
}

TEST(tcp_conn, unset_flag)
{
    hj::tcp_conn::io_t io;
    hj::tcp_conn       conn1{io};

    hj::tcp_conn::flag_t flag1 = 0x1;
    hj::tcp_conn::flag_t flag2 = 0x2;

    conn1.set_flag(flag1);
    ASSERT_EQ(conn1.get_flag() == flag1, true);

    conn1.set_flag(flag2);
    ASSERT_EQ(conn1.get_flag() == (flag1 | flag2), true);

    conn1.unset_flag(flag2);
    ASSERT_EQ(conn1.get_flag() == flag1, true);
}

TEST(tcp_conn, is_connected)
{
    hj::tcp_conn::io_t io;
    hj::tcp_conn       conn1{io};

    ASSERT_EQ(conn1.is_connected(), false);
}

TEST(tcp_conn, is_w_closed)
{
    hj::tcp_conn::io_t io;
    hj::tcp_conn       conn1{io};

    ASSERT_EQ(conn1.is_w_closed(), false);
}

TEST(tcp_conn, is_r_closed)
{
    hj::tcp_conn::io_t io;
    hj::tcp_conn       conn1{io};

    ASSERT_EQ(conn1.is_r_closed(), false);
}

TEST(tcp_conn, set_w_closed)
{
    std::thread t1([]() {
        hj::tcp_conn::io_t io;
        hj::tcp_listener   li{io};
        auto               sock = li.accept(10008);
        ASSERT_EQ(sock == nullptr, false);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        li.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_conn::io_t io;
    hj::tcp_conn       conn1{io};
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
        hj::tcp_conn::io_t io;
        hj::tcp_listener   li{io};
        auto               sock = li.accept(10009);
        ASSERT_EQ(sock == nullptr, false);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        li.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_conn::io_t io;
    hj::tcp_conn       conn1{io};
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
        hj::tcp_conn::io_t io;
        hj::tcp_listener   li{io};
        for(int i = 0; i < 3; i++)
        {
            auto sock = li.accept(10010);
            ASSERT_EQ(sock == nullptr, false);
            sock->close();

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        li.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_conn::io_t io;
    hj::tcp_conn       conn1{io};
    ASSERT_EQ(conn1.connect("127.0.0.1", 10010), true);
    ASSERT_EQ(conn1.connect("127.0.0.1", 10010), false);

    hj::tcp_conn conn2{io};
    ASSERT_EQ(conn2.connect("127.0.0.1", 10010), true);
    ASSERT_EQ(conn2.disconnect(), true);
    ASSERT_EQ(conn2.connect("127.0.0.1", 10010), true);

    t1.join();
}

TEST(tcp_conn, async_connect)
{
    std::thread t1([]() {
        hj::tcp_conn::io_t io;
        hj::tcp_listener   li{io};
        for(int i = 0; i < 2; i++)
        {
            auto sock = li.accept(10011);
            ASSERT_EQ(sock == nullptr, false);
            sock->close();
        }
        li.close();
    });

    hj::tcp_conn::io_t io;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    int          connect_times = 0;
    hj::tcp_conn conn1{io};
    ASSERT_EQ(
        conn1.async_connect("127.0.0.1",
                            10011,
                            [&connect_times](hj::tcp_conn::conn_ptr_t   conn,
                                             const hj::tcp_conn::err_t &err) {
                                ASSERT_EQ(conn != nullptr, true);
                                ASSERT_EQ(err.failed(), false);
                                connect_times++;

                                conn->disconnect();
                            }),
        true);

    hj::tcp_conn conn2{io};
    ASSERT_EQ(
        conn2.async_connect("127.0.0.1",
                            10011,
                            [&connect_times](hj::tcp_conn::conn_ptr_t   conn,
                                             const hj::tcp_conn::err_t &err) {
                                ASSERT_EQ(conn != nullptr, true);
                                ASSERT_EQ(err.failed(), false);
                                connect_times++;

                                conn->disconnect();
                            }),
        true);

    io.run();
    t1.join();
    ASSERT_EQ(connect_times == 2, true);
}

TEST(tcp_conn, disconnect)
{
    std::thread t1([]() {
        hj::tcp_conn::io_t io;
        hj::tcp_listener   li{io};
        for(int i = 0; i < 2; i++)
        {
            auto sock = li.accept(10012);
            ASSERT_EQ(sock == nullptr, false);
            sock->close();
        }
        li.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_conn::io_t io;
    hj::tcp_conn       conn1{io};
    ASSERT_EQ(conn1.disconnect(), false);
    ASSERT_EQ(conn1.connect("127.0.0.1", 10012), true);
    ASSERT_EQ(conn1.disconnect(), true);

    hj::tcp_conn conn2{io};
    ASSERT_EQ(conn2.connect("127.0.0.1", 10012), true);
    ASSERT_EQ(conn2.disconnect(), true);
    ASSERT_EQ(conn2.disconnect(), false);

    t1.join();
}

TEST(tcp_conn, send)
{
    std::thread t1([]() {
        hj::tcp_conn::io_t io;
        hj::tcp_listener   li{io};
        for(int i = 0; i < 2; i++)
        {
            auto         base = li.accept(10013);
            message     *msg1 = new message("");
            message     *msg2 = new message("");
            hj::tcp_conn sock{io, base};
            sock.set_decode_handler(std::bind(my_codec::decode,
                                              std::placeholders::_1,
                                              std::placeholders::_2,
                                              std::placeholders::_3));
            ASSERT_EQ(sock.recv(msg1), true);
            ASSERT_EQ(msg1->text == std::string("hello"), true);

            ASSERT_EQ(sock.recv(msg2), true);
            ASSERT_EQ(msg2->text == std::string("world"), true);
            sock.close();
            delete msg1;
            delete msg2;
        }
        li.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    message *msg1 = new message("hello");
    message *msg2 = new message("world");

    hj::tcp_conn::io_t io;
    hj::tcp_conn       conn1{io};
    conn1.set_send_handler(
        [](hj::tcp_conn::conn_ptr_t conn, hj::tcp_conn::msg_ptr_t msg) {
            (void) conn;
            ASSERT_EQ(msg != nullptr, true);
        });
    conn1.set_encode_handler(std::bind(my_codec::encode,
                                       std::placeholders::_1,
                                       std::placeholders::_2,
                                       std::placeholders::_3));
    ASSERT_EQ(conn1.connect("127.0.0.1", 10013), true);
    ASSERT_EQ(conn1.send(msg1), true);
    ASSERT_EQ(conn1.send(msg2), true);

    hj::tcp_conn conn2{io};
    conn2.set_send_handler(
        [](hj::tcp_conn::conn_ptr_t conn, hj::tcp_conn::msg_ptr_t msg) {
            (void) conn;
            ASSERT_EQ(msg != nullptr, true);
        });
    conn2.set_encode_handler([](unsigned char          *data,
                                const std::size_t       len,
                                hj::tcp_conn::msg_ptr_t msg) -> std::size_t {
        return my_codec::encode(data, len, msg);
    });
    ASSERT_EQ(conn2.connect("127.0.0.1", 10013), true);
    ASSERT_EQ(conn2.send(msg1), true);
    ASSERT_EQ(conn2.send(msg2), true);

    t1.join();

    delete msg1;
    delete msg2;
}

TEST(tcp_conn, recv)
{
    std::thread t1([]() {
        hj::tcp_conn::io_t io;
        hj::tcp_listener   li{io};
        message           *msg1 = new message("hello");
        message           *msg2 = new message("world");
        for(int i = 0; i < 2; i++)
        {
            auto         base = li.accept(10014);
            hj::tcp_conn sock{io, base};
            sock.set_encode_handler(std::bind(my_codec::encode,
                                              std::placeholders::_1,
                                              std::placeholders::_2,
                                              std::placeholders::_3));
            ASSERT_EQ(sock.send(msg1), true);
            ASSERT_EQ(sock.send(msg2), true);
        }
        li.close();

        delete msg1;
        delete msg2;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    message *msg1 = new message("");
    message *msg2 = new message("");

    hj::tcp_conn::io_t io;
    hj::tcp_conn       conn1{io};
    conn1.set_decode_handler(std::bind(my_codec::decode,
                                       std::placeholders::_1,
                                       std::placeholders::_2,
                                       std::placeholders::_3));
    ASSERT_EQ(conn1.connect("127.0.0.1", 10014), true);
    ASSERT_EQ(conn1.recv(msg1), true);
    ASSERT_EQ(msg1->text == "hello", true);
    ASSERT_EQ(conn1.recv(msg2), true);
    ASSERT_EQ(msg2->text == "world", true);

    hj::tcp_conn conn2{io};
    conn2.set_decode_handler(std::bind(my_codec::decode,
                                       std::placeholders::_1,
                                       std::placeholders::_2,
                                       std::placeholders::_3));
    ASSERT_EQ(conn2.connect("127.0.0.1", 10014), true);
    ASSERT_EQ(conn2.recv(msg1), true);
    ASSERT_EQ(msg1->text == "hello", true);
    ASSERT_EQ(conn2.recv(msg2), true);
    ASSERT_EQ(msg2->text == "world", true);

    t1.join();

    delete msg1;
    delete msg2;
}

TEST(tcp_conn, async_send)
{
    std::thread t1([]() {
        hj::tcp_conn::io_t io;
        hj::tcp_listener   li{io};
        for(int i = 0; i < 2; i++)
        {
            auto         base = li.accept(10015);
            message     *msg1 = new message("");
            message     *msg2 = new message("");
            hj::tcp_conn sock{io, base};
            sock.set_decode_handler(std::bind(my_codec::decode,
                                              std::placeholders::_1,
                                              std::placeholders::_2,
                                              std::placeholders::_3));

            ASSERT_EQ(sock.recv(msg1), true);
            ASSERT_EQ(msg1->text == std::string("hello"), true);

            ASSERT_EQ(sock.recv(msg2), true);
            ASSERT_EQ(msg2->text == std::string("world"), true);

            delete msg1;
            delete msg2;
        }
        li.close();
    });

    int nsend = 0;
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    hj::tcp_conn::io_t io;
    hj::tcp_conn       conn1{io};
    conn1.set_send_handler(
        [&nsend](hj::tcp_conn::conn_ptr_t conn, hj::tcp_conn::msg_ptr_t msg) {
            (void) conn;
            nsend++;
            ASSERT_EQ(msg != nullptr, true);

            delete static_cast<message *>(msg);
        });
    conn1.set_encode_handler(std::bind(my_codec::encode,
                                       std::placeholders::_1,
                                       std::placeholders::_2,
                                       std::placeholders::_3));
    ASSERT_EQ(conn1.async_connect("127.0.0.1",
                                  10015,
                                  [](hj::tcp_conn::conn_ptr_t   conn,
                                     const hj::tcp_conn::err_t &err) {
                                      ASSERT_EQ(err.failed(), false);
                                      message *msg1 = new message("hello");
                                      conn->async_send(msg1);
                                      message *msg2 = new message("world");
                                      conn->async_send(msg2);
                                  }),
              true);

    hj::tcp_conn conn2{io};
    conn2.set_send_handler(
        [&nsend](hj::tcp_conn::conn_ptr_t conn, hj::tcp_conn::msg_ptr_t msg) {
            (void) conn;
            nsend++;
            ASSERT_EQ(msg != nullptr, true);

            delete static_cast<message *>(msg);
        });
    conn2.set_encode_handler(std::bind(my_codec::encode,
                                       std::placeholders::_1,
                                       std::placeholders::_2,
                                       std::placeholders::_3));
    ASSERT_EQ(conn2.async_connect("127.0.0.1",
                                  10015,
                                  [](hj::tcp_conn::conn_ptr_t   conn,
                                     const hj::tcp_conn::err_t &err) {
                                      ASSERT_EQ(err.failed(), false);
                                      message *msg1 = new message("hello");
                                      conn->async_send(msg1);
                                      message *msg2 = new message("world");
                                      conn->async_send(msg2);
                                  }),
              true);

    io.run();
    t1.join();
    ASSERT_EQ(nsend == 4, true);
}

TEST(tcp_conn, async_recv)
{
    std::thread t1([]() {
        hj::tcp_conn::io_t io;
        hj::tcp_listener   li{io};
        message           *msg1 = new message("hello");
        message           *msg2 = new message("world");
        for(int i = 0; i < 2; i++)
        {
            auto         base = li.accept(10016);
            hj::tcp_conn sock{io, base};
            sock.set_encode_handler(std::bind(my_codec::encode,
                                              std::placeholders::_1,
                                              std::placeholders::_2,
                                              std::placeholders::_3));
            ASSERT_EQ(sock.send(msg1), true);
            ASSERT_EQ(sock.send(msg2), true);
        }
        li.close();

        delete msg1;
        delete msg2;
    });

    int nrecv = 0;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_conn::io_t io;
    hj::tcp_conn       conn1{io};
    conn1.set_decode_handler(std::bind(my_codec::decode,
                                       std::placeholders::_1,
                                       std::placeholders::_2,
                                       std::placeholders::_3));
    conn1.set_recv_handler(
        [&nrecv](hj::tcp_conn::conn_ptr_t conn, hj::tcp_conn::msg_ptr_t arg) {
            (void) conn;
            nrecv++;
            auto msg = static_cast<message *>(arg);
            ASSERT_EQ(msg != nullptr, true);
            ASSERT_EQ(msg->text.empty(), false);

            delete msg;
        });

    ASSERT_EQ(conn1.async_connect("127.0.0.1",
                                  10016,
                                  [](hj::tcp_conn::conn_ptr_t   conn,
                                     const hj::tcp_conn::err_t &err) {
                                      ASSERT_EQ(err.failed(), false);
                                      message *msg1 = new message("");
                                      conn->async_recv(msg1);
                                      message *msg2 = new message("");
                                      conn->async_recv(msg2);
                                  }),
              true);

    hj::tcp_conn conn2{io};
    conn2.set_decode_handler(std::bind(my_codec::decode,
                                       std::placeholders::_1,
                                       std::placeholders::_2,
                                       std::placeholders::_3));
    conn2.set_recv_handler(
        [&nrecv](hj::tcp_conn::conn_ptr_t conn, hj::tcp_conn::msg_ptr_t arg) {
            (void) conn;
            nrecv++;
            auto msg = static_cast<message *>(arg);
            ASSERT_EQ(msg != nullptr, true);
            ASSERT_EQ(msg->text.empty(), false);

            delete msg;
        });

    ASSERT_EQ(conn2.async_connect("127.0.0.1",
                                  10016,
                                  [](hj::tcp_conn::conn_ptr_t   conn,
                                     const hj::tcp_conn::err_t &err) {
                                      ASSERT_EQ(err.failed(), false);
                                      message *msg1 = new message("");
                                      conn->async_recv(msg1);
                                      message *msg2 = new message("");
                                      conn->async_recv(msg2);
                                  }),
              true);

    io.run();
    t1.join();
    ASSERT_EQ(nrecv == 4, true);
}

TEST(tcp_conn, close)
{
    hj::tcp_conn::io_t io;
    hj::tcp_conn       conn1{io};
    ASSERT_EQ(conn1.is_w_closed(), false);
    ASSERT_EQ(conn1.is_r_closed(), false);

    conn1.close();
    ASSERT_EQ(conn1.is_w_closed(), true);
    ASSERT_EQ(conn1.is_r_closed(), true);

    conn1.close();
    ASSERT_EQ(conn1.is_w_closed(), true);
    ASSERT_EQ(conn1.is_r_closed(), true);
}