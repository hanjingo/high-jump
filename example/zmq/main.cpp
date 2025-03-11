#include <string>
#include <iostream>
#include <thread>

#include <libcpp/net/zmq/zmq_chan.hpp>
#include <libcpp/net/zmq/zmq_publisher.hpp>
#include <libcpp/net/zmq/zmq_subscriber.hpp>
#include <libcpp/net/zmq/zmq_pubsub_broker.hpp>

static void* ctx = zmq_ctx_new();

static libcpp::zmq_publisher puber{ctx};
static libcpp::zmq_subscriber suber1{ctx};
static libcpp::zmq_subscriber suber2{ctx};

static libcpp::zmq_pubsub_broker xbroker{ctx};
static libcpp::zmq_publisher xpuber{ctx};
static libcpp::zmq_subscriber xsuber1{ctx};
static libcpp::zmq_subscriber xsuber2{ctx};

void pub_sub()
{
    std::cout << "pub-sub example" << std::endl;

    std::cout << "puber bind tcp://*:10087 with ret = " 
        << puber.bind("tcp://*:10087") << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "suber1 connect: tcp://localhost:10087 with ret = " 
        << suber1.connect("tcp://localhost:10087") << std::endl;
    std::cout << "suber2 connect: tcp://localhost:10087 with ret = " 
        << suber2.connect("tcp://localhost:10087") << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "suber1 sub hello with ret = " << suber1.sub("hello") << std::endl;
    std::cout << "suber2 sub hello with ret = " << suber2.sub("hello") << std::endl;
    std::cout << "suber2 sub world with ret = " << suber2.sub("world") << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::string msg{"hello;001"};
    std::cout << "puber pub with ret = " << puber.pub(msg) 
        << ", msg = " << msg << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::string recv{};
    std::cout << "suber1 ret:" << suber1.recv(recv) << ", recv:" << recv << std::endl;
    std::cout << "suber2 ret:" << suber2.recv(recv) << ", recv:" << recv << std::endl;
    std::cout << std::endl;

    std::string msg1{"hello;002"};
    std::cout << "puber safe_pub with ret = " << puber.safe_pub(msg1) 
        << ", msg1 = " << msg1 << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::string recv1{};
    std::cout << "suber1 ret:" << suber1.recv(recv1) << ", recv1:" << recv1 << std::endl;
    std::cout << "suber2 ret:" << suber2.recv(recv1) << ", recv1:" << recv1 << std::endl;
    std::cout << std::endl;

    std::string msg2{"world;001"};
    std::cout << "puber safe_pub with ret = " << puber.safe_pub(msg2) 
        << ", msg2 = " << msg2 << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::string recv2{};
    std::cout << "suber1 ret:" << suber1.recv(recv2, ZMQ_NOBLOCK) << ", recv2:" << recv2 << std::endl;
    std::cout << "suber2 ret:" << suber2.recv(recv2, ZMQ_NOBLOCK) << ", recv2:" << recv2 << std::endl;

    std::cout << "pub-sub example end\n" << std::endl;
}

void xpub_xsub()
{
    std::cout << "xpub-xsub example" << std::endl;

    std::thread([]() {
        std::cout << "xbroker.bind ret=" << xbroker.bind("tcp://*:10088", "tcp://*:10089") << std::endl;;
        xbroker.poll();
        std::cout << "xbroker poll end" << std::endl;
    }).detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    std::thread([]() {
        std::cout << "xpuber bind broker ret = " << xpuber.bind_broker("tcp://localhost:10089") << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(150));

        int ret = 0;
        std::string msg{"xhello;001"};
        ret = xpuber.pub(msg);
        std::cout << "xpuber pub with ret = " << ret << ", msg = " << msg << std::endl;
        if (ret < 0) {
            return;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        std::string msg1{"xhello;002"};
        ret = xpuber.pub(msg1);
        std::cout << "xpuber pub: with ret = " << ret << ", msg1 = " << msg1 << std::endl;
        if (ret < 0) {
            return;
        }
    }).detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    std::thread([]() {
        std::cout << "xsuber1 connect with ret= " << xsuber1.connect("tcp://localhost:10088") 
            << ", addr = tcp://127.0.0.1:10088" << std::endl;

        std::cout << "xsuber1 sub xhello with ret:" << xsuber1.sub("xhello") << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        std::string recv{};
        int ret = 0;
        while (true) 
        {
            ret = xsuber1.recv(recv);
            std::cout << "xsuber1 recv with ret:" << ret << ", recv:" << recv << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }).detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(198));
    

    // std::thread([]() {
    //     xsuber2.connect("tcp://localhost:10088");
    //     xsuber2.sub("xhello");
    //     std::this_thread::sleep_for(std::chrono::milliseconds(200));

    //     std::string recv{};
    //     while (true)
    //     {
    //         std::cout << "xsuber2 ret:" << xsuber2.recv(recv) << ", recv:" << recv << std::endl;
    //         std::this_thread::sleep_for(std::chrono::milliseconds(200));
    //     }
    // }).detach();
    // std::cout << "xsuber2 connect: " << "tcp://127.0.0.1:10088" << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    std::cout << "xpub-xsub example end\n" << std::endl;
}

int main(int argc, char* argv[])
{
    // pub_sub();

    xpub_xsub();

    std::cin.get();
    return 0;
}