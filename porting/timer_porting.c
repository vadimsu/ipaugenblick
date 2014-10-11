/*
 * timer_porting.c
 *
 *  Created on: Jul 6, 2014
 *      Author: Vadim Suraev vadim.suraev@gmail.com
 *  Contains functions for porting Linux kernel timer subsystem
 *  for the Linux TCP/IP ported to userland and integrated with DPDK 1.6
 */
#include <specific_includes/dummies.h>
#include <specific_includes/linux/types.h>
#include <specific_includes/linux/bitops.h>
#include <specific_includes/linux/slab.h>
#include <specific_includes/linux/timer.h>
#include <rte_config.h>
#include <rte_common.h>
#include <rte_memcpy.h>
#include <rte_lcore.h>
#include <rte_mempool.h>
#include <rte_cycles.h>
#include <rte_hash.h>
#include <rte_byteorder.h>

int del_timer(struct timer_list * timer)
{
    if(rte_timer_pending(&timer->tim)) {
        return !rte_timer_stop(&timer->tim);
    } 
    return 0;
}
int del_timer_sync(struct timer_list *timer)
{
    if(rte_timer_pending(&timer->tim)) {
        rte_timer_stop_sync(&timer->tim);
        return 1;
    }
    return 0;
}
void timer_expiry_cbk(struct rte_timer *tim, void *arg)
{
	struct timer_list *timer = (struct timer_list *)arg;
	timer->function(timer->data);
}
int mod_timer(struct timer_list *timer, unsigned long expires)
{
//printf("%d %d\n",rte_get_timer_hz(),expires-jiffies);
    int pending = rte_timer_pending(&timer->tim);
    return rte_timer_reset(&timer->tim,rte_get_timer_hz()*(expires-jiffies)/HZ,SINGLE,rte_lcore_id(),timer_expiry_cbk,timer) || pending;
}

void add_timer(struct timer_list *timer)
{
	mod_timer(timer, timer->expires);
}

void init_timer_key(struct timer_list *timer, unsigned int flags,
		    const char *name)
{
	rte_timer_init(&timer->tim);
}
