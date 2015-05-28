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
#include <string.h>
#include <rte_config.h>
#include <rte_common.h>
#include <rte_cycles.h>

static struct rte_ring *loop_ring = NULL;
#define TRANSMITTER_RING_SIZE 4096

void init_dpdk_sw_loop()
{
	struct socket *sock;
	struct ifreq ifr;
	struct sockaddr_in *sin = (struct sockaddr_in *)&ifr.ifr_addr;

    loop_ring = rte_ring_create("dpdk_sw_loop_ring", TRANSMITTER_RING_SIZE, rte_socket_id(), 0);
    if(loop_ring == NULL) {
	    printf("FATAL %s %d\n",__FILE__,__LINE__);while(1);
    }
    if(sock_create_kern(AF_INET,SOCK_STREAM,0,&sock)) {
    	printf("cannot create socket %s %d\n",__FILE__,__LINE__);
    	goto leave;
    }
    memset(&ifr,0,sizeof(ifr));
    strcpy(ifr.ifr_ifrn.ifrn_name,"lo");
    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = htonl(0x7F000001);
    if(inet_ioctl(sock,SIOCSIFADDR,&ifr)) {
    	printf("Cannot set IF addr %s %d\n",__FILE__,__LINE__);
    	goto leave;
    }
    memset(&ifr,0,sizeof(ifr));
    strcpy(ifr.ifr_ifrn.ifrn_name,"lo");
    ifr.ifr_flags |= IFF_UP;
    if(inet_ioctl(sock,SIOCSIFFLAGS,&ifr)) {
    	printf("Cannot set IF flags %s %d\n",__FILE__,__LINE__);
    	goto leave;
    }
    memset(&ifr,0,sizeof(ifr));
    strcpy(ifr.ifr_ifrn.ifrn_name,"lo");
    if(inet_ioctl(sock,SIOCGIFADDR,&ifr)) {
    	printf("Cannot get IF addr %s %d\n",__FILE__,__LINE__);
    	goto leave;
    }
leave:
    kernel_close(sock);
}

int loop_enqueue(struct rte_mbuf **mbufs,int size)
{
	return rte_ring_enqueue_burst(loop_ring,(void **)mbufs,size);
}
int loop_dequeue(struct rte_mbuf **mbufs,int size)
{
	return rte_ring_dequeue_burst(loop_ring,(void **)mbufs,size);
}
