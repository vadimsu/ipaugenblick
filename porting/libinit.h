/*
 * libinit.h
 *
 *  Created on: Jul 6, 2014
 *      Author: Vadim Suraev vadim.suraev@gmail.com
 *  Contains function prototypes for initialization of
 *  Linux TCP/IP ported to userland and integrated with DPDK 1.6
 */

#ifndef __LIBINIT_H_
#define __LIBINIT_H_
/* This function returns a pointer to kernel's interface structure, required to access the driver
 * Parameter:ethernet port number
 */
void *get_dpdk_dev_by_port_num(int port_num);
/* This function allocates rte_mbuf */
void *get_buffer();
/* this function gets a pointer to data in the newly allocated rte_mbuf */
void *get_data_ptr(void *buf);
/* this function releases the rte_mbuf */
void release_buffer(void *buf);
/* this function returns an available mbufs count */
int get_buffer_count();
/*
 * This function may be called to calculate driver's optimal polling interval .
 * Paramters: a pointer to socket structure
 * Returns: None
 *
 */
int get_max_drv_poll_interval_in_micros(int port_num);

#endif /* __LIBINIT_H_ */
