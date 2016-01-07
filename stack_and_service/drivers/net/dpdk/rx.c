/*
 * rx_thread.c
 *
 *  Created on: Jul 6, 2014
 *      Author: Vadim Suraev vadim.suraev@gmail.com
 *  Contains functions for glueing ported Linux kernel's TCP/IP to the DPDK 1.6
 */
#include <specific_includes/dummies.h>
#include <specific_includes/linux/types.h>
#include <string.h>
#include <rte_config.h>
#include <rte_common.h>
#include <rte_cycles.h>
#include <rte_ethdev.h>

uint64_t received = 0;
uint8_t get_queue_idx(void);

int dpdk_dev_get_received(int port_num,struct rte_mbuf **tbl,int tbl_size)
{
	unsigned nb_rx = 0;

	nb_rx = rte_eth_rx_burst((uint8_t)port_num, get_queue_idx(), tbl,tbl_size);
	received += nb_rx;
	return nb_rx;
}
