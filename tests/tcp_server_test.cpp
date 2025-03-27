#include <gtest/gtest.h>
#include <libcpp/net/tcp.hpp>
#include <thread>
#include <string>
#include <iostream>

class my_message : public libcpp::message
{
public:
    my_message() : _text{std::string()} {}
    my_message(const char* str) : _text{str} {}
    ~my_message() {}

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
        std::cout << "I AM HERE decode with len=" << len << std::endl;
        if (len < 2)
            return 0;

        std::cout << "I AM HERE decode start copy" << std::endl;
        char tmp[len];
        memcpy(tmp, buf, len);
        std::cout << "I AM HERE decode end copy" << std::endl;
        _text = std::string(tmp);
        std::cout << "I AM HERE decode _text=" << _text << std::endl;
        return _text.size();
    }

    std::string text() { return _text; }
    void set_text(const std::string& text) { _text = text; }

private:
    std::string _text;
};

TEST(tcp_server, bind)
{
    std::thread([](){
        static int lambda_entryed_times = 0;
        libcpp::tcp_listener::io_t io;
        my_message* resp = new my_message();
        libcpp::tcp_server srv{
            [resp, lambda_entryed_times](libcpp::tcp_server::msg_ptr_t req)->libcpp::tcp_server::msg_ptr_t{
                lambda_entryed_times++;
                my_message* real = (my_message*)(req);
                if (real->text() == std::string("hello"))
                    resp->set_text("world");
                else if (real->text() == std::string("harry"))
                    resp->set_text("potter");
                else 
                    resp->set_text("unknown msg");
                return (libcpp::message*)(resp);
            },
            [resp](libcpp::tcp_server::msg_ptr_t req)->libcpp::tcp_server::msg_ptr_t{
                return resp;
            }
        };

        libcpp::tcp_listener li{io};
        auto sock = li.accept(10011);
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
    libcpp::tcp_conn conn{io};
    ASSERT_EQ(conn.connect("127.0.0.1", 10011), true);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    my_message* msg1 = new my_message("hello");
    ASSERT_EQ(conn.send(msg1), true);

    my_message* msg2 = new my_message();
    ASSERT_EQ(conn.recv(msg2), true);
    ASSERT_EQ(msg2->text() == std::string("world"), true);

    std::cout << "fuck1" << std::endl;
    my_message* msg3 = new my_message("harry");
    ASSERT_EQ(conn.send(msg3), true);
    std::cout << "fuck2" << std::endl;

    my_message* msg4 = new my_message();
    ASSERT_EQ(conn.recv(msg4), true);
    ASSERT_EQ(msg4->text() == std::string("potter"), true);
    std::cout << "fuck3" << std::endl;

    delete msg1; delete msg2;
    msg1 = nullptr; msg2 = nullptr;
    delete msg3; delete msg4;
    msg3 = nullptr; msg4 = nullptr;
};

TEST(tcp_server, unbind)
{

};