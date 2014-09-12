/*
 * timing_porting.c
 *
 *  Created on: Jul 6, 2014
 *      Author: Vadim Suraev vadim.suraev@gmail.com
 *  Contains functions Linux kernel time
 *  for the Linux TCP/IP ported to userland and integrated with DPDK 1.6
 */
#include <specific_includes/dummies.h>
#include <specific_includes/linux/types.h>
#include <specific_includes/linux/bitops.h>
#include <specific_includes/linux/slab.h>
#include <specific_includes/linux/ktime.h>
#include <rte_config.h>
#include <rte_common.h>
#include <rte_memcpy.h>
#include <rte_lcore.h>
#include <rte_mempool.h>
#include <rte_cycles.h>
#include <rte_hash.h>
#include <rte_byteorder.h>

static struct rte_timer systick;
unsigned long long jiffies = 0;

static void systick_expiry_cbk(struct rte_timer *tim,void *arg)
{
	jiffies++;
//	if(!(jiffies%HZ))
//		printf("JIFFIES %d HZ %d %d %p\n",jiffies,HZ,jiffies/HZ,tim);
	rte_timer_reset(&systick,rte_get_timer_hz()/1000,SINGLE,rte_lcore_id(),systick_expiry_cbk,NULL);
}

void init_systick(int core_id)
{
	rte_timer_init(&systick);
	rte_timer_reset(&systick,rte_get_timer_hz()/1000,SINGLE,core_id,systick_expiry_cbk,NULL);
}

unsigned long round_jiffies_up(unsigned long j)
{
    return j;
}

unsigned long get_seconds(void)
{
    return 0;
}
unsigned int jiffies_to_usecs(const unsigned long j)
{
    return j/1000;
}

unsigned long msecs_to_jiffies(const unsigned int m)
{
    return m;
}

unsigned long usecs_to_jiffies(const unsigned int u)
{
    return u*1000;
}
unsigned int jiffies_to_msecs(const unsigned long j)
{
    return j;
}

ktime_t ktime_get_real(void)
{
    ktime_t now;
    now.tv64 = 0;
    return now;
}

/**
 * ns_to_timeval - Convert nanoseconds to timeval
 * @nsec:       the nanoseconds value to be converted
 *
 * Returns the timeval representation of the nsec parameter.
 */
struct timeval ns_to_timeval(const s64 nsec)
{
	struct timespec ts = ns_to_timespec(nsec);
	struct timeval tv;

	tv.tv_sec = ts.tv_sec;
	tv.tv_usec = (suseconds_t) ts.tv_nsec / 1000;

	return tv;
}
EXPORT_SYMBOL(ns_to_timeval);

/**
 * ns_to_timespec - Convert nanoseconds to timespec
 * @nsec:       the nanoseconds value to be converted
 *
 * Returns the timespec representation of the nsec parameter.
 */
struct timespec ns_to_timespec(const s64 nsec)
{
	struct timespec ts;
	s32 rem;

	if (!nsec)
		return (struct timespec) {0, 0};

	ts.tv_sec = div_s64_rem(nsec, NSEC_PER_SEC, &rem);
	if (unlikely(rem < 0)) {
		ts.tv_sec--;
		rem += NSEC_PER_SEC;
	}
	ts.tv_nsec = rem;

	return ts;
}

#define CONFIG_BASE_SMALL 0
#define TVN_BITS (CONFIG_BASE_SMALL ? 4 : 6)
#define TVR_BITS (CONFIG_BASE_SMALL ? 6 : 8)
#define TVN_SIZE (1 << TVN_BITS)
#define TVR_SIZE (1 << TVR_BITS)
struct tvec {
	struct list_head vec[TVN_SIZE];
};
struct tvec_root {
	struct list_head vec[TVR_SIZE];
};
struct tvec_base {
//	spinlock_t lock;
	struct timer_list *running_timer;
	unsigned long timer_jiffies;
	unsigned long next_timer;
	unsigned long active_timers;
	struct tvec_root tv1;
	struct tvec tv2;
	struct tvec tv3;
	struct tvec tv4;
	struct tvec tv5;
} ____cacheline_aligned;
struct tvec_base boot_tvec_bases;
EXPORT_SYMBOL(boot_tvec_bases);

/*
 * Convert jiffies/jiffies_64 to clock_t and back.
 */
clock_t jiffies_to_clock_t(unsigned long x)
{
#if (TICK_NSEC % (NSEC_PER_SEC / USER_HZ)) == 0
# if HZ < USER_HZ
	return x * (USER_HZ / HZ);
# else
	return x / (HZ / USER_HZ);
# endif
#else
	return div_u64((u64)x * TICK_NSEC, NSEC_PER_SEC / USER_HZ);
#endif
}

static inline u32 get_arch_timeoffset(void) { return 0; }

EXPORT_SYMBOL(jiffies_to_clock_t);
#if 0
static inline s64 timekeeping_get_ns(struct timekeeper *tk)
{
	cycle_t cycle_now, cycle_delta;
	struct clocksource *clock;
	s64 nsec;

	/* read clocksource: */
	clock = tk->clock;
	cycle_now = clock->read(clock);

	/* calculate the delta since the last update_wall_time: */
	cycle_delta = (cycle_now - clock->cycle_last) & clock->mask;

	nsec = cycle_delta * tk->mult + tk->xtime_nsec;
	nsec >>= tk->shift;

	/* If arch requires, add in get_arch_timeoffset() */
	return nsec + get_arch_timeoffset();
}
#endif
/**
 * __getnstimeofday - Returns the time of day in a timespec.
 * @ts:		pointer to the timespec to be set
 *
 * Updates the time of day in the timespec.
 * Returns 0 on success, or -ve when suspended (timespec will be undefined).
 */
int __getnstimeofday(struct timespec *ts)
{
#if 0
	struct timekeeper *tk = &timekeeper;
//	unsigned long seq;
	s64 nsecs = 0;

//	do {
		seq = read_seqcount_begin(&timekeeper_seq);

		ts->tv_sec = tk->xtime_sec;
		nsecs = timekeeping_get_ns(tk);

//	} while (read_seqcount_retry(&timekeeper_seq, seq));

	ts->tv_nsec = 0;
	timespec_add_ns(ts, nsecs);

	/*
	 * Do not bail out early, in case there were callers still using
	 * the value, even in the face of the WARN_ON.
	 */
	if (unlikely(timekeeping_suspended))
		return -EAGAIN;
#endif
	return 0;
}
EXPORT_SYMBOL(__getnstimeofday);

/**
 * getnstimeofday - Returns the time of day in a timespec.
 * @ts:		pointer to the timespec to be set
 *
 * Returns the time of day in a timespec (WARN if suspended).
 */
void getnstimeofday(struct timespec *ts)
{
	__getnstimeofday(ts);
}
EXPORT_SYMBOL(getnstimeofday);
static unsigned long round_jiffies_common(unsigned long j, int cpu,
		bool force_up)
{
	int rem;
	unsigned long original = j;

	/*
	 * We don't want all cpus firing their timers at once hitting the
	 * same lock or cachelines, so we skew each extra cpu with an extra
	 * 3 jiffies. This 3 jiffies came originally from the mm/ code which
	 * already did this.
	 * The skew is done by adding 3*cpunr, then round, then subtract this
	 * extra offset again.
	 */
	j += cpu * 3;

	rem = j % HZ;

	/*
	 * If the target jiffie is just after a whole second (which can happen
	 * due to delays of the timer irq, long irq off times etc etc) then
	 * we should round down to the whole second, not up. Use 1/4th second
	 * as cutoff for this rounding as an extreme upper bound for this.
	 * But never round down if @force_up is set.
	 */
	if (rem < HZ/4 && !force_up) /* round down */
		j = j - rem;
	else /* round up */
		j = j - rem + HZ;

	/* now that we have rounded, subtract the extra skew again */
	j -= cpu * 3;

	/*
	 * Make sure j is still in the future. Otherwise return the
	 * unmodified value.
	 */
	return time_is_after_jiffies(j) ? j : original;
}
/**
 * __round_jiffies_relative - function to round jiffies to a full second
 * @j: the time in (relative) jiffies that should be rounded
 * @cpu: the processor number on which the timeout will happen
 *
 * __round_jiffies_relative() rounds a time delta  in the future (in jiffies)
 * up or down to (approximately) full seconds. This is useful for timers
 * for which the exact time they fire does not matter too much, as long as
 * they fire approximately every X seconds.
 *
 * By rounding these timers to whole seconds, all such timers will fire
 * at the same time, rather than at various times spread out. The goal
 * of this is to have the CPU wake up less, which saves power.
 *
 * The exact rounding is skewed for each processor to avoid all
 * processors firing at the exact same time, which could lead
 * to lock contention or spurious cache line bouncing.
 *
 * The return value is the rounded version of the @j parameter.
 */
unsigned long __round_jiffies_relative(unsigned long j, int cpu)
{
	unsigned long j0 = jiffies;

	/* Use j0 because jiffies might change while we run */
	return round_jiffies_common(j + j0, cpu, false) - j0;
}
EXPORT_SYMBOL_GPL(__round_jiffies_relative);
/**
 * round_jiffies_relative - function to round jiffies to a full second
 * @j: the time in (relative) jiffies that should be rounded
 *
 * round_jiffies_relative() rounds a time delta  in the future (in jiffies)
 * up or down to (approximately) full seconds. This is useful for timers
 * for which the exact time they fire does not matter too much, as long as
 * they fire approximately every X seconds.
 *
 * By rounding these timers to whole seconds, all such timers will fire
 * at the same time, rather than at various times spread out. The goal
 * of this is to have the CPU wake up less, which saves power.
 *
 * The return value is the rounded version of the @j parameter.
 */
unsigned long round_jiffies_relative(unsigned long j)
{
	return __round_jiffies_relative(j, raw_smp_processor_id());
}
EXPORT_SYMBOL_GPL(round_jiffies_relative);
