/*
 * load_balancer.h
 *
 *  Created on: Jul 6, 2014
 *      Author: Vadim Suraev vadim.suraev@gmail.com
 *  Contains function prototypes for load balancing between the cores
 */

#ifndef __LOAD_BALANCER_H_
#define __LOAD_BALANCER_H_


int load_balancer_set_poller_core_id(uint8_t poller_core_id);

int load_balancer_get_poller_core_id();

void load_balancer_init(uint8_t mask);

#endif /* __LOAD_BALANCER_H_ */
