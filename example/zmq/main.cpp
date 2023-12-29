#include <string>
#include <iostream>

#include <libcpp/net/zmq.hpp>

static zmqpp::context ctx;

static libcpp::zmq_router router{ctx};
static libcpp::zmq_dealer d1{ctx, "d1"};
static libcpp::zmq_dealer d2{ctx, "d2"};

static libcpp::zmq_publisher puber{ctx};
static libcpp::zmq_subscriber suber1{ctx};
static libcpp::zmq_subscriber suber2{ctx};

static libcpp::zmq_pub_sub_broker xbroker{ctx};
static libcpp::zmq_publisher xpuber{ctx};
static libcpp::zmq_subscriber xsuber1{ctx};
static libcpp::zmq_subscriber xsuber2{ctx};

static libcpp::zmq_router m_router{ctx};
static libcpp::zmq_monitor monitor_router{ctx, m_router.socket()};
static libcpp::zmq_dealer m_dealer{ctx, "m_dealer"};
static libcpp::zmq_monitor monitor_dealer{ctx, m_dealer.socket()};

void router_dealer()
{
    std::cout << "router-dealer example" << std::endl;
    router.bind("tcp://127.0.0.1:10086");
    std::thread t1([]() {
        router.loop_start();
    });
    t1.detach();

    d1.set(libcpp::zmq_option::send_high_water_mark, 409600);
    d1.set(libcpp::zmq_option::linger, 1000);
    d1.set(libcpp::zmq_option::identity, "d1");
    d1.connect("tcp://127.0.0.1:10086");
    d1.add_loop_event([]()->bool{
        zmqpp::message msg;
        if (d1.recv(msg))
        {
            std::cout << "dealer1 recv: " << msg.get(2) << std::endl;
            return true;
        }
        return false;
    });
    std::thread t2([&]() {
        d1.loop_start();
    });
    t2.detach();
    std::cout << "dealer1 connect: " << "tcp://127.0.0.1:10086" << std::endl;

    d2.connect("tcp://127.0.0.1:10086");
    d2.add_loop_event([]()->bool{
        zmqpp::message msg;
        if (d2.recv(msg))
        {
            std::cout << "dealer2 recv: " << msg.get(2) << std::endl;
            return true;
        }
        return false;
    });
    std::thread t3([&]() {
        d2.loop_start();
    });
    t3.detach();
    std::cout << "dealer2 connect: " << "tcp://127.0.0.1:10086" << std::endl;

    if (!d1.unsafe_send("d2", "hello")) {
        return;
    }
    std::cout << "dealer1 unsafe_send: " << "hello" << std::endl;

    if (!d2.send("d1", "world")) {
        return;
    }
    std::cout << "dealer2 send: " << "world" << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "router-dealer example end\n" << std::endl;
}

void pub_sub()
{
    std::cout << "pub-sub example" << std::endl;

    puber.bind("tcp://127.0.0.1:10087");

    suber1.connect("tcp://127.0.0.1:10087");
    suber1.add_loop_event([]()->bool{
        auto msg = suber1.recv();
        if (msg.parts() == 0)
        {
            return false;
        }

        std::cout << "suber1 recv: " << msg.get(0) << std::endl;
        return true;
    });
    std::thread t2([]() {
        suber1.loop_start();
    });
    t2.detach();
    std::cout << "suber1 connect: " << "tcp://127.0.0.1:10087" << std::endl;

    suber2.connect("tcp://127.0.0.1:10087");
    suber2.add_loop_event([]()->bool{
        auto msg = suber2.recv();
        if (msg.parts() == 0)
        {
            return false;
        }

        std::cout << "suber2 recv: " << msg.get(0) << std::endl;
        return true;
    });
    std::thread t3([]() {
        suber2.loop_start();
    });
    t3.detach();
    std::cout << "suber2 connect: " << "tcp://127.0.0.1:10087" << std::endl;

    suber1.sub("hello");
    suber2.sub("hello");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    zmqpp::message msg{"hello;001"};
    if (!puber.unsafe_pub(msg)) {
        return;
    }
    std::cout << "puber unsafe_pub: " << "hello;001" << std::endl;

    zmqpp::message msg1{"hello;002"};
    if (!puber.pub(msg1)) {
        return;
    }
    std::cout << "puber pub: " << "hello;002" << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "pub-sub example end\n" << std::endl;
}

void xpub_xsub()
{
    std::cout << "xpub-xsub example" << std::endl;

    std::thread t1([]() {
        xbroker.bind("tcp://127.0.0.1:10088", "tcp://127.0.0.1:10089");
        xbroker.loop_start();
    });
    t1.detach();

    std::thread t2([]() {
        xpuber.bind_broker("tcp://127.0.0.1:10089");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        zmqpp::message msg{"xhello;001"};
        if (!xpuber.unsafe_pub(msg)) {
            return;
        }
        std::cout << "xpuber unsafe_pub: " << "xhello;001" << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        zmqpp::message msg1{"xhello;002"};
        if (!xpuber.pub(msg1)) {
            return;
        }
        std::cout << "xpuber pub: " << "xhello;002" << std::endl;
    });
    t2.detach();

    std::thread t3([]() {
        xsuber1.connect("tcp://127.0.0.1:10088");
        xsuber1.sub("xhello");
        xsuber1.add_loop_event([]()->bool{
            zmqpp::message msg;
            if (!xsuber1.recv(msg))
            {
                return false;
            }
            std::cout << "xsuber1 recv: " << msg.get(0) << std::endl;
            return true;
        });
        xsuber1.loop_start();
    });
    t3.detach();
    std::cout << "xsuber1 connect: " << "tcp://127.0.0.1:10088" << std::endl;

    std::thread t4([]() {
        xsuber2.connect("tcp://127.0.0.1:10088");
        xsuber2.sub("xhello");
        xsuber2.add_loop_event([]()->bool{
            zmqpp::message msg;
            if (!xsuber2.recv(msg))
            {
                return false;
            }
            std::cout << "xsuber2 recv: " << msg.get(0) << std::endl;
            return true;
        });
        xsuber2.loop_start();
    });
    t4.detach();
    std::cout << "xsuber2 connect: " << "tcp://127.0.0.1:10088" << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "xpub-xsub example end\n" << std::endl;
}

void monitor()
{
    std::cout << "monitor example" << std::endl;

    monitor_router.add_event_handler(libcpp::event::listening, [](libcpp::zmq_event & e)-> bool{
        std::cout << "get listening event >>" << std::endl;
        std::cout << "Event = " << e.Event << ", Value = " << e.Value
                  << ", Address = " << e.Address << std::endl;
        return true;
    });
    std::thread t1([]() {
        monitor_router.loop_start();
    });
    t1.detach();

    monitor_dealer.add_event_handler(libcpp::event::connected, [](libcpp::zmq_event & e)-> bool{
        std::cout << "get connected event >>" << std::endl;
        std::cout << "Event = " << e.Event << ", Value = " << e.Value
                  << ", Address = " << e.Address << std::endl;
        return true;
    });
    std::thread t2([]() {
        monitor_dealer.loop_start();
    });
    t2.detach();

    m_router.bind("tcp://127.0.0.1:10090");
    m_dealer.connect("tcp://127.0.0.1:10090");

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "monitor example end" << std::endl;
}

int main(int argc, char* argv[])
{
    router_dealer();

    pub_sub();

    xpub_xsub();

    monitor();

    std::cin.get();
    return 0;
}
