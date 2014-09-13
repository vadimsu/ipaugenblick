/*
 * load_balancer2.h
 *
 *  Created on: Jul 6, 2014
 *      Author: Vadim Suraev vadim.suraev@gmail.com
 *  Contains function prototypes for load balancing between the cores
 */

#ifndef __LOAD_BALANCER2_H_
#define __LOAD_BALANCER2_H_

int load_balancer_get_core_to_process_packet(struct sk_buff *skb);

#endif /* __LOAD_BALANCER2_H_ */
