/*
 * dpdk_sw_loop.c
 *
 *  Created on: Jul 6, 2014
 *      Author: Vadim Suraev vadim.suraev@gmail.com
 *  Contains functions for glueing ported Linux kernel's TCP/IP to the DPDK 1.6
 */
#include <specific_includes/dummies.h>
#include <specific_includes/linux/types.h>
#include <specific_includes/linux/bitops.h>
#include <specific_includes/linux/slab.h>
#include <specific_includes/linux/hash.h>
#include <specific_includes/linux/socket.h>
#include <specific_includes/linux/sockios.h>
#include <specific_includes/linux/if_ether.h>
#include <specific_includes/linux/netdevice.h>
#include <specific_includes/linux/etherdevice.h>
#include <specific_includes/linux/ethtool.h>
#include <specific_includes/linux/skbuff.h>
#include <specific_includes/net/net_namespace.h>
#include <specific_includes/net/sock.h>
#include <specific_includes/linux/rtnetlink.h>
#include <specific_includes/net/dst.h>
#include <specific_includes/net/checksum.h>
#include <specific_includes/linux/err.h>
#include <specific_includes/linux/if_arp.h>
#include <specific_includes/linux/if_vlan.h>
#include <specific_includes/linux/ip.h>
#include <specific_includes/net/ip.h>
#include <specific_includes/linux/ipv6.h>
#include <specific_includes/linux/in.h>
#include <specific_includes/linux/inetdevice.h>
#include <specific_includes/linux/hashtable.h>
#include <specific_includes/linux/if_macvlan.h>
#include <specific_includes/linux/if_arp.h>
#include <specific_includes/net/net_namespace.h>
#include <string.h>
#include <rte_config.h>
#include <rte_common.h>
#include <rte_cycles.h>
#include <syslog.h>

typedef struct
{
	int port_number;
}dpdk_dev_priv_t;

static struct rte_ring *loop_ring[1024];
#define TRANSMITTER_RING_SIZE 4096

static int lpbkdpdk_set_mac(struct net_device *netdev, void *p)
{
	struct sockaddr *addr = p;

	if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;

    memcpy(netdev->dev_addr, addr->sa_data, netdev->addr_len);
	return 0;
}

struct rte_mempool *get_direct_pool(uint16_t portid);

static netdev_tx_t lpbkdpdk_xmit_frame(struct sk_buff *skb,
                                  struct net_device *netdev)
{
	int i,offset = 0, network_offset = 0,transport_offset = 0;
	struct rte_mbuf **mbuf,*head,*copied_mbuf;
	dpdk_dev_priv_t *priv = netdev_priv(netdev);
	skb_dst_force(skb);
	head = skb->header_mbuf;
	rte_pktmbuf_data_len(head) = skb_headroom(skb) + skb_headlen(skb);

	/* across all the stack, pkt.data in rte_mbuf is not moved while skb's data is.
	 * now is the time to do that
	 */
	rte_pktmbuf_adj(head,skb_headroom(skb));/* now mbuf data is at eth header */
	
	mbuf = &head->next;
	
	copied_mbuf = rte_pktmbuf_alloc(get_direct_pool(priv->port_number));
	memcpy(rte_pktmbuf_mtod(copied_mbuf,void *),rte_pktmbuf_mtod(head,void *), rte_pktmbuf_data_len(head));
	rte_pktmbuf_data_len(copied_mbuf) = rte_pktmbuf_data_len(head);
	network_offset = skb_network_header(skb) - skb_mac_header(skb);
	transport_offset = skb_transport_header(skb) - skb_mac_header(skb);
//	printf("%s %s %d %d %d\n",__FILE__,__func__,__LINE__,rte_pktmbuf_data_len(copied_mbuf), rte_pktmbuf_data_len(head));
	offset = rte_pktmbuf_data_len(head);
	rte_pktmbuf_free_seg(head);
	skb->header_mbuf = NULL;
	/* typically, the headers are in skb->header_mbuf
	 * while the data is in the frags.
	 * An exception could be ICMP where skb->header_mbuf carries some payload aside headers
	 */
	for (i = 0; i < (int)skb_shinfo(skb)->nr_frags; i++) {
		*mbuf = skb_shinfo(skb)->frags[i].page.p;
		skb_frag_ref(skb,i);	
		memcpy(rte_pktmbuf_mtod(copied_mbuf,char *) + offset,rte_pktmbuf_mtod((*mbuf),void *), rte_pktmbuf_data_len((*mbuf)));
		offset += rte_pktmbuf_data_len((*mbuf));
		rte_pktmbuf_data_len(copied_mbuf) += rte_pktmbuf_data_len((*mbuf));
		//printf("%s %s %d %d %d\n",__FILE__,__func__,__LINE__,rte_pktmbuf_data_len(copied_mbuf), rte_pktmbuf_data_len((*mbuf)));
		rte_pktmbuf_free_seg((*mbuf));
		mbuf = &((*mbuf)->next);
	}	
        *mbuf = NULL;
//	printf("%s %s %d %p %p %d %d %d %d\n",__FILE__,__func__,__LINE__,skb->data,skb->head,skb->len, skb->end,skb->mac_header,skb->network_header);
	kfree_skb(skb);
	skb = build_skb(copied_mbuf,rte_pktmbuf_data_len(copied_mbuf));
	if(unlikely(skb == NULL)) {
		rte_pktmbuf_free(copied_mbuf);
		return NETDEV_TX_OK;
	}

	rte_pktmbuf_pkt_len(copied_mbuf) = offset;
	skb->protocol = eth_type_trans(skb, netdev);
//	skb_set_network_header(skb,network_offset);	
	skb->len = offset;
	skb->ip_summed = CHECKSUM_UNNECESSARY;
	skb->dev = netdev;
	skb_pull(skb,network_offset);
	skb_set_transport_header(skb,transport_offset);
#if 0
	printf("%s %s %d %d %d\n",__FILE__,__func__,__LINE__,rte_pktmbuf_data_len(copied_mbuf), rte_pktmbuf_pkt_len(copied_mbuf));
printf("%s %s %d %p %p %d %d %d %d\n",__FILE__,__func__,__LINE__,skb->data,skb->head,skb->len, skb->end,skb->mac_header,skb->network_header);
{
    for(i = 0;i < offset;i++) {
	if(!(i%8))
		printf("\n");
	printf("  %x",rte_pktmbuf_mtod(copied_mbuf, unsigned char *)[i]);
    }
}
#endif
	rte_ring_enqueue_burst(loop_ring[priv->port_number],&skb,1);
	return NETDEV_TX_OK;
}

/* this function polls DPDK PMD driver for the received buffers.
 * It constructs skb and submits it to the stack.
 * netif_receive_skb is used, we don't have HW interrupt/BH contexts here
 */
static void rx_construct_skb_and_submit(struct net_device *netdev)
{
	int size = MAX_PKT_BURST,ret,i;
	struct sk_buff *skbs[MAX_PKT_BURST];
        
	dpdk_dev_priv_t *priv = netdev_priv(netdev);

	ret = rte_ring_dequeue_burst(loop_ring[priv->port_number],(void **)skbs,size);

	for(i = 0;i < ret;i++) {
#if 0
printf("%s %s %d %d %d %p\n",__FILE__,__func__,__LINE__,i,ret,skbs[i]);
{
    int j;
    for(j = 0;j < rte_pktmbuf_data_len(skbs[i]->header_mbuf);j++) {
	if(!(j%8))
		printf("\n");
	printf("  %x",rte_pktmbuf_mtod(skbs[i]->header_mbuf, unsigned char *)[j]);
    }
}
#endif
		netif_receive_skb(skbs[i]);
//printf("%s %s %d\n",__FILE__,__func__,__LINE__);
	}
}

static int lpbkdpdk_open(struct net_device *netdev)
{
	return 0;
}
static int lpbkdpdk_close(struct net_device *netdev)
{
	return 0;
}

static void lpbkdpdk_netpoll(struct net_device *netdev)
{
	rx_construct_skb_and_submit(netdev);
}

static const struct net_device_ops dpdk_netdev_ops = {
         .ndo_open               = lpbkdpdk_open,
         .ndo_stop               = lpbkdpdk_close,
         .ndo_start_xmit         = lpbkdpdk_xmit_frame,
#ifdef CONFIG_NET_POLL_CONTROLLER
         .ndo_poll_controller    = lpbkdpdk_netpoll,
#endif
	.ndo_set_mac_address    = lpbkdpdk_set_mac
};

void * init_dpdk_sw_loop(int portid)
{
	char ring_name[1024];
	struct net_device *netdev;
	dpdk_dev_priv_t *priv;

	sprintf(ring_name,"dpdkswloop%d",portid);

    	loop_ring[portid] = rte_ring_create(ring_name, TRANSMITTER_RING_SIZE, rte_socket_id(), 0);
    	if(loop_ring == NULL) {
		printf("FATAL %s %d\n",__FILE__,__LINE__);while(1);
    	}
    	netdev = alloc_netdev_mqs(sizeof(dpdk_dev_priv_t),ring_name,ether_setup,1,1);
	if(netdev == NULL) {
		syslog(LOG_ERR,"cannot allocate netdevice %s %d\n",__FILE__,__LINE__);
		return NULL;
	}
	priv = netdev_priv(netdev);
	memset(priv, 0, sizeof(dpdk_dev_priv_t));
	priv->port_number = portid;
	netdev->netdev_ops = &dpdk_netdev_ops;

	netdev->mtu		= /*64 * 1024*/ 1500;
	netdev->hard_header_len	= ETH_HLEN;	/* 14	*/
	netdev->addr_len		= ETH_ALEN;	/* 6	*/
	netdev->tx_queue_len	= 0;
	netdev->type		= ARPHRD_LOOPBACK;	/* 0x0001*/
	netdev->flags		= IFF_LOOPBACK;
//	netdev->priv_flags	       &= ~IFF_XMIT_DST_RELEASE;
	netdev->hw_features	= NETIF_F_ALL_TSO | NETIF_F_UFO;
	netdev->features 		= NETIF_F_SG | NETIF_F_FRAGLIST
		| NETIF_F_ALL_TSO
		| NETIF_F_UFO
		| NETIF_F_HW_CSUM
		| NETIF_F_RXCSUM
		| NETIF_F_HIGHDMA
		| NETIF_F_LLTX
		| NETIF_F_NETNS_LOCAL
		| NETIF_F_VLAN_CHALLENGED
		| NETIF_F_LOOPBACK;

	if(register_netdev(netdev)) {
		syslog(LOG_ERR,"Cannot register netdev %s %d\n",__FILE__,__LINE__);
		return NULL;
	}
	init_net.loopback_dev = netdev;
	return netdev;
}
