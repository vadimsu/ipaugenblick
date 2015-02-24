#ifndef __DUMMIES_H__
#define __DUMMIES_H__
#define EXPORT_SYMBOL(a)
#define EXPORT_SYMBOL_GPL(a)
#define ASSERT_RTNL()
#define KERN_DEBUG ""
#define KERN_ERR ""
#define KERN_WARNING ""
#define KERN_INFO ""
#define rcu_access_pointer(p) p
#define get_jiffies_64() jiffies
#ifdef __BIG_ENDIAN
#undef __BIG_ENDIAN
#endif
#define __LITTLE_ENDIAN 1234
#define __LITTLE_ENDIAN_BITFIELD
#define kmemcheck_bitfield_begin(a)
#define kmemcheck_bitfield_end(a)
#define __force
#define __rcu
#define __stringify
#define __aligned(a)
#define get_order(a) a
#define __must_be_array(arr) 0
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))
#define swap(a, b) \
	do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)
//#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
/*#define container_of(ptr, type, member) ({                      \
         const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
         (type *)( (char *)__mptr - offsetof(type,member) );})*/
#define __ALIGN_KERNEL(x, a)		__ALIGN_KERNEL_MASK(x, (typeof(x))(a) - 1)
#define __ALIGN_KERNEL_MASK(x, mask)	(((x) + (mask)) & ~(mask))
#define ALIGN(x, a)		__ALIGN_KERNEL((x), (a))
extern unsigned long long jiffies;
#define false 0
#define true 1
#define NUMA_NO_NODE 0
//#define GFP_ATOMIC 0
//#define  __GFP_WAIT 0
//#define GFP_KERNEL 0
//#define __GFP_COLD 0
//#define __GFP_NOMEMALLOC 0
//#define __GFP_MEMALLOC 0
//#define SLAB_HWCACHE_ALIGN 0
//#define SLAB_PANIC 0
#define MAX_ADDR_LEN	32
#define __percpu
#define __user
#define __must_check
#define __kprobes
#define min_t(type, x, y) ({			\
	type __min1 = (x);			\
	type __min2 = (y);			\
	__min1 < __min2 ? __min1: __min2; })

#define max_t(type, x, y) ({			\
	type __max1 = (x);			\
	type __max2 = (y);			\
	__max1 > __max2 ? __max1: __max2; })
#define CACHE_LINE_SIZE 64
#define SMP_CACHE_BYTES CACHE_LINE_SIZE
#define L1_CACHE_BYTES CACHE_LINE_SIZE
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define kmemcheck_annotate_bitfield(a,b)
//#define HZ 100
#define USHRT_MAX 0xFFFF
#define __init
#define __initdata
#define __setup(a,b)
#define __always_inline inline __attribute__((always_inline))
//#define __releases
//#define ARCH_KMALLOC_MINALIGN 64
#define ____cacheline_aligned_in_smp
#define ____cacheline_aligned
#define INT_MIN		(-INT_MAX - 1)
#define INT_MAX		((int)(~0U>>1))
//#define __cpu_to_be16(a) a
//#define cpu_to_be16 __cpu_to_be16
//#define list_for_each_entry_rcu list_for_each_entry
#define ns_capable(a,b) (1)
#define CAP_NET_RAW 1
#define __read_mostly
#define DEFINE_WAIT(a)
#define prepare_to_wait(a,b,c)
#define finish_wait(a,b)
#define signal_pending(a) 0
#define O_NONBLOCK	00004000
#define spin_lock_bh(a)
#define spin_unlock_bh(a)
//#define for_each_possible_cpu(i) RTE_LCORE_FOREACH(i)
#define FIELD_SIZEOF(a,b)
#define BUILD_BUG_ON(a)
//#define MODULE_ALIAS_NETPROTO(a)
#define __printf(a,b)
#define spin_trylock(a) (1)
#define spin_lock(a)
#define spin_lock_bh(a)
#define spin_unlock(a)
#define spin_unlock_bh(a)
#define spin_lock_nested(a,b)
#define spin_lock_init(a)
#define _atomic_spin_lock_irqsave(a,b)
#define _atomic_spin_unlock_irqrestore(a,b)
#define atomic_long_t long
#define ATOMIC64_INIT(i)	{ (i) }
#define ATOMIC_LONG_INIT(i)	ATOMIC64_INIT(i)
#define INITIAL_JIFFIES 0
#define LONG_MAX 0xFFFFFFFF
//#define PAGE_SIZE 1024*2000
#define PAGE_SHIFT 16
#define local_bh_disable()
#define local_bh_enable()
#define __kernel
//#define percpu_counter_read(a) 0
//#define __percpu_counter_add(a,b,c)
//#define percpu_counter_init(a,b)
//#define DECLARE_PER_CPU_ALIGNED(a,b) a b;
//#define DEFINE_PER_CPU(a,b) a b
//# define this_cpu_inc(pcp)		this_cpu_add((pcp), 1)
#define mutex_init(a)
#define mutex_lock(a)
#define mutex_unlock(a)
//#define percpu_counter_sum_positive(a) 0
#define IS_ENABLED defined
#define read_lock_bh(a)
#define read_unlock_bh(a)
#define write_lock_bh(a)
#define write_unlock_bh(a)
#define write_lock(a)
#define write_unlock(a)
#define write_seqlock_bh(a)
#define write_sequnlock_bh(a)
#define write_seqlock(a)
#define write_sequnlock(a)
#define rwlock_init(a)
#define seqlock_init(a)
#define down_write(a)
#define up_write(a)
#define down_read(a)
#define up_read(a)
#define synchronize_srcu(a)
#define srcu_read_lock(a) 0
#define srcu_read_unlock(a,b)
//#define register_netdevice_notifier(a)
#define NULL 0
#define xfrm4_policy_check(a,b,c) 1
/*#define IP_INC_STATS_BH(a,b)
#define NET_INC_STATS_BH(a,b)
#define NET_INC_STATS(a,b)
#define NET_INC_STATS_USER(a,b)
#define NET_ADD_STATS_USER(a,b,c)
#define IP_UPD_PO_STATS_BH(a,b,c)
#define IP_ADD_STATS_BH(a,b,c)
#define IP_UPD_PO_STATS(a,b,c)
#define IP_INC_STATS(a,b)
#define SNMP_INC_STATS(a,b)
#define SNMP_DEC_STATS(a,b)
#define SNMP_ADD_STATS_USER(a,b,c)
#define SNMP_ADD_STATS(a,b,c)*/
//#define get_cpu_var(a) a
//#define ATOMIC_INIT(a) a
//typedef unsigned char u8;
//typedef unsigned short u16;
//typedef unsigned int u32;
//#define MSEC_PER_SEC 1000
//#define NSEC_PER_MSEC 1000000
#define read_unlock(a)
#define read_lock(a)
#define rcu_read_lock()
#define rcu_read_unlock()
#define spin_lock_irqsave(a,b)
#define spin_unlock_irqrestore(a,b)
#define raw_local_irq_save(a)
#define raw_local_irq_restore(a)
#define rwlock_init(a)
#define local_irq_save(a)
#define local_irq_restore(a)
#define lockdep_set_class_and_name(a,b,c)
#define raw_spin_lock_irqsave(a,b)
#define raw_spin_unlock_irqrestore(a,b)
#define raw_spin_lock_init(a)
//#define STATIC_KEY_INIT_FALSE 0
#define FIONREAD	0x541B
#define TIOCINQ		FIONREAD
#define TIOCOUTQ	0x5411
#define ACCESS_ONCE(x) (*(volatile typeof(x) *)&(x))
#define UINT_MAX 0xFFFFFFFF
#define NR_CPUS 32
#define __deprecated
#define rcu_dereference_protected(a,b) a
#define rcu_dereference_check(a,b) a
#define rcu_dereference_raw(a) a
#define ockdep_rtnl_is_held()
#define rcu_dereference_index_check(a,b) a
#define  __acquires(a)
#define  __releases(a)
#define mutex_acquire(a,b,c,d)
#define mutex_release(a,b,c)
#define mutex_lock_interruptible(a) 0
#define prepare_to_wait_exclusive(a,b,c)
#define schedule()
//#define write_pnet(a,b)
#define read_seqbegin(a) 0
#define read_seqretry(a,b) 0
#define remove_proc_entry(a,b)
//#define call_netevent_notifiers(a,b)
//#define alloc_percpu(a) malloc(a)
/*static inline int copy_from_user(void *dst,void *src,int size)
{
    memcpy(dst,src,size); 
    return 0;
}
static inline int copy_to_user(void *dst,void *src,int size)
{
    memcpy(dst,src,size);
    return 0;
}*/
//#define PTR_ERR(a) -3
//#define IS_ERR(a) (a == 0)
#define BUG() { printf("%s %d\n",__FILE__,__LINE__); abort(); }
//#define WARN_ON(a) (a)
#define WARN_ON(condition) ({                                           \
         int __ret_warn_on = !!(condition);                              \
         if (unlikely(__ret_warn_on))                                    \
                 __WARN();                                               \
         unlikely(__ret_warn_on);                                        \
})
#define rcu_assign_pointer(a,b) a = b
#define rcu_dereference(a) a
#define RCU_INIT_POINTER(a,b) a = b
#define rcu_dereference_rtnl(a) a
#define rcu_dereference_bh(a) a
#define rcu_read_lock_bh()
#define rcu_read_unlock_bh()
#define rcu_read_lock_held() 1
#define rcu_read_lock_bh_held() 1
#define netpoll_rx_enable(a)
#define netpoll_rx_disable(a)
#define net_dmaengine_put()
#define dev_deactivate_many(a)
#define might_sleep()
#define add_device_randomness(a,b)
#define dev_activate(a)
#define net_dmaengine_get()
//#define notifier_to_errno(a) a
//#define smp_mb__after_clear_bit()
#define hotcpu_notifier(a,b) a(NULL,b,NULL)
#define netdev_kobject_init() 0
#define netdev_unregister_kobject(a)
#define netdev_unregister_kobject(a)
#define dev_shutdown(a)
#define netpoll_poll_lock(a) NULL
#define netpoll_poll_unlock(a)
#define trace_napi_poll(a)
#define trace_napi_gro_frags_entry(a)
#define trace_netif_receive_skb_entry(skb)
#define trace_netif_receive_skb(skb)
#define trace_netif_rx_ni_entry(skb)
#define trace_netif_rx_entry(skb)
#define trace_netif_rx(skb)
#define trace_net_dev_queue(skb)
#define trace_net_dev_xmit(skb, rc, dev, skb_len)
#define trace_net_dev_start_xmit(skb, dev)
#define trace_napi_gro_receive_entry(skb)
#define trace_sock_rcvqueue_full(sk, skb)
#define trace_udp_fail_queue_rcv_skb(a,b)
#define trace_kfree_skb(skb, udp_recvmsg)
#define trace_skb_copy_datagram_iovec(skb, len)
#define __netdev_watchdog_up(dev)
#define smp_rmb()
#define net_rx_queue_update_kobjects(dev, real_num_rx_queues, rxq)
#define netdev_queue_update_kobjects(dev, real_num_tx_queues,txq) 0
#define in_irq() 0
#define irqs_disabled() 0
//#define __this_cpu_inc(a)
//#define __this_cpu_dec(a)
#define __this_cpu_read(a) 0
#define put_cpu()
#define preempt_enable()
#define preempt_disable()
#define do_softirq()
#define local_softirq_pending() 0
#define netpoll_rx_on(skb) 1
#define netpoll_rx(skb) 0
#define __rtnl_unlock(a)
#define __rtnl_lock(a)
#define synchronize_rcu()
#define synchronize_rcu_expedited()
#define on_each_cpu(a,b,c) a(b)
#define dump_stack()
#define rcu_barrier()
#define rtmsg_ifinfo(a,b,c,d)
#define dev_init_scheduler(a)
#define linkwatch_run_queue(a)
#define is_vmalloc_addr(a) 0
//#define call_netdevice_notifiers_info(NETDEV_CHANGE, dev,info)
#define time_after(a,b)		\
	 ((long)((b) - (a)) < 0)
#define time_before(a,b)	time_after(b,a)
#define time_after_eq(a,b)	\
	(((long)((a) - (b)) >= 0))
#define time_before_eq(a,b)	time_after_eq(b,a)
#define time_is_after_jiffies(a) time_before(jiffies, a)
#define pr_debug printf
#define pr_err printf
#define pr_emerg printf
#define pr_info printf
#define pr_warn printf
#define pr_crit printf
#define pr_notice printf
#define strlcpy strncpy
//#define net_warn_ratelimited printf
//#define net_info_ratelimited printf
//#define net_ratelimit() 1
//#define ERR_PTR(a) NULL
#define min(x, y) ({				\
	typeof(x) _min1 = (x);			\
	typeof(y) _min2 = (y);			\
	(void) (&_min1 == &_min2);		\
	_min1 < _min2 ? _min1 : _min2; })

#define max(x, y) ({				\
	typeof(x) _max1 = (x);			\
	typeof(y) _max2 = (y);			\
	(void) (&_max1 == &_max2);		\
	_max1 > _max2 ? _max1 : _max2; })
/*static inline int notifier_from_errno(int err)
{
    return 0;
}*/
#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)
#define BUG_ON(condition) do { if (unlikely(condition)) BUG(); } while(0)
#define __WARN()		printf("WARN %s %d\n",__FILE__, __LINE__)
#define WARN(condition, format...) ({                                           \
         int __ret_warn_on = !!(condition);                              \
         if (unlikely(__ret_warn_on))                                    \
                __WARN();                                  \
         unlikely(__ret_warn_on);                                        \
 })
#define WARN_ON(condition) ({						\
	int __ret_warn_on = !!(condition);				\
	if (unlikely(__ret_warn_on))					\
		__WARN();						\
	unlikely(__ret_warn_on);					\
})
#define WARN_ON_ONCE(condition) WARN_ON(condition)
//#define GFP_ATOMIC 0
//#define __GFP_COLD 0
//#define GFP_KERNEL 0
//#define __GFP_REPEAT 0
//#define __GFP_COMP 0
//#define __GFP_NOWARN 0
//#define __GFP_NORETRY 0
//#define __GFP_ZERO 0
//#define GFP_USER 0
#define page_address(a) a->pkt.data
//#define gfp_t int
//#define ___htons(x) x
//#define htons(x) ___htons(x)
#define local_irq_disable()
#define local_irq_enable()
#define net_eq(a,b) 1
//#define atomic_inc_not_zero(v)		/*atomic_add_unless((v), 1, 0)*/1
//#define USEC_PER_SEC 1000
#define MAX_SCHEDULE_TIMEOUT 0xFFFFFFFF
#define schedule_timeout(a) a
#define NR_FILE 1024
#define CONFIG_DEFAULT_TCP_CONG "reno"
//#define csum_partial_copy_nocheck(src, dst, len, sum)	\
//	csum_partial_copy((src), (dst), (len), (sum))
#define __visible
#define __exit
unsigned long msecs_to_jiffies(const unsigned int m);
unsigned int jiffies_to_msecs(const unsigned long j);
#define sock_update_classid(sk)
#define sock_update_netprioidx(sk)
#define sock_intr_errno(timeo) 0
#define __packed __attribute__ ((__packed__))
#define cond_resched()
#define prefetch(a) rte_prefetch0(a)
#define panic(a) { printf("PANIC %s\n",a); while(1); }
#define __attribute_const__
#define noinline_for_stack
#define need_resched() 0
#define init_completion(a)
#define complete_all(a)
#define noinline
#define inet_ehash_locks_alloc(a) 0
#define kmap(page) page->mbuf->pkt.data
#define kunmap
#define get_page(a)
#define put_page(a)
#define get_user(fl4_icmp_type, type) 0
#define put_user(a, optlen) 0
#define __get_page(a)
#define __put_page(a)
#define __get_user(fl4_icmp_type, type) 0
#define __put_user(a, optlen) 0
#define barrier()
#define put_net(a) a
#define get_net(a) a
#define kunmap_atomic(addr)
# define HZ		CONFIG_HZ	/* Internal kernel timer frequency */
# define USER_HZ	100		/* some user interfaces are */
#define NSEC_PER_SEC	1000000000L
#define TICK_NSEC ((NSEC_PER_SEC+HZ/2)/HZ)
int strncpy_from_user ( char * destination, const char * source, int num );
#define virt_to_head_page(a) a
#define IP_MAX_MSF		10
#if 1
#define RELOC_HIDE(ptr, off)					\
  ({ unsigned long __ptr;					\
    __asm__ ("" : "=r"(__ptr) : "0"(ptr));		\
    (typeof(ptr)) (__ptr + (off)); })
#else
#define RELOC_HIDE(ptr, off) (ptr+off)
#endif
#define __cpu_to_le64(x) ((__force __le64)(__u64)(x))
#define cpu_to_le64 __cpu_to_le64
#define __ALIGN_KERNEL_MASK(x, mask)	(((x) + (mask)) & ~(mask))
#define __ALIGN_MASK(x, mask)	__ALIGN_KERNEL_MASK((x), (mask))
#define __ALIGN_KERNEL(x, a)		__ALIGN_KERNEL_MASK(x, (typeof(x))(a) - 1)
#define ALIGN(x, a)		__ALIGN_KERNEL((x), (a))
//#define PTR_ALIGN(p, a)		((typeof(p))ALIGN((unsigned long)(p), (a)))
#define PTR_ALIGN(p, a)	RTE_PTR_ALIGN(p,a)
//static inline unsigned long __arch_hweight64(__u64 w)
//{
//	return __sw_hweight64(w);
//}
#define hweight64(w) (__builtin_constant_p(w) ? __const_hweight64(w) : __arch_hweight64(w))
#define prefetchw(a)
#define kmemcheck_annotate_variable(a)
#define security_sk_classify_flow(sk, fl)

#define security_req_classify_flow(req, fl)

#define security_skb_owned_by(skb, sk)

#define security_sock_graft(sk, parent)

#define yield()

#define smp_mb()

#define smp_wmb()
#endif
