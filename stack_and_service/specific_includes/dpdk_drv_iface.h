
#ifndef __DPDK_DRV_IFACE_H__
#define __DPDK_DRV_IFACE_H__

void *create_netdev(int port_num);

void add_dev_addr(void *netdev,int instance,char *ip_addr,char *ip_mask);

void set_dev_addr(void *netdev,char *mac_addr,char *ip_addr,char *ip_mask);

//void user_transmitted_callback(struct rte_mbuf *mbuf,struct socket *sock);

void run_rx_thread(int portnum);
void run_tx_thread(int portnum);

void dpdk_dev_init_rx_ring(int port_num);
int dpdk_dev_get_received(int port_num,struct rte_mbuf **tbl,int tbl_size);
void dpdk_dev_init_tx_ring(int port_num);
void dpdk_dev_enqueue_for_tx(int port_num,struct rte_mbuf *m);

void transmit_pending(int port_num);

int get_tx_overflow(int port_num);

#endif /* __DPDK_DRV_IFACE_H__ */
