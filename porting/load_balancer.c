/*
 * load_balancer.c
 *
 *  Created on: Jul 6, 2014
 *      Author: Vadim Suraev vadim.suraev@gmail.com
 *  dispatches skbs to cores
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
#include <specific_includes/linux/skbuff.h>
#include <rte_config.h>
#include <getopt.h>
#include <specific_includes/dpdk_drv_iface.h>
#include <pools.h>
#include <porting/load_balancer.h>
#include <porting/load_balancer2.h>

static uint16_t load_balancer_cpu_mask = 0;
static uint8_t load_balancer_total_cpus = 0;
static uint8_t load_balancer_poller_core_id = 0;

static uint8_t  idx_to_core_ids[MAXCPU];
static uint64_t load_balancer_stats[MAXCPU];
static uint8_t load_balancer_percpu_arp[MAXCPU];
static uint8_t load_balancer_next_core_for_udp = 0;

/* This is a dirty trick for one-to-one setup, must be rewritten */
void on_arp_request()
{
    load_balancer_percpu_arp[rte_lcore_id()] = 1;
}

static int load_balancer_get_core_for_arp(struct sk_buff *skb)
{
	const struct arphdr *arp;
        int i;
	
	arp = (const struct arphdr *)skb->data;
	if(arp->ar_op != htons(ARPOP_REPLY)) {
		return load_balancer_poller_core_id;
	}
        for(i = 0;i < rte_lcore_count();i++) {
            if(load_balancer_percpu_arp[i]) {
                load_balancer_percpu_arp[i] = 0;
                return i;
            }
        }
        return load_balancer_poller_core_id;
}
int load_balancer_get_core_to_process_packet(struct sk_buff *skb)
{
	struct iphdr *iph;
	const struct tcphdr *th;
	int rc,len;

	if(skb->protocol == htons(ETH_P_ARP)) {
		return load_balancer_get_core_for_arp(skb);
	}
	if(skb->protocol != htons(ETH_P_IP)) {
		return load_balancer_poller_core_id;
	}
	/*skb->data points now to ip header */
	iph = (struct iphdr *)skb->data;
	len = ntohs(iph->tot_len);
	if (skb->len < len) {
		return load_balancer_poller_core_id;
	} else if (len < (iph->ihl*4))
		return load_balancer_poller_core_id;
	if((iph->protocol != IPPROTO_TCP)&&(iph->protocol != IPPROTO_UDP)) {
		return load_balancer_poller_core_id;/* not yet load balancing except TCP */
	}
        if(iph->protocol == IPPROTO_UDP) {
            if(load_balancer_total_cpus == load_balancer_next_core_for_udp) {
                load_balancer_next_core_for_udp = 0;
                return load_balancer_poller_core_id;
            }
            rc = idx_to_core_ids[load_balancer_next_core_for_udp++];
	    return rc;
        }
        th = (struct tcphdr *)(skb->data + iph->ihl*4);
	load_balancer_stats[load_balancer_cpu_mask & th->source]++;
	return idx_to_core_ids[load_balancer_cpu_mask & th->source];
}

int load_balancer_set_poller_core_id(uint8_t poller_core_id)
{
	load_balancer_poller_core_id = poller_core_id;
}

int load_balancer_get_poller_core_id()
{
	return load_balancer_poller_core_id;
}
/* number of cpus in mask must be power of 2 */
void load_balancer_init(uint8_t mask)
{
	int i;

	for(i = 0;i < MAXCPU;i++) {
		if(mask & (uint8_t)(1 << i)) {
			idx_to_core_ids[load_balancer_total_cpus] = i;
			load_balancer_total_cpus++;
		}
	}
	for(i = 0;i < (load_balancer_total_cpus - 1);i++) {
		load_balancer_cpu_mask |= 1 << i;
	}
	printf("%s %d %x %d %x %d %d %d\n",__FILE__,__LINE__,
			mask,load_balancer_total_cpus,load_balancer_cpu_mask,idx_to_core_ids[0],idx_to_core_ids[1],idx_to_core_ids[2]);
}

void load_balancer_print_stats()
{
	int i;
	for(i = 0;i < 4;i++) {
		printf("load_balancer_stats#%d %"PRIu64"\n",i,load_balancer_stats[i]);
	}
}
