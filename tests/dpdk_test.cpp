#include <gtest/gtest.h>
#include <libcpp/hardware/dpdk.h>

TEST(dpdk, dpdk_eal_init)
{
    int argc = 1;
    const char* arg0 = "tests";
    char* argv[] = {const_cast<char*>(arg0), nullptr};
    int ret = rte_eal_init(argc, argv);
    if (ret < 0) {
        std::cerr << "DPDK EAL init failed, check hugepage and permissions!" << std::endl;
    }
    ASSERT_GE(ret, 0);
}

TEST(dpdk, dpdk_eth_dev_count_avail)
{
    ASSERT_EQ(rte_eth_dev_count_avail() >= 0, true);
}

TEST(dpdk, dpdk_eth_dev_configure)
{
    uint16_t nb_ports = rte_eth_dev_count_avail();
    if (nb_ports == 0) 
    {
        std::cerr << "ERROR: No available Ethernet ports found!" << std::endl;
        return;
    }

    uint16_t port_id = 0;
    struct rte_eth_conf port_conf = {};
    port_conf.rxmode.max_lro_pkt_size = RTE_ETHER_MAX_LEN;

    int ret = rte_eth_dev_configure(port_id, 1, 1, &port_conf);
    ASSERT_GE(ret, 0);
}

// TEST(dpdk, dpdk_pktmbuf_pool_create)
// {
//     const char* pool_name = "test_pool";
//     unsigned int num_mbufs = 8192;
//     unsigned int mbuf_size = RTE_MBUF_DEFAULT_BUF_SIZE;
//     unsigned int socket_id = rte_socket_id();

//     struct rte_mempool* mbuf_pool = rte_pktmbuf_pool_create(pool_name, num_mbufs, 0, 0, mbuf_size, socket_id);
//     ASSERT_NE(mbuf_pool, nullptr);
// }

// TEST(dpdk, dpdk_eth_dev_start)
// {
//     uint16_t port = 0;
//     int ret = rte_eth_dev_start(port);
//     ASSERT_GE(ret, 0);
// }

// TEST(dpdk, dpdk_eth_rx_burst)
// {
//     uint16_t port = 0;
//     struct rte_mbuf* bufs[32];
//     unsigned int nb_rx = rte_eth_rx_burst(port, 0, bufs, 32);
//     ASSERT_GE(nb_rx, 0);
// }

// TEST(dpdk, dpdk_dev_stop)
// {
//     uint16_t port = 0;
//     rte_eth_dev_stop(port);
//     int ret = rte_eth_dev_is_valid_port(port);
//     ASSERT_EQ(ret, 0);
// }

// TEST(dpdk, dpdk_dev_close)
// {
//     uint16_t port = 0;
//     int ret = rte_eth_dev_close(port);
//     ASSERT_GE(ret, 0);
// }