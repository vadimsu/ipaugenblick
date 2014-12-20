/*
 * device.c
 *
 *  Created on: Jul 6, 2014
 *      Author: Vadim Suraev vadim.suraev@gmail.com
 *  Contains functions glueing ported Linux kernel's TCP/IP to the DPDK 1.6
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
#include <specific_includes/dpdk_drv_iface.h>
#include <specific_includes/linux/netdev_features.h>
#include <string.h>
#include <rte_config.h>
#include <rte_common.h>
#include <rte_cycles.h>
#include <rte_timer.h>

typedef struct
{
	int port_number;
}dpdk_dev_priv_t;

/* this function polls DPDK PMD driver for the received buffers.
 * It constructs skb and submits it to the stack.
 * netif_receive_skb is used, we don't have HW interrupt/BH contexts here
 */
static void rx_construct_skb_and_submit(struct net_device *netdev)
{
	int size = MAX_PKT_BURST,ret,i,frag_idx;
	struct rte_mbuf *mbufs[MAX_PKT_BURST],*m;
	struct sk_buff *skb;
        struct ethhdr *eth;
	dpdk_dev_priv_t *priv = netdev_priv(netdev);

	ret = dpdk_dev_get_received(priv->port_number,mbufs,size);
	if(unlikely(ret <= 0)) {
		return;
	}
	for(i = 0;i < ret;i++) {
		skb = build_skb(mbufs[i],mbufs[i]->pkt.data_len);
		if(unlikely(skb == NULL)) {
			rte_pktmbuf_free(mbufs[i]);
			continue;
		}
		skb->len = mbufs[i]->pkt.data_len;
#if 0 /* once the receive will scatter the packets, this will be needed */
		m = mbufs[i]->pkt.next;
		frag_idx = 0;
		while(unlikely(m)) {
			struct page pg;

			pg.mbuf = m;
			skb_add_rx_frag(skb,frag_idx,&pg,0,m->pkt.data_len,m->pkt.data_len);
			frag_idx++;
			m = m->pkt.next;
		}
#endif
                /* removing vlan tagg */
                eth = (struct ethhdr *)skb->data;
                if(eth->h_proto == htons(ETH_P_8021Q)) {
                    unsigned i = 0;
                    uint8_t *hdr_str = (uint8_t *) skb->data;
                    for(i = 0; i < 12 ; i++)
                        hdr_str[(11 - i) + 4] = hdr_str[(11-i)];
                    skb->data = &hdr_str[4];
                    skb->len = skb->len - 4;
                }
		skb->protocol = eth_type_trans(skb, netdev);
		skb->ip_summed = CHECKSUM_UNNECESSARY;
		skb->dev = netdev;
		netif_receive_skb(skb);
	}
}

static int dpdk_open(struct net_device *netdev)
{
#ifdef DPDK_SW_LOOP
#endif
	return 0;
}
static int dpdk_close(struct net_device *netdev)
{
	return 0;
}
static netdev_tx_t dpdk_xmit_frame(struct sk_buff *skb,
                                  struct net_device *netdev)
{
	int i,pkt_len = 0;
	struct rte_mbuf **mbuf,*head;
	dpdk_dev_priv_t *priv = netdev_priv(netdev);
	skb_dst_force(skb);

	head = skb->header_mbuf;
	head->pkt.data_len = skb_headroom(skb) + skb_headlen(skb);
	/* across all the stack, pkt.data in rte_mbuf is not moved while skb's data is.
	 * now is the time to do that
	 */
	rte_pktmbuf_adj(head,skb_headroom(skb));/* now mbuf data is at eth header */
	pkt_len = head->pkt.data_len;
	head->pkt.data = skb->data;
	mbuf = &head->pkt.next;
	skb->header_mbuf = NULL;
	head->pkt.nb_segs = 1 + skb_shinfo(skb)->nr_frags;
	/* typically, the headers are in skb->header_mbuf
	 * while the data is in the frags.
	 * An exception could be ICMP where skb->header_mbuf carries some payload aside headers
	 */
	for (i = 0; i < (int)skb_shinfo(skb)->nr_frags; i++) {
		*mbuf = skb_shinfo(skb)->frags[i].page.p;
		skb_frag_ref(skb,i);
		pkt_len += (*mbuf)->pkt.data_len;
		mbuf = &((*mbuf)->pkt.next);
	}
        *mbuf = NULL;
	head->pkt.pkt_len = pkt_len;
#ifdef GSO
        if ((skb->protocol == htons(ETH_P_IP))&&(ip_hdr(skb)->protocol == IPPROTO_TCP)&&(i> 0)) {
                 struct iphdr *iph = ip_hdr(skb);
                 head->pkt.vlan_macip.data = skb_network_header_len(skb) | (skb_network_offset(skb) << 9);
                 head->pkt.hash.fdir.hash = tcp_hdrlen(skb); /* ugly, but no other place */
                 head->pkt.hash.fdir.id = skb_transport_offset(skb);
                 head->ol_flags = PKT_TX_TCP_CKSUM | PKT_TX_IP_CKSUM;
                 iph->tot_len = 0;
                 iph->check = 0; 
                 tcp_hdr(skb)->check = ~csum_tcpudp_magic(iph->saddr,
                                                          iph->daddr, 0,
                                                          IPPROTO_TCP,
                                                          0);
        }
#endif
	/* this will pass the mbuf to DPDK PMD driver */
	dpdk_dev_enqueue_for_tx(priv->port_number,head);
	kfree_skb(skb);
	return NETDEV_TX_OK;
}
static struct rtnl_link_stats64 *dpdk_get_stats64(struct net_device *netdev,
                                                 struct rtnl_link_stats64 *stats)
{
	return NULL;
}
static void dpdk_set_rx_mode(struct net_device *netdev)
{
}
static int dpdk_set_mac(struct net_device *netdev, void *p)
{
	struct sockaddr *addr = p;

	if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;

    memcpy(netdev->dev_addr, addr->sa_data, netdev->addr_len);
	return 0;
}
static int dpdk_change_mtu(struct net_device *netdev, int new_mtu)
{
	return 0;
}
static int dpdk_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd)
{
	return 0;
}
static void dpdk_tx_timeout(struct net_device *netdev)
{
}
static int dpdk_vlan_rx_add_vid(struct net_device *netdev,
                               __be16 proto, u16 vid)
{
	return 0;
}
static int dpdk_vlan_rx_kill_vid(struct net_device *netdev,
                                __be16 proto, u16 vid)
{
	return 0;
}
static int dpdk_ndo_set_vf_mac(struct net_device *netdev, int vf, u8 *mac)
{
	return 0;
}
static int dpdk_ndo_set_vf_vlan(struct net_device *netdev,
                               int vf, u16 vlan, u8 qos)
{
	return 0;
}
static int dpdk_ndo_set_vf_bw(struct net_device *netdev, int vf, int tx_rate)
{
	return 0;
}
static int dpdk_ndo_set_vf_spoofchk(struct net_device *netdev, int vf,
                                   bool setting)
{
	return 0;
}
static int dpdk_ndo_get_vf_config(struct net_device *netdev,
                                 int vf, struct ifla_vf_info *ivi)
{
	return 0;
}
static void dpdk_netpoll(struct net_device *netdev)
{
	dpdk_dev_priv_t *priv = netdev_priv(netdev);
	/* check for received packets.
	 * Then check if there are mbufs ready for tx, but not submitted yet
	 */
	rx_construct_skb_and_submit(netdev);
}
static netdev_features_t dpdk_fix_features(struct net_device *netdev,
         netdev_features_t features)
{
	netdev->features = features;
	return netdev->features;
}
static int dpdk_set_features(struct net_device *netdev,
         netdev_features_t features)
{
	netdev->features = features;
	return 0;
}
static const struct net_device_ops dpdk_netdev_ops = {
         .ndo_open               = dpdk_open,
         .ndo_stop               = dpdk_close,
         .ndo_start_xmit         = dpdk_xmit_frame,
         .ndo_get_stats64        = dpdk_get_stats64,
         .ndo_set_rx_mode        = dpdk_set_rx_mode,
         .ndo_set_mac_address    = dpdk_set_mac,
         .ndo_change_mtu         = dpdk_change_mtu,
         .ndo_do_ioctl           = dpdk_ioctl,
         .ndo_tx_timeout         = dpdk_tx_timeout,
         .ndo_validate_addr      = eth_validate_addr,
         .ndo_vlan_rx_add_vid    = dpdk_vlan_rx_add_vid,
         .ndo_vlan_rx_kill_vid   = dpdk_vlan_rx_kill_vid,
         .ndo_set_vf_mac         = dpdk_ndo_set_vf_mac,
         .ndo_set_vf_vlan        = dpdk_ndo_set_vf_vlan,
         .ndo_set_vf_tx_rate     = dpdk_ndo_set_vf_bw,
         .ndo_set_vf_spoofchk    = dpdk_ndo_set_vf_spoofchk,
         .ndo_get_vf_config      = dpdk_ndo_get_vf_config,
#ifdef CONFIG_NET_POLL_CONTROLLER
         .ndo_poll_controller    = dpdk_netpoll,
#endif
         .ndo_fix_features       = dpdk_fix_features,
         .ndo_set_features       = dpdk_set_features,
};

void user_transmitted_callback(struct rte_mbuf *mbuf)
{
	rte_pktmbuf_free_seg(mbuf);
}
void add_dev_addr(void *netdev,int instance,char *ip_addr,char *ip_mask)
{
	struct socket *sock;
	struct ifreq ifr;
	struct rtentry rt;
	struct arpreq r;
	struct net_device *dev;
	struct sockaddr_in *sin = (struct sockaddr_in *)&ifr.ifr_addr;
	int i;
	struct sockaddr macaddr;

	dev = (struct net_device *)netdev;
	if(netdev == NULL) {
		printf("netdev is NULL%s %d\n",__FILE__,__LINE__);
		goto leave;
	}
	memset(&ifr,0,sizeof(ifr));
	sprintf(ifr.ifr_ifrn.ifrn_name,"%s:%d",dev->name,instance);
	if(sock_create_kern(AF_INET,SOCK_STREAM,0,&sock)) {
		printf("cannot create socket %s %d\n",__FILE__,__LINE__);
		goto leave;
	}
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = inet_addr(ip_addr);
	if(inet_ioctl(sock,SIOCSIFADDR,&ifr)) {
		printf("Cannot set IF addr %s %d\n",__FILE__,__LINE__);
		goto leave;
	}
	memset(&ifr,0,sizeof(ifr));
	strcpy(ifr.ifr_ifrn.ifrn_name,dev->name);
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = inet_addr(ip_mask);
	if(inet_ioctl(sock,SIOCSIFNETMASK,&ifr)) {
		printf("Cannot set IF mask %s %d\n",__FILE__,__LINE__);
		goto leave;
	}
	memset(&ifr,0,sizeof(ifr));
	sprintf(ifr.ifr_ifrn.ifrn_name,"%s:%d",dev->name,instance);
	ifr.ifr_flags |= IFF_UP;
	if(inet_ioctl(sock,SIOCSIFFLAGS,&ifr)) {
		printf("Cannot set IF flags %s %d\n",__FILE__,__LINE__);
		goto leave;
	}
	memset(&ifr,0,sizeof(ifr));
	sprintf(ifr.ifr_ifrn.ifrn_name,"%s:%d",dev->name,instance);
	if(inet_ioctl(sock,SIOCGIFADDR,&ifr)) {
		printf("Cannot get IF addr %s %d\n",__FILE__,__LINE__);
		goto leave;
	}
leave:
	kernel_close(sock);
}
void set_dev_addr(void *netdev,char *mac_addr,char *ip_addr,char *ip_mask)
{
	struct socket *sock;
	struct ifreq ifr;
	struct rtentry rt;
	struct arpreq r;
	struct net_device *dev;
	struct sockaddr_in *sin = (struct sockaddr_in *)&ifr.ifr_addr;
	int i;
	struct sockaddr macaddr;

	dev = (struct net_device *)netdev;
	if(netdev == NULL) {
		printf("netdev is NULL%s %d\n",__FILE__,__LINE__);
		goto leave;
	}
        dev->mtu = 1500;
#ifdef GSO
        dev->gso_max_segs = 2;
        dev->gso_max_size = 4096;
#endif
	memcpy(macaddr.sa_data,mac_addr,ETH_ALEN);
	memset(&ifr,0,sizeof(ifr));
	strcpy(ifr.ifr_ifrn.ifrn_name,dev->name);
	if(sock_create_kern(AF_INET,SOCK_STREAM,0,&sock)) {
		printf("cannot create socket %s %d\n",__FILE__,__LINE__);
		goto leave;
	}
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = inet_addr(ip_addr);
	if(inet_ioctl(sock,SIOCSIFADDR,&ifr)) {
		printf("Cannot set IF addr %s %d\n",__FILE__,__LINE__);
		goto leave;
	}
	memset(&ifr,0,sizeof(ifr));
	strcpy(ifr.ifr_ifrn.ifrn_name,dev->name);
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = inet_addr(ip_mask);
	if(inet_ioctl(sock,SIOCSIFNETMASK,&ifr)) {
		printf("Cannot set IF mask %s %d\n",__FILE__,__LINE__);
		goto leave;
	}
	eth_mac_addr(dev,&macaddr);
	memset(&ifr,0,sizeof(ifr));
	strcpy(ifr.ifr_ifrn.ifrn_name,dev->name);
	ifr.ifr_flags |= IFF_UP;
	if(inet_ioctl(sock,SIOCSIFFLAGS,&ifr)) {
		printf("Cannot set IF flags %s %d\n",__FILE__,__LINE__);
		goto leave;
	}
	memset(&ifr,0,sizeof(ifr));
	strcpy(ifr.ifr_ifrn.ifrn_name,dev->name);
	if(inet_ioctl(sock,SIOCGIFADDR,&ifr)) {
		printf("Cannot get IF addr %s %d\n",__FILE__,__LINE__);
		goto leave;
	}
leave:
	kernel_close(sock);
}

void *create_netdev(int port_num)
{
	struct net_device *netdev;
	dpdk_dev_priv_t *priv;
    char dev_name[20];

    sprintf(dev_name,"dpdk_if%d",port_num);
	netdev = alloc_netdev_mqs(sizeof(dpdk_dev_priv_t),dev_name,ether_setup,1,1);
	if(netdev == NULL) {
		printf("cannot allocate netdevice %s %d\n",__FILE__,__LINE__);
		return NULL;
	}
	priv = netdev_priv(netdev);
	memset(priv, 0, sizeof(dpdk_dev_priv_t));
	priv->port_number = port_num;
	netdev->netdev_ops = &dpdk_netdev_ops;
#ifdef GSO
	netdev->features = NETIF_F_SG | NETIF_F_GSO | NETIF_F_FRAGLIST;
#else
        netdev->features = NETIF_F_SG | NETIF_F_FRAGLIST/*|NETIF_F_V4_CSUM not yet!*/;
#endif
	netdev->hw_features = 0;

	netdev->vlan_features = 0;

	if(register_netdev(netdev)) {
		printf("Cannot register netdev %s %d\n",__FILE__,__LINE__);
		return NULL;
	}
	return netdev;
}
extern uint64_t received;
extern uint64_t transmitted;
extern uint64_t tx_dropped;
void dpdk_dev_print_stats()
{
	printf("PHY received %"PRIu64" transmitted %"PRIu64" dropped %"PRIu64"\n",received,transmitted,tx_dropped);
}
