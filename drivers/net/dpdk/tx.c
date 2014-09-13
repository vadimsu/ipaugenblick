/*
 * tx_thread.c
 *
 *  Created on: Jul 6, 2014
 *      Author: Vadim Suraev vadim.suraev@gmail.com
 *  Contains functions for glueing ported Linux kernel's TCP/IP to the DPDK 1.6
 */
#include <string.h>
#include <rte_config.h>
#include <rte_common.h>
#include <rte_cycles.h>
#include <rte_ethdev.h>

uint64_t transmitted = 0;
uint64_t tx_dropped[MAXCPU];
uint64_t poller_tx_dropped = 0;
struct rte_ring *ip_stack_to_dev_ring[RTE_MAX_ETHPORTS];

void dpdk_dev_enqueue_for_tx(int port_num,struct rte_mbuf *m)
{
	unsigned ret;

	ret = rte_ring_enqueue_burst(ip_stack_to_dev_ring[port_num],(void **)&m,1);

	if(ret != 1) {
		rte_pktmbuf_free(m);
		tx_dropped[rte_lcore_id()]++;
	}
}

void dpdk_dev_tx_poll_init(int port_num)
{
	char ring_name[1024];
	int cpu_idx;
	sprintf(ring_name,"ip_stack_to_dev_ring%d",port_num);
	ip_stack_to_dev_ring[port_num] = rte_ring_create(ring_name,4096,rte_socket_id(),0);
	if(!ip_stack_to_dev_ring[port_num]) {
		printf("Cannot initiate ring %s %d\n",__FILE__,__LINE__);
		abort();
	}
	for(cpu_idx = 0;cpu_idx < MAXCPU;cpu_idx++) {
		tx_dropped[cpu_idx] = 0;
	}
}

void dpdk_dev_tx_poll(int port_num)
{
	unsigned ret,i;
	struct rte_mbuf *tbl[MAX_PKT_BURST];

	ret = rte_ring_dequeue_burst(ip_stack_to_dev_ring[port_num],(void **)tbl,MAX_PKT_BURST);

	if(ret) {
		i = rte_eth_tx_burst(port_num, (uint16_t) /*queue_id*/0, tbl, (uint16_t) ret);

		transmitted += i;
		if (unlikely(i < ret)) {
			poller_tx_dropped += ret - i;
			for(;i < ret;i++)
			    rte_pktmbuf_free(tbl[i]);
		}
	}
}
