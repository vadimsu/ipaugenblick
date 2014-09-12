/*
 * tasklet_workqueues_porting.c
 *
 *  Created on: Jul 6, 2014
 *      Author: Vadim Suraev vadim.suraev@gmail.com
 *  Contains functions to port Linux kernel tasklet & workqueues (delayed as well)
 *  for the userland TCP/IP with integration with DPDK 1.6
 */
#include <specific_includes/dummies.h>
#include <specific_includes/linux/types.h>
#include <specific_includes/linux/bitops.h>
#include <specific_includes/linux/slab.h>
#include <specific_includes/linux/timer.h>
#include <specific_includes/linux/tasklet.h>
#include <rte_config.h>
#include <rte_common.h>
#include <rte_memcpy.h>
#include <rte_lcore.h>
#include <rte_mempool.h>
#include <rte_cycles.h>
#include <rte_hash.h>
#include <rte_byteorder.h>

struct workqueue_struct *system_power_efficient_wq;

void tasklet_init(struct tasklet_struct *t,
                   void (*func)(unsigned long), unsigned long data)
{
         t->next = NULL;
         t->state = 0;
         atomic_set(&t->count, 0);
         t->func = func;
         t->data = data;
}
EXPORT_SYMBOL(tasklet_init);
void tasklet_schedule(struct tasklet_struct *t)
{
        t->func(t->data);
}
struct workqueue_struct *system_wq = NULL;

bool queue_work_on(int cpu, struct workqueue_struct *wq,
			struct work_struct *work)
{
    delayed_work_timer_fn((unsigned long)work);
    return true;
}
void delayed_work_expiry_cbk(struct rte_timer *tim, void *arg)
{
	struct work_struct *work = (struct work_struct *)arg;
	delayed_work_timer_fn((unsigned long)work);
}
//bool queue_work_on(int cpu, struct workqueue_struct *wq,
//			struct work_struct *work);
bool queue_delayed_work_on(int cpu, struct workqueue_struct *wq,
			struct delayed_work *work, unsigned long delay)
{
    work->timer.expires = jiffies + delay;
    return (rte_timer_reset(&work->timer.tim,rte_get_timer_hz()*work->timer.expires,SINGLE,rte_lcore_id(),delayed_work_expiry_cbk,work) == 0);
}
bool mod_delayed_work_on(int cpu, struct workqueue_struct *wq,
			struct delayed_work *dwork, unsigned long delay)
{
    dwork->timer.expires = jiffies + delay;
    return (rte_timer_reset(&dwork->timer.tim,rte_get_timer_hz()*dwork->timer.expires,SINGLE,rte_lcore_id(),delayed_work_expiry_cbk,dwork) == 0);
}

bool cancel_delayed_work(struct delayed_work *dwork)
{
    rte_timer_stop(&dwork->timer.tim);
    return true;
}

void delayed_work_timer_fn(unsigned long __data)
{
	struct work_struct *work = (struct work_struct *)__data;
	work->func(work);
}

bool cancel_delayed_work_sync(struct delayed_work *dwork)
{
    rte_timer_stop_sync(&dwork->timer.tim);
    return true;
}
