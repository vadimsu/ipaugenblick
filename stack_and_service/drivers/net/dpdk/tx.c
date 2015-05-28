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
uint64_t tx_dropped = 0;

void dpdk_dev_enqueue_for_tx(int port_num,struct rte_mbuf *m)
{
	unsigned ret = rte_eth_tx_burst(port_num, (uint16_t) /*queue_id*/0, &m, (uint16_t) 1);
	transmitted += ret;
	if (unlikely(ret < 1)) {
		tx_dropped ++;
		rte_pktmbuf_free(m);
	}
}
