#include <gtest/gtest.h>
#include <hj/net/zmq.hpp>
#include <thread>
#include <chrono>
#include <vector>
#include <future>
#include <cstring>

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
    ASSERT_EQ(w_ch << msg, hj::zmq::io_status::ok);

    std::string recv_str;
    ASSERT_EQ(r_ch >> recv_str, hj::zmq::io_status::ok);
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
    ASSERT_EQ(prod.push(msg), hj::zmq::io_status::ok);

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
    ASSERT_EQ(prod.send_multipart(send_frames), hj::zmq::io_status::ok);

    std::vector<std::string> recv_frames;
    ASSERT_EQ(cons.recv_multipart(recv_frames), hj::zmq::io_status::ok);

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

    std::promise<void> cons_ready_promise;
    std::promise<void> prod_ready_promise;
    auto               cons_ready = cons_ready_promise.get_future();
    auto               prod_ready = prod_ready_promise.get_future();

    std::thread cons_thread(
        [ctx, &addr, &w_ch_out, p = std::move(cons_ready_promise)]() mutable {
            hj::zmq::consumer cons(ctx);
            EXPECT_NO_THROW(cons.connect(addr));
            p.set_value(); // Consumer connect 完成
            cons.safe_pull(w_ch_out, hj::zmq::cmd::STOP);
        });

    std::thread prod_thread([ctx,
                             &addr,
                             r_ch = w_ch_in.make_r_chan(),
                             p    = std::move(prod_ready_promise)]() mutable {
        hj::zmq::producer prod(ctx);
        EXPECT_NO_THROW(prod.bind(addr));
        p.set_value(); // Producer bind 完成
        prod.safe_push(r_ch, hj::zmq::cmd::STOP);
    });

    cons_ready.wait();
    prod_ready.wait();

    w_ch_in << "msg_1";
    w_ch_in << "msg_2";

    uint8_t          stop_cmd = hj::zmq::cmd::STOP;
    hj::zmq::message stop_msg(sizeof(stop_cmd));
    std::memcpy(stop_msg.data(), &stop_cmd, sizeof(stop_cmd));
    w_ch_in.send(std::move(stop_msg));

    std::string res1, res2;

    ASSERT_EQ(r_ch_out >> res1, hj::zmq::io_status::ok);
    ASSERT_EQ(res1, "msg_1");

    ASSERT_EQ(r_ch_out >> res2, hj::zmq::io_status::ok);
    ASSERT_EQ(res2, "msg_2");

    if(prod_thread.joinable())
        prod_thread.join();
    if(cons_thread.joinable())
        cons_thread.join();
}

TEST(zmq, publisher_subscriber_basic)
{
    auto        ctx       = hj::zmq::context::create();
    std::string pub_addr  = "inproc://test-ps-basic";
    std::string ctrl_addr = "inproc://test-ps-basic-ctrl";

    hj::zmq::socket sync_service(ctx, ZMQ_REP);
    EXPECT_NO_THROW(sync_service.bind(ctrl_addr));

    hj::zmq::publisher pub(ctx);
    EXPECT_NO_THROW(pub.bind(pub_addr));

    std::thread sub_thread([ctx, &pub_addr, &ctrl_addr]() {
        hj::zmq::subscriber sub(ctx);
        EXPECT_NO_THROW(sub.connect(pub_addr));
        EXPECT_NO_THROW(sub.sub(""));

        hj::zmq::socket sync_client(ctx, ZMQ_REQ);
        sync_client.connect(ctrl_addr);
        hj::zmq::message ready_msg("READY");
        sync_client.send(std::move(ready_msg));

        hj::zmq::message ack_msg;
        sync_client.recv(ack_msg);

        auto recv = sub.recv_string();
        ASSERT_TRUE(recv.has_value());
        ASSERT_EQ(recv.value(), "pubmsg");
    });

    hj::zmq::message sync_req;
    sync_service.recv(sync_req);
    hj::zmq::message ack_resp("ACK");
    sync_service.send(std::move(ack_resp));

    std::string msg = "pubmsg";
    ASSERT_EQ(pub.pub(msg), hj::zmq::io_status::ok);

    if(sub_thread.joinable())
        sub_thread.join();
}

TEST(zmq, publisher_subscriber_safe_pipeline)
{
    auto        ctx  = hj::zmq::context::create();
    std::string addr = "inproc://test-safe-ps-pipeline";

    hj::zmq::w_chan w_ch_in(ctx);
    hj::zmq::w_chan w_ch_out(ctx);

    hj::zmq::r_chan r_ch_out = w_ch_out.make_r_chan();

    std::promise<void> sub_ready_promise;
    std::promise<void> pub_ready_promise;
    auto               sub_ready = sub_ready_promise.get_future();
    auto               pub_ready = pub_ready_promise.get_future();

    std::thread sub_thread(
        [ctx, &addr, &w_ch_out, p = std::move(sub_ready_promise)]() mutable {
            hj::zmq::subscriber sub(ctx);
            EXPECT_NO_THROW(sub.connect(addr));
            EXPECT_NO_THROW(sub.sub(""));
            p.set_value(); // Subscriber connect 和 sub 完成
            sub.safe_recv(w_ch_out, hj::zmq::cmd::STOP);
        });

    std::thread pub_thread([ctx,
                            &addr,
                            r_ch = w_ch_in.make_r_chan(),
                            p    = std::move(pub_ready_promise)]() mutable {
        hj::zmq::publisher pub(ctx);
        EXPECT_NO_THROW(pub.bind(addr));
        p.set_value(); // Publisher bind 完成
        pub.safe_pub(r_ch, hj::zmq::cmd::STOP);
    });

    sub_ready.wait();
    pub_ready.wait();

    w_ch_in << "pub_data_1";

    uint8_t          stop_cmd = hj::zmq::cmd::STOP;
    hj::zmq::message stop_msg(sizeof(stop_cmd));
    std::memcpy(stop_msg.data(), &stop_cmd, sizeof(stop_cmd));
    w_ch_in.send(std::move(stop_msg));

    std::string res1;
    ASSERT_EQ(r_ch_out >> res1, hj::zmq::io_status::ok);
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
    hj::zmq::broker bk(ctx, std::move(xpub), std::move(xsub));
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

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
    poller.add(cons, &user_tag, ZMQ_POLLIN);
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

    std::vector<hj::zmq::poller::event_entry> active_events;
    int rc1 = poller.poll(active_events, 10);
    ASSERT_EQ(rc1, 0);
    ASSERT_TRUE(active_events.empty());

    ASSERT_EQ(prod.push("poller_test"), hj::zmq::io_status::ok);

    int rc2 = poller.poll(active_events, 1000);
    ASSERT_GT(rc2, 0);
    ASSERT_EQ(active_events.size(), 1u);
    ASSERT_TRUE(active_events[0].revents & ZMQ_POLLIN);

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
    int *ptr = active_events[0].as_ptr<int>();
    ASSERT_NE(ptr, nullptr);
    ASSERT_EQ(*ptr, 100);
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

    auto recv_data = cons.pull_string();
    ASSERT_TRUE(recv_data.has_value());
    ASSERT_EQ(recv_data.value(), "poller_test");
}

TEST(zmq, broker_steerable_graceful_shutdown)
{
    auto ctx = hj::zmq::context::create();

    hj::zmq::socket xpub(ctx, ZMQ_XPUB);
    hj::zmq::socket xsub(ctx, ZMQ_XSUB);

    std::string xpub_addr = "inproc://xpub-steerable-test";
    std::string xsub_addr = "inproc://xsub-steerable-test";

    hj::zmq::broker bk(ctx, std::move(xpub), std::move(xsub));
    EXPECT_NO_THROW(bk.bind(xpub_addr, xsub_addr));

    std::promise<void> proxy_ready_promise;
    auto               proxy_ready = proxy_ready_promise.get_future();

    std::thread broker_thread(
        [&bk, p = std::move(proxy_ready_promise)]() mutable {
            p.set_value();
            bk.proxy();
        });

    proxy_ready.wait();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_NO_THROW(bk.stop());

    if(broker_thread.joinable())
    {
        broker_thread.join();
    }
}

TEST(zmq, poller_dynamic_modify_and_remove)
{
    auto              ctx = hj::zmq::context::create();
    hj::zmq::producer prod(ctx);
    hj::zmq::consumer cons(ctx);
    std::string       addr = "inproc://test-poller-dynamic";

    EXPECT_NO_THROW(prod.bind(addr));
    EXPECT_NO_THROW(cons.connect(addr));

    hj::zmq::poller poller;
    uintptr_t       tag = 42;

    poller.add(cons, tag, ZMQ_POLLOUT);
    ASSERT_EQ(prod.push("hello"), hj::zmq::io_status::ok);

    std::vector<hj::zmq::poller::event_entry> active_events;
    int rc1 = poller.poll(active_events, 10);
    ASSERT_EQ(rc1, 0);

    ASSERT_TRUE(poller.modify(cons, ZMQ_POLLIN));

    int rc2 = poller.poll(active_events, 1000);
    ASSERT_GT(rc2, 0);
    ASSERT_EQ(active_events.size(), 1u);
    ASSERT_TRUE(active_events[0].revents & ZMQ_POLLIN);

    auto recv_data = cons.pull_string();
    ASSERT_TRUE(recv_data.has_value());

    ASSERT_TRUE(poller.remove(cons));

    ASSERT_EQ(prod.push("world"), hj::zmq::io_status::ok);

    int rc3 = poller.poll(active_events, 10);
    ASSERT_EQ(rc3, 0);
    ASSERT_TRUE(active_events.empty());
}

TEST(zmq, poller_event_entry_predicates)
{
    auto              ctx = hj::zmq::context::create();
    hj::zmq::producer prod(ctx);
    hj::zmq::consumer cons(ctx);
    std::string       addr = "inproc://test-poller-predicates";

    EXPECT_NO_THROW(prod.bind(addr));
    EXPECT_NO_THROW(cons.connect(addr));

    hj::zmq::poller poller;
    poller.add(cons, 100, ZMQ_POLLIN);

    ASSERT_EQ(prod.push("predicate_test"), hj::zmq::io_status::ok);

    std::vector<hj::zmq::poller::event_entry> active_events;
    int rc = poller.poll(active_events, 1000);
    ASSERT_GT(rc, 0);
    ASSERT_EQ(active_events.size(), 1u);

    const auto &ev = active_events[0];

    EXPECT_TRUE(ev.readable());
    EXPECT_FALSE(ev.writable());
    EXPECT_FALSE(ev.error());
}

TEST(zmq, rich_exception_handling)
{
    auto ctx = hj::zmq::context::create();

    try
    {
        hj::zmq::r_chan r_ch(ctx, "invalid_protocol://address");
        FAIL() << "Expected hj::zmq::zmq_error";
    }
    catch(const hj::zmq::zmq_error &ex)
    {
        EXPECT_EQ(ex.operation(), "zmq_connect failed for r_chan");
        EXPECT_EQ(ex.error_number(), EPROTONOSUPPORT);
        EXPECT_STREQ(ex.error_name(), "EPROTONOSUPPORT");

        std::string what_msg = ex.what();
        EXPECT_NE(what_msg.find("zmq_connect failed for r_chan failed:"),
                  std::string::npos);
        EXPECT_NE(what_msg.find("errno="), std::string::npos);
    }
}

TEST(zmq, multipart_message_vector_send_recv)
{
    auto              ctx = hj::zmq::context::create();
    hj::zmq::producer prod(ctx);
    hj::zmq::consumer cons(ctx);
    std::string       addr = "inproc://test-multipart-msg-vector-send";

    EXPECT_NO_THROW(prod.bind(addr));
    EXPECT_NO_THROW(cons.connect(addr));

    std::vector<hj::zmq::message> send_msgs;
    send_msgs.emplace_back("Part_A");
    send_msgs.emplace_back("Part_B");
    send_msgs.emplace_back("Part_C");

    ASSERT_EQ(prod.send_multipart(send_msgs), hj::zmq::io_status::ok);

    std::vector<hj::zmq::message> recv_msgs;
    ASSERT_EQ(cons.recv_multipart(recv_msgs), hj::zmq::io_status::ok);

    ASSERT_EQ(recv_msgs.size(), 3u);
    ASSERT_EQ(recv_msgs[0].to_string_view(), "Part_A");
    ASSERT_EQ(recv_msgs[1].to_string_view(), "Part_B");
    ASSERT_EQ(recv_msgs[2].to_string_view(), "Part_C");
}

TEST(zmq, eagain_nonblocking)
{
    auto ctx = hj::zmq::context::create();

    hj::zmq::socket sock_a(ctx, ZMQ_PAIR);
    sock_a.bind("inproc://eagain_test");

    hj::zmq::message recv_msg;
    ASSERT_EQ(sock_a.recv(recv_msg, ZMQ_DONTWAIT),
              hj::zmq::io_status::would_block);

    hj::zmq::consumer pull_sock(ctx);
    pull_sock.set_opt(ZMQ_RCVHWM, 1);
    pull_sock.bind("inproc://eagain_hwm_test");

    hj::zmq::producer push_sock(ctx);
    push_sock.set_opt(ZMQ_SNDHWM, 1);
    push_sock.connect("inproc://eagain_hwm_test");

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    hj::zmq::io_status status       = hj::zmq::io_status::ok;
    int                pushed_count = 0;

    while(pushed_count < 1000)
    {
        status = push_sock.push("BURST_DATA", ZMQ_DONTWAIT);
        if(status == hj::zmq::io_status::would_block)
            break;

        ASSERT_EQ(status, hj::zmq::io_status::ok);
        pushed_count++;
    }

    ASSERT_EQ(status, hj::zmq::io_status::would_block);
    ASSERT_GT(pushed_count, 0);
}

#ifndef _WIN32
static void dummy_signal_handler(int)
{
}

TEST(zmq, eintr_signal_interruption)
{
    struct sigaction sa;
    sa.sa_handler = dummy_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, nullptr);

    auto            ctx = hj::zmq::context::create();
    hj::zmq::socket sock(ctx, ZMQ_REP);
    sock.bind("inproc://eintr_test");

    std::thread::id main_thread_id = std::this_thread::get_id();

    std::thread killer([main_thread_id]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        pthread_kill(static_cast<pthread_t>(/* native_handle */ pthread_self()),
                     SIGUSR1);
    });

    hj::zmq::message   msg;
    hj::zmq::io_status status = sock.recv(msg);

    if(killer.joinable())
        killer.join();

    EXPECT_TRUE(status == hj::zmq::io_status::interrupted
                || status == hj::zmq::io_status::ok);
}
#endif

TEST(zmq, eterm_context_shutdown)
{
    auto ctx  = hj::zmq::context::create();
    auto sock = std::make_shared<hj::zmq::socket>(ctx, ZMQ_REP);
    sock->bind("inproc://eterm_test");
    sock->unbind_from_thread();

    std::promise<void> thread_started;
    auto               future = thread_started.get_future();

    std::thread recv_thread([sock, p = std::move(thread_started)]() mutable {
        sock->bind_to_current_thread();
        p.set_value();

        hj::zmq::message   msg;
        hj::zmq::io_status st = sock->recv(msg);
        EXPECT_TRUE(st == hj::zmq::io_status::closed
                    || st == hj::zmq::io_status::interrupted);
    });

    future.wait();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    ctx->shutdown();

    if(recv_thread.joinable())
        recv_thread.join();
}

TEST(zmq, socket_and_context_lifetime)
{
    auto ctx = hj::zmq::context::create();
    {
        hj::zmq::socket temp_sock(ctx, ZMQ_PUB);
        temp_sock.bind("inproc://lifetime_test_1");
    }

    EXPECT_NO_THROW({
        hj::zmq::socket new_sock(ctx, ZMQ_SUB);
        new_sock.connect("inproc://lifetime_test_1");
    });

    hj::zmq::socket *dangling_sock_ptr = nullptr;
    {
        auto inner_ctx    = hj::zmq::context::create();
        dangling_sock_ptr = new hj::zmq::socket(inner_ctx, ZMQ_PAIR);
        dangling_sock_ptr->bind("inproc://lifetime_test_2");
    }

    EXPECT_NO_THROW({
        hj::zmq::message msg("alive");
        dangling_sock_ptr->send(std::move(msg), ZMQ_DONTWAIT);
    });

    delete dangling_sock_ptr;
}

TEST(zmq, move_semantics_socket_broker_context)
{
    auto ctx = hj::zmq::context::create();

    hj::zmq::context ctx_moved = std::move(*ctx);
    ASSERT_EQ(ctx->get(), nullptr);
    ASSERT_NE(ctx_moved.get(), nullptr);

    *ctx = std::move(ctx_moved);

    hj::zmq::socket sock1(ctx, ZMQ_PUSH);
    sock1.bind("inproc://move_test_sock");

    hj::zmq::socket sock2 = std::move(sock1);
    ASSERT_EQ(sock1.get(), nullptr);
    ASSERT_NE(sock2.get(), nullptr);

    hj::zmq::consumer cons(ctx);
    cons.connect("inproc://move_test_sock");
    ASSERT_EQ(sock2.send(hj::zmq::message("moved_sock")),
              hj::zmq::io_status::ok);

    auto recved = cons.pull_string();
    ASSERT_TRUE(recved.has_value());
    ASSERT_EQ(recved.value(), "moved_sock");

    hj::zmq::socket xpub(ctx, ZMQ_XPUB);
    hj::zmq::socket xsub(ctx, ZMQ_XSUB);
    hj::zmq::broker bk1(ctx, std::move(xpub), std::move(xsub));

    hj::zmq::broker bk2 = std::move(bk1);
    EXPECT_NO_THROW(bk2.bind("inproc://bk_moved_pub", "inproc://bk_moved_sub"));
}

TEST(zmq, multipart_edge_cases)
{
    auto              ctx = hj::zmq::context::create();
    hj::zmq::producer prod(ctx);
    hj::zmq::consumer cons(ctx);
    std::string       addr = "inproc://test-multipart-edges";

    prod.bind(addr);
    cons.connect(addr);

    {
        std::vector<std::string_view> empty_vector;
        ASSERT_EQ(prod.send_multipart(empty_vector), hj::zmq::io_status::ok);
    }

    {
        std::string binary_data("\x00\x01\x02\xFF\xfe\0test\0data", 15);
        std::string large_frame(1 * 1024 * 1024, 'X');

        std::vector<std::string_view> frames = {"",
                                                "SingleFrame",
                                                binary_data,
                                                large_frame};

        ASSERT_EQ(prod.send_multipart(frames), hj::zmq::io_status::ok);

        std::vector<std::string> recv_frames;
        ASSERT_EQ(cons.recv_multipart(recv_frames), hj::zmq::io_status::ok);

        ASSERT_EQ(recv_frames.size(), 4u);
        EXPECT_EQ(recv_frames[0], "");
        EXPECT_EQ(recv_frames[1], "SingleFrame");
        EXPECT_EQ(recv_frames[2].size(), 15u);
        EXPECT_EQ(memcmp(recv_frames[2].data(), binary_data.data(), 15), 0);
        EXPECT_EQ(recv_frames[3].size(), 1 * 1024 * 1024u);
        EXPECT_EQ(recv_frames[3], large_frame);
    }
}

TEST(zmq, pub_sub_behavior_filtering_and_unsubscribe)
{
    auto        ctx  = hj::zmq::context::create();
    std::string addr = "inproc://pub_sub_test";

    hj::zmq::publisher pub(ctx);
    pub.bind(addr);

    pub.pub("LOST_MESSAGE");

    hj::zmq::subscriber sub(ctx);
    sub.connect(addr);

    sub.sub("TopicA");

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    pub.pub("TopicB_Data");
    pub.pub("TopicA_Data");

    std::string recv_str;
    ASSERT_EQ(sub.recv_string(0).value_or(""), "TopicA_Data");

    sub.unsub("TopicA");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    pub.pub("TopicA_Data_2");

    hj::zmq::message empty_check;
    ASSERT_EQ(sub.recv(empty_check, ZMQ_DONTWAIT),
              hj::zmq::io_status::would_block);
}

TEST(zmq, single_producer_single_consumer_100k)
{
    constexpr uint32_t TOTAL_MESSAGES = 100'000;
    auto               ctx            = hj::zmq::context::create();
    std::string        addr           = "inproc://stress_1p1c_100k";

    std::promise<void> sub_ready;
    auto               sub_ready_future = sub_ready.get_future();

    auto cons_future = std::async(
        std::launch::async,
        [ctx, addr, &sub_ready, TOTAL_MESSAGES]() {
            hj::zmq::consumer cons(ctx);
            cons.set_opt(ZMQ_RCVHWM, 200'000);
            cons.connect(addr);

            sub_ready.set_value();

            uint32_t expected_seq    = 0;
            uint32_t received_count  = 0;
            bool     order_error     = false;
            bool     duplicate_error = false;

            while(received_count < TOTAL_MESSAGES)
            {
                hj::zmq::message msg;
                if(cons.pull(msg) == hj::zmq::io_status::ok)
                {
                    if(msg.size() == sizeof(uint32_t))
                    {
                        uint32_t seq =
                            *static_cast<const uint32_t *>(msg.data());
                        if(seq != expected_seq)
                        {
                            if(seq < expected_seq)
                                duplicate_error = true;
                            else
                                order_error = true;
                        }
                        expected_seq = seq + 1;
                        received_count++;
                    }
                }
            }
            return std::make_tuple(received_count,
                                   order_error,
                                   duplicate_error);
        });

    sub_ready_future.wait();

    auto start_time = std::chrono::high_resolution_clock::now();

    hj::zmq::producer prod(ctx);
    prod.set_opt(ZMQ_SNDHWM, 200'000);
    prod.bind(addr);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    for(uint32_t i = 0; i < TOTAL_MESSAGES; ++i)
    {
        hj::zmq::message msg(sizeof(uint32_t));
        std::memcpy(msg.data(), &i, sizeof(uint32_t));
        ASSERT_EQ(prod.push(std::move(msg)), hj::zmq::io_status::ok);
    }

    auto [recvd_count, order_err, dup_err] = cons_future.get();
    auto end_time = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> diff = end_time - start_time;
    double                        ops  = TOTAL_MESSAGES / diff.count();

    EXPECT_EQ(recvd_count, TOTAL_MESSAGES);
    EXPECT_FALSE(order_err);
    EXPECT_FALSE(dup_err);

    std::cout << "[ STRESS   ] 1P1C 100k Messages Processed in " << diff.count()
              << "s (" << static_cast<uint64_t>(ops) << " msg/sec)"
              << std::endl;
}

TEST(zmq, multi_producer_multi_consumer_concurrent)
{
    constexpr int      NUM_PRODUCERS     = 4;
    constexpr int      NUM_CONSUMERS     = 4;
    constexpr uint32_t MSGS_PER_PRODUCER = 25'000;
    constexpr uint32_t TOTAL_EXPECTED    = NUM_PRODUCERS * MSGS_PER_PRODUCER;

    auto        shared_ctx = hj::zmq::context::create();
    std::string addr       = "inproc://stress_mpmc_concurrent";

    hj::zmq::consumer binder_cons(shared_ctx);
    binder_cons.set_opt(ZMQ_RCVHWM, 100'000);
    binder_cons.bind(addr);

    binder_cons.unbind_from_thread();

    std::atomic<uint32_t>    total_received_count{0};
    std::atomic<bool>        consumers_start{false};
    std::atomic<int>         producers_finished{0};
    std::vector<std::thread> consumer_threads;
    consumer_threads.reserve(NUM_CONSUMERS);

    for(int i = 0; i < NUM_CONSUMERS; ++i)
    {
        consumer_threads.emplace_back([shared_ctx,
                                       addr,
                                       i,
                                       &binder_cons,
                                       &total_received_count,
                                       &consumers_start,
                                       &producers_finished,
                                       TOTAL_EXPECTED,
                                       NUM_PRODUCERS]() {
            std::unique_ptr<hj::zmq::consumer> cons_ptr;
            if(i == 0)
            {
                binder_cons.bind_to_current_thread();
            } else
            {
                cons_ptr = std::make_unique<hj::zmq::consumer>(shared_ctx);
                cons_ptr->set_opt(ZMQ_RCVHWM, 100'000);
                cons_ptr->connect(addr);
            }
            hj::zmq::consumer &cons = (i == 0) ? binder_cons : *cons_ptr;

            while(!consumers_start.load())
            {
                std::this_thread::yield();
            }

            while(true)
            {
                hj::zmq::message msg;
                if(cons.pull(msg, ZMQ_DONTWAIT) == hj::zmq::io_status::ok)
                {
                    total_received_count.fetch_add(1,
                                                   std::memory_order_relaxed);
                } else
                {
                    if(producers_finished.load() == NUM_PRODUCERS)
                    {
                        if(cons.pull(msg, ZMQ_DONTWAIT)
                           != hj::zmq::io_status::ok)
                            break;
                        else
                            total_received_count.fetch_add(
                                1,
                                std::memory_order_relaxed);
                    }
                    std::this_thread::yield();
                }
            }
        });
    }

    consumers_start.store(true);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    std::vector<std::thread> producer_threads;
    producer_threads.reserve(NUM_PRODUCERS);

    auto start_time = std::chrono::high_resolution_clock::now();

    for(int p = 0; p < NUM_PRODUCERS; ++p)
    {
        producer_threads.emplace_back(
            [shared_ctx, addr, p, MSGS_PER_PRODUCER, &producers_finished]() {
                hj::zmq::producer prod(shared_ctx);
                prod.set_opt(ZMQ_SNDHWM, 100'000);
                prod.connect(addr);

                for(uint32_t i = 0; i < MSGS_PER_PRODUCER; ++i)
                {
                    uint64_t payload = (static_cast<uint64_t>(p) << 32) | i;
                    hj::zmq::message msg(sizeof(payload));
                    std::memcpy(msg.data(), &payload, sizeof(payload));

                    while(prod.push(std::move(msg), ZMQ_DONTWAIT)
                          == hj::zmq::io_status::would_block)
                    {
                        std::this_thread::yield();
                        msg = hj::zmq::message(sizeof(payload));
                        std::memcpy(msg.data(), &payload, sizeof(payload));
                    }
                }
                producers_finished.fetch_add(1);
            });
    }

    for(auto &t : producer_threads)
        t.join();
    for(auto &t : consumer_threads)
        t.join();

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end_time - start_time;

    EXPECT_EQ(total_received_count.load(), TOTAL_EXPECTED);
    std::cout << "[ STRESS   ] MPMC Concurrent 100k Messages in "
              << diff.count() << "s ("
              << static_cast<uint64_t>(TOTAL_EXPECTED / diff.count())
              << " msg/sec)" << std::endl;
}

TEST(zmq, destruction_under_load_context_shutdown)
{
    auto        ctx  = hj::zmq::context::create();
    std::string addr = "inproc://destruction_under_load";

    std::atomic<bool>     consumer_running{false};
    std::atomic<uint64_t> messages_processed{0};

    std::thread recv_thread(
        [ctx, addr, &consumer_running, &messages_processed]() {
            hj::zmq::consumer cons(ctx);
            cons.connect(addr);
            consumer_running.store(true);

            while(true)
            {
                hj::zmq::message   msg;
                hj::zmq::io_status st = cons.pull(msg);

                if(st == hj::zmq::io_status::closed
                   || st == hj::zmq::io_status::interrupted)
                {
                    break;
                }
                if(st == hj::zmq::io_status::ok)
                {
                    messages_processed.fetch_add(1);
                }
            }
        });

    std::thread send_thread([ctx, addr, &consumer_running]() {
        hj::zmq::producer prod(ctx);
        prod.bind(addr);

        while(!consumer_running.load())
        {
            std::this_thread::yield();
        }

        uint64_t counter = 0;
        while(true)
        {
            hj::zmq::message msg(sizeof(counter));
            std::memcpy(msg.data(), &counter, sizeof(counter));

            hj::zmq::io_status st = prod.push(std::move(msg));
            if(st == hj::zmq::io_status::closed
               || st == hj::zmq::io_status::interrupted)
            {
                break;
            }
            counter++;
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_GT(messages_processed.load(), 0u);

    ctx->shutdown();

    if(recv_thread.joinable())
    {
        recv_thread.join();
    }
    if(send_thread.joinable())
    {
        send_thread.join();
    }

    SUCCEED();
}