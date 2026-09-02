#include <gtest/gtest.h>
#include <hj/net/zmq.hpp>
#include <thread>
#include <chrono>
#include <vector>

TEST(zmq, context_and_message_basic)
{
    EXPECT_NO_THROW({
        auto ctx = hj::zmq::context::create();
        ASSERT_NE(ctx->get(), nullptr);
    });

    hj::zmq::message msg1;
    ASSERT_NE(msg1.get(), nullptr);
    ASSERT_EQ(msg1.size(), 0u);

    hj::zmq::message msg2(32);
    ASSERT_NE(msg2.get(), nullptr);
    ASSERT_EQ(msg2.size(), 32u);
    ASSERT_NE(msg2.data(), nullptr);

    hj::zmq::message msg3 = std::move(msg2);
    ASSERT_EQ(msg3.size(), 32u);
    ASSERT_EQ(msg2.size(), 0u);

    static bool freed = false;
    {
        char            *buffer = new char[100];
        hj::zmq::message zero_copy_msg(
            buffer,
            100,
            [](void *data, void *hint) {
                delete[] static_cast<char *>(data);
                bool *p_freed = static_cast<bool *>(hint);
                if(p_freed)
                    *p_freed = true;
            },
            &freed);
        ASSERT_EQ(zero_copy_msg.size(), 100u);
    }
    ASSERT_TRUE(freed);
}

TEST(zmq, exception_handling)
{
    auto ctx = hj::zmq::context::create();
    EXPECT_THROW(
        { hj::zmq::r_chan r_ch(ctx, "invalid_protocol://address"); },
        hj::zmq::zmq_error);
}

TEST(zmq, chan_basic)
{
    auto            ctx = hj::zmq::context::create();
    hj::zmq::w_chan w_ch(ctx);
    hj::zmq::r_chan r_ch = w_ch.make_r_chan();

    std::string msg = "hello zmq chan";
    ASSERT_TRUE(w_ch << msg);

    std::string recv_str;
    ASSERT_TRUE(r_ch >> recv_str);
    ASSERT_EQ(recv_str, msg);
}

TEST(zmq, producer_consumer_basic)
{
    auto              ctx = hj::zmq::context::create();
    hj::zmq::producer prod(ctx);
    hj::zmq::consumer cons(ctx);
    std::string       addr = "inproc://test-pc-basic";

    EXPECT_NO_THROW(prod.bind(addr));
    EXPECT_NO_THROW(cons.connect(addr));

    std::string msg = "producer_consumer_data";
    ASSERT_TRUE(prod.push(msg));

    auto recv = cons.pull_string();
    ASSERT_TRUE(recv.has_value());
    ASSERT_EQ(recv.value(), msg);
}

TEST(zmq, multipart_message_basic)
{
    auto              ctx = hj::zmq::context::create();
    hj::zmq::producer prod(ctx);
    hj::zmq::consumer cons(ctx);
    std::string       addr = "inproc://test-multipart-basic";

    EXPECT_NO_THROW(prod.bind(addr));
    EXPECT_NO_THROW(cons.connect(addr));

    std::vector<std::string_view> send_frames = {"Frame1", "Frame2", "Frame3"};
    ASSERT_TRUE(prod.send_multipart(send_frames));

    std::vector<std::string> recv_frames;
    ASSERT_TRUE(cons.recv_multipart(recv_frames));

    ASSERT_EQ(recv_frames.size(), 3u);
    ASSERT_EQ(recv_frames[0], "Frame1");
    ASSERT_EQ(recv_frames[1], "Frame2");
    ASSERT_EQ(recv_frames[2], "Frame3");
}

TEST(zmq, producer_consumer_safe_pipeline)
{
    auto        ctx  = hj::zmq::context::create();
    std::string addr = "inproc://test-safe-pc-pipeline";

    hj::zmq::w_chan w_ch_in(ctx);
    hj::zmq::w_chan w_ch_out(ctx);

    hj::zmq::r_chan r_ch_out = w_ch_out.make_r_chan();

    std::thread cons_thread([ctx, &addr, &w_ch_out]() {
        hj::zmq::consumer cons(ctx);
        EXPECT_NO_THROW(cons.connect(addr));
        cons.safe_pull(w_ch_out, hj::zmq::control_cmd::STOP);
    });

    std::thread prod_thread(
        [ctx, &addr, r_ch = w_ch_in.make_r_chan()]() mutable {
            hj::zmq::producer prod(ctx);
            EXPECT_NO_THROW(prod.bind(addr));
            prod.safe_push(r_ch, hj::zmq::control_cmd::STOP);
        });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    w_ch_in << "msg_1";
    w_ch_in << "msg_2";

    // 发送 STOP 控制信号来终止管道
    hj::zmq::control_cmd stop_cmd = hj::zmq::control_cmd::STOP;
    hj::zmq::message     stop_msg(&stop_cmd, sizeof(stop_cmd), nullptr);
    w_ch_in.send(stop_msg);

    std::string res1, res2;

    ASSERT_TRUE(r_ch_out >> res1);
    ASSERT_EQ(res1, "msg_1");

    ASSERT_TRUE(r_ch_out >> res2);
    ASSERT_EQ(res2, "msg_2");

    if(prod_thread.joinable())
        prod_thread.join();
    if(cons_thread.joinable())
        cons_thread.join();
}

TEST(zmq, publisher_subscriber_basic)
{
    auto                ctx = hj::zmq::context::create();
    hj::zmq::publisher  pub(ctx);
    hj::zmq::subscriber sub(ctx);
    std::string         addr = "inproc://test-ps-basic";

    EXPECT_NO_THROW(pub.bind(addr));
    EXPECT_NO_THROW(sub.connect(addr));
    EXPECT_NO_THROW(sub.sub(""));

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    std::string msg = "pubmsg";
    ASSERT_TRUE(pub.pub(msg));

    auto recv = sub.recv_string();
    ASSERT_TRUE(recv.has_value());
    ASSERT_EQ(recv.value(), msg);
}

TEST(zmq, publisher_subscriber_safe_pipeline)
{
    auto        ctx  = hj::zmq::context::create();
    std::string addr = "inproc://test-safe-ps-pipeline";

    hj::zmq::w_chan w_ch_in(ctx);
    hj::zmq::w_chan w_ch_out(ctx);

    hj::zmq::r_chan r_ch_out = w_ch_out.make_r_chan();

    std::thread sub_thread([ctx, &addr, &w_ch_out]() {
        hj::zmq::subscriber sub(ctx);
        EXPECT_NO_THROW(sub.connect(addr));
        EXPECT_NO_THROW(sub.sub(""));
        sub.safe_recv(w_ch_out, hj::zmq::control_cmd::STOP);
    });

    std::thread pub_thread(
        [ctx, &addr, r_ch = w_ch_in.make_r_chan()]() mutable {
            hj::zmq::publisher pub(ctx);
            EXPECT_NO_THROW(pub.bind(addr));
            pub.safe_pub(r_ch, hj::zmq::control_cmd::STOP);
        });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    w_ch_in << "pub_data_1";

    // 发送 STOP 信号终止
    hj::zmq::control_cmd stop_cmd = hj::zmq::control_cmd::STOP;
    hj::zmq::message     stop_msg(&stop_cmd, sizeof(stop_cmd), nullptr);
    w_ch_in.send(stop_msg);

    std::string res1;
    ASSERT_TRUE(r_ch_out >> res1);
    ASSERT_EQ(res1, "pub_data_1");

    if(pub_thread.joinable())
        pub_thread.join();
    if(sub_thread.joinable())
        sub_thread.join();
}

TEST(zmq, broker_basic)
{
    auto            ctx = hj::zmq::context::create();
    hj::zmq::socket xpub(ctx, ZMQ_XPUB);
    hj::zmq::socket xsub(ctx, ZMQ_XSUB);
    hj::zmq::broker bk(xpub, xsub);
    std::string     xpub_addr = "inproc://xpub-broker";
    std::string     xsub_addr = "inproc://xsub-broker";

    EXPECT_NO_THROW(bk.bind(xpub_addr, xsub_addr));
}

TEST(zmq, poller_basic)
{
    auto              ctx = hj::zmq::context::create();
    hj::zmq::producer prod(ctx);
    hj::zmq::consumer cons(ctx);
    std::string       addr = "inproc://test-poller-basic";

    EXPECT_NO_THROW(prod.bind(addr));
    EXPECT_NO_THROW(cons.connect(addr));

    hj::zmq::poller poller;
    int             user_tag = 100;
    poller.add(cons, &user_tag, ZMQ_POLLIN);

    std::vector<hj::zmq::poller::event_entry> active_events;
    int rc1 = poller.poll(active_events, 10);
    ASSERT_EQ(rc1, 0);
    ASSERT_TRUE(active_events.empty());

    ASSERT_TRUE(prod.push("poller_test"));

    int rc2 = poller.poll(active_events, 1000);
    ASSERT_GT(rc2, 0);
    ASSERT_EQ(active_events.size(), 1u);
    ASSERT_TRUE(active_events[0].revents & ZMQ_POLLIN);
    ASSERT_EQ(*static_cast<int *>(active_events[0].user_data), 100);

    auto recv_data = cons.pull_string();
    ASSERT_TRUE(recv_data.has_value());
    ASSERT_EQ(recv_data.value(), "poller_test");
}