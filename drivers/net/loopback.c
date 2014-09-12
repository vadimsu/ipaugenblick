/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		Pseudo-driver for the loopback interface.
 *
 * Version:	@(#)loopback.c	1.0.4b	08/16/93
 *
 * Authors:	Ross Biro
 *		Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *		Donald Becker, <becker@scyld.com>
 *
 *		Alan Cox	:	Fixed oddments for NET3.014
 *		Alan Cox	:	Rejig for NET3.029 snap #3
 *		Alan Cox	: 	Fixed NET3.029 bugs and sped up
 *		Larry McVoy	:	Tiny tweak to double performance
 *		Alan Cox	:	Backed out LMV's tweak - the linux mm
 *					can't take it...
 *              Michael Griffith:       Don't bother computing the checksums
 *                                      on packets received on the loopback
 *                                      interface.
 *		Alexey Kuznetsov:	Potential hang under some extreme
 *					cases removed.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */
#include <specific_includes/dummies.h>
#include <specific_includes/linux/types.h>
#include <specific_includes/linux/slab.h>
//#include <linux/kernel.h>
//#include <linux/jiffies.h>
//#include <linux/module.h>
//#include <linux/interrupt.h>
//#include <linux/fs.h>
//#include <linux/types.h>
//#include <linux/string.h>
#include <specific_includes/linux/socket.h>
//#include <linux/errno.h>
//#include <linux/fcntl.h>
#include <specific_includes/linux/in.h>

//#include <asm/uaccess.h>
//#include <asm/io.h>

#include <specific_includes/linux/inet.h>
#include <specific_includes/linux/netdevice.h>
#include <specific_includes/linux/etherdevice.h>
#include <specific_includes/linux/skbuff.h>
#include <specific_includes/linux/ethtool.h>
#include <specific_includes/net/sock.h>
#include <specific_includes/net/checksum.h>
#include <specific_includes/linux/if_ether.h>	/* For the statistics structure. */
#include <specific_includes/linux/if_arp.h>	/* For ARPHRD_ETHER */
#include <specific_includes/linux/ip.h>
#include <specific_includes/linux/tcp.h>
#include <specific_includes/linux/percpu.h>
#include <specific_includes/net/net_namespace.h>
//#include <linux/u64_stats_sync.h>

struct pcpu_lstats {
	u64			packets;
	u64			bytes;
//	struct u64_stats_sync	syncp;
};

/*
 * The higher levels take care of making this non-reentrant (it's
 * called with bh's disabled).
 */
static netdev_tx_t loopback_xmit(struct sk_buff *skb,
				 struct net_device *dev)
{
#if 0
	struct pcpu_lstats *lb_stats;
	int len;

	skb_orphan(skb);

	/* Before queueing this packet to netif_rx(),
	 * make sure dst is refcounted.
	 */
	skb_dst_force(skb);

	skb->protocol = eth_type_trans(skb, dev);

	/* it's OK to use per_cpu_ptr() because BHs are off */
	lb_stats = this_cpu_ptr(dev->lstats);
#if 1
{
	static int pr = 1;
	if(!((pr++)%7))
	{
		printf("%s %d %p\n",__FILE__,__LINE__,skb);
		kfree_skb(skb);
		return NETDEV_TX_OK;
	}
printf("%s %d %p\n",__FILE__,__LINE__,skb);
}
#endif
/*{
	struct rte_mbuf *mbuf;
	int i;

	for (i = 0; i < (int)skb_shinfo(skb)->nr_frags; i++)
	{
		mbuf = skb_shinfo(skb)->frags[i].page.p;
		printf("%s %d %p %d\n",__FILE__,__LINE__,mbuf,rte_mbuf_refcnt_read(mbuf));
	}
}*/
	len = skb->len;
	if (likely(netif_rx(skb) == NET_RX_SUCCESS)) {
		//u64_stats_update_begin(&lb_stats->syncp);
		lb_stats->bytes += len;
		lb_stats->packets++;
		//u64_stats_update_end(&lb_stats->syncp);
	}
#else
	int i,pkt_len = 0;
	struct rte_mbuf **mbuf,*head;
	skb_dst_force(skb);
//	printf("%s %d %d %d %d %d %p %d %d\n",
//__FILE__,__LINE__,(int)skb_shinfo(skb)->nr_frags,skb_headlen(skb),skb->len,skb->data_len,skb,skb->protocol,skb_headroom(skb));
	head = skb->header_mbuf;
	head->pkt.data_len = skb_headroom(skb) + skb_headlen(skb);
	rte_pktmbuf_adj(head,skb_headroom(skb));/* now mbuf data is at eth header */
	pkt_len = head->pkt.data_len;
	mbuf = &head->pkt.next;
	skb->header_mbuf = NULL;
	for (i = 0; i < (int)skb_shinfo(skb)->nr_frags; i++)
	{
		*mbuf = skb_shinfo(skb)->frags[i].page.p;
//		printf("%s %d %d %p\n",__FILE__,__LINE__,rte_mbuf_refcnt_read(*mbuf),skb_shinfo(skb)->frags[i].page.p);
		skb_frag_ref(skb,i);
		pkt_len += (*mbuf)->pkt.data_len;
		mbuf = &((*mbuf)->pkt.next);
	}
	head->pkt.pkt_len = pkt_len;
	dpdk_dev_enqueue_for_tx(head);
	kfree_skb(skb);
#endif
	return NETDEV_TX_OK;
}

static struct rtnl_link_stats64 *loopback_get_stats64(struct net_device *dev,
						      struct rtnl_link_stats64 *stats)
{
	u64 bytes = 0;
	u64 packets = 0;
	int i;

	for_each_possible_cpu(i) {
		const struct pcpu_lstats *lb_stats;
		u64 tbytes, tpackets;
		unsigned int start;

		lb_stats = per_cpu_ptr(dev->lstats, i);
//		do {
//			start = u64_stats_fetch_begin_bh(&lb_stats->syncp);
			tbytes = lb_stats->bytes;
			tpackets = lb_stats->packets;
//		} while (u64_stats_fetch_retry_bh(&lb_stats->syncp, start));
		bytes   += tbytes;
		packets += tpackets;
	}
	stats->rx_packets = packets;
	stats->tx_packets = packets;
	stats->rx_bytes   = bytes;
	stats->tx_bytes   = bytes;
	return stats;
}

static u32 always_on(struct net_device *dev)
{
	return 1;
}

static const struct ethtool_ops loopback_ethtool_ops = {
	.get_link		= always_on,
};

static int loopback_dev_init(struct net_device *dev)
{
	int i;
	dev->lstats = alloc_percpu(struct pcpu_lstats);
	if (!dev->lstats)
		return -ENOMEM;

	for_each_possible_cpu(i) {
		struct pcpu_lstats *lb_stats;
		lb_stats = per_cpu_ptr(dev->lstats, i);
//		u64_stats_init(&lb_stats->syncp);
	}
	return 0;
}

static void loopback_dev_free(struct net_device *dev)
{
	dev_net(dev)->loopback_dev = NULL;
	free_percpu(dev->lstats);
	free_netdev(dev);
}

static const struct net_device_ops loopback_ops = {
	.ndo_init      = loopback_dev_init,
	.ndo_start_xmit= loopback_xmit,
	.ndo_get_stats64 = loopback_get_stats64,
};

/*
 * The loopback device is special. There is only one instance
 * per network namespace.
 */
static void loopback_setup(struct net_device *dev)
{
	dev->mtu		= 64 * 1024;
	dev->hard_header_len	= ETH_HLEN;	/* 14	*/
	dev->addr_len		= ETH_ALEN;	/* 6	*/
	dev->tx_queue_len	= 0;
	dev->type		= ARPHRD_LOOPBACK;	/* 0x0001*/
	dev->flags		= IFF_LOOPBACK;
	dev->priv_flags	       &= ~IFF_XMIT_DST_RELEASE;
	dev->hw_features	= NETIF_F_ALL_TSO | NETIF_F_UFO;
	dev->features 		= NETIF_F_SG | NETIF_F_FRAGLIST
		| NETIF_F_ALL_TSO
		| NETIF_F_UFO
		| NETIF_F_HW_CSUM
		| NETIF_F_RXCSUM
		| NETIF_F_HIGHDMA
		| NETIF_F_LLTX
		| NETIF_F_NETNS_LOCAL
		| NETIF_F_VLAN_CHALLENGED
		| NETIF_F_LOOPBACK;
	dev->ethtool_ops	= &loopback_ethtool_ops;
	dev->header_ops		= &eth_header_ops;
	dev->netdev_ops		= &loopback_ops;
	dev->destructor		= loopback_dev_free;
}

/* Setup and register the loopback device. */
static __net_init int loopback_net_init(struct net *net)
{
	struct net_device *dev;
	int err;

	err = -ENOMEM;
	dev = alloc_netdev(0, "lo", loopback_setup);
	if (!dev)
		goto out;

	dev_net_set(dev, net);
	err = register_netdev(dev);
	if (err)
		goto out_free_netdev;

	BUG_ON(dev->ifindex != LOOPBACK_IFINDEX);
	net->loopback_dev = dev;
	return 0;


out_free_netdev:
	free_netdev(dev);
out:
	if (net_eq(net, &init_net))
	{
		while(1)
			printf("loopback: Failed to register netdevice: %d\n", err);
	}
	return err;
}

/* Registered in net/core/dev.c */
struct pernet_operations __net_initdata loopback_net_ops = {
       .init = loopback_net_init,
};
