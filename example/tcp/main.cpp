#include <iostream>

#include <libcpp/net/tcp.hpp>

int main(int argc, char* argv[])
{
    std::thread t1([]() {
        char buf[1024] = { 0 };
        std::cout << "start sync listening..." << std::endl;
        libcpp::tcp_listener listener{};
        auto sess = listener.accept(10086);
        if (sess == nullptr)
        {
            std::cout << "srv accept connion fail" << std::endl;
            return;
        }
        std::cout << "srv accept connion success" << std::endl;

        auto len = sess->recv(buf, 1024);
        std::cout << "srv recv<< " << std::string(buf, 1024) << std::endl;

        len = sess->send(std::string("world").c_str(), 6);
        std::cout << "srv send>> world" << std::endl;
    });
    t1.detach();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::thread t2([]() {
        char buf[1024] = { 0 };
        std::cout << "start sync connect..." << std::endl;
        libcpp::tcp_socket sess{};
        if (!sess.connect("127.0.0.1", 10086))
        {
            std::cout << "connect 127.0.0.1:10086 fail" << std::endl;
            return;
        }
        std::cout << "connect 127.0.0.1:10086 success" << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        auto len = sess.send(std::string("hello").c_str(), 6);
        std::cout << "cli send hello" << std::endl;

        len = sess.recv(buf, 1024);
        std::cout << "cli recv " << std::string(buf, len) << std::endl;
    });
    t2.detach();

    std::cout << "\n" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::thread t3([]() {
        std::cout << "start async listening..." << std::endl;
        libcpp::tcp_listener listener{};
        listener.async_accept(
            10087,
            [](const libcpp::tcp_listener::err_t& err,
               libcpp::tcp_socket* sess) {
                char buf[1024] = { 0 };
                if (err.failed())
                {
                    std::cout << "srv async accept connion fail" << std::endl;
                    return;
                }
                std::cout << "srv async accept connion success" << std::endl;

                sess->async_recv(
                    buf,
                    1024,
                    [&](const libcpp::tcp_listener::err_t& err,
                        std::size_t sz) {
                        if (err.failed())
                        {
                            std::cout << "srv async recv fail" << std::endl;
                            return;
                        }

                        std::cout << "srv recv<< " << std::string(buf, 1024)
                                  << std::endl;
                    });


                // len = sess->send(std::string("world").c_str(), 6);
                // std::cout << "srv send>> world" << std::endl;
            });
    });
    t3.detach();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::thread t4([]() {
        char buf[1024] = { 0 };
        std::cout << "start async connect..." << std::endl;
        libcpp::tcp_socket sess{};
        sess.async_connect(
            "127.0.0.1",
            10087,
            [](const libcpp::tcp_socket::err_t& err, libcpp::tcp_socket* sess) {
                if (!err.failed())
                {
                    std::cout << "async connect 127.0.0.1:10087 fail"
                              << std::endl;
                    return;
                }
                std::cout << "async connect 127.0.0.1:10087 success"
                          << std::endl;

                sess->async_send(
                    std::string("hello").c_str(),
                    6,
                    [](const libcpp::tcp_socket::err_t& err, std::size_t sz) {
                        if (err.failed())
                        {
                            std::cout << "cli send fail" << std::endl;
                            return;
                        }

                        std::cout << "cli send hello" << std::endl;
                    });
            });

        // len = sess.recv(buf, 1024);
        // std::cout << "cli recv " << std::string(buf, len) << std::endl;
    });
    t4.detach();


    std::cin.get();
    return 0;
}