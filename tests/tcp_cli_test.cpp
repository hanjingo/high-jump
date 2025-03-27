#include <gtest/gtest.h>
#include <libcpp/net/tcp.hpp>
#include <thread>
#include <string>
#include <iostream>

class my_message_cli : public libcpp::message
{
public:
    my_message_cli() : _text{std::string()} {}
    my_message_cli(const char* str) : _text{str} {}
    ~my_message_cli() {}

    std::size_t size() override
    {
        return _text.size() + 1;
    }

    std::size_t encode(unsigned char* buf, const std::size_t len) override
    {
        memcpy(buf, _text.c_str(), len);
        return len;
    }

    std::size_t decode(const unsigned char* buf, const std::size_t len) override
    {
        if (len < 2)
            return 0;

        char tmp[len];
        memcpy(tmp, buf, len);
        _text = std::string(tmp);
        return _text.size() + 1;
    }

    std::string text() { return _text; }
    void set_text(const std::string& text) { _text = text; }

private:
    std::string _text;
};

TEST(tcp_client, bind)
{
    std::thread([](){
        static int lambda_entryed_times = 0;
        libcpp::tcp_listener::io_t io;
        my_message_cli* resp = new my_message_cli();
        libcpp::tcp_server srv{
            [resp](libcpp::tcp_server::msg_ptr_t req)->libcpp::tcp_server::msg_ptr_t{
                lambda_entryed_times++;
                my_message_cli* real = (my_message_cli*)(req);
                if (real->text() == std::string("hello"))
                    resp->set_text("world");
                else if (real->text() == std::string("harry"))
                    resp->set_text("potter");
                else 
                    resp->set_text("unknown msg");
                return (libcpp::message*)(resp);
            },
            [](libcpp::tcp_server::msg_ptr_t req)->libcpp::tcp_server::msg_ptr_t{
                my_message_cli* resp = new my_message_cli();
                return (libcpp::message*)(resp);
            }
        };

        libcpp::tcp_listener li{io};
        auto sock = li.accept(10012);
        ASSERT_EQ(sock != nullptr, true);

        auto conn = new libcpp::tcp_conn(io, sock);
        ASSERT_EQ(srv.bind(conn), true);

        io.run_for(std::chrono::milliseconds(500));
        ASSERT_EQ(lambda_entryed_times, 2);
        delete resp; resp = nullptr;
        delete conn; conn = nullptr;
    }).detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    libcpp::tcp_listener::io_t io;
    libcpp::tcp_conn* conn = new libcpp::tcp_conn(io);
    ASSERT_EQ(conn->connect("127.0.0.1", 10012), true);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    static bool hello_check_ok = false;
    static bool harry_check_ok = false;
    libcpp::tcp_client cli{[](libcpp::tcp_server::msg_ptr_t req, libcpp::tcp_server::msg_ptr_t resp){
        my_message_cli* req_real = (my_message_cli*)(req);
        my_message_cli* resp_real = (my_message_cli*)(resp);

        if (req_real->text() == std::string("hello")) {
            hello_check_ok = (resp_real->text() == std::string("world"));
        }
        else if (req_real->text() == std::string("harry")) {
            harry_check_ok = (resp_real->text() == std::string("potter"));
        }
        else {
            hello_check_ok = false;
            harry_check_ok = false;
        }
    }};
    ASSERT_EQ(cli.bind(conn), true);
    
    my_message_cli* msg1 = new my_message_cli("hello");
    my_message_cli* msg2 = new my_message_cli();
    ASSERT_EQ(cli.req(conn, msg1, msg2), true);

    my_message_cli* msg3 = new my_message_cli("harry");
    my_message_cli* msg4 = new my_message_cli();
    ASSERT_EQ(cli.req(conn, msg3, msg4), true);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_EQ(hello_check_ok, true);
    ASSERT_EQ(harry_check_ok, true);
    delete msg1; delete msg2;
    msg1 = nullptr; msg2 = nullptr;
    delete msg3; delete msg4;
    msg3 = nullptr; msg4 = nullptr;
};

TEST(tcp_client, detach)
{

};