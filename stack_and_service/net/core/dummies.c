
#include <specific_includes/dummies.h>
//#include <asm/uaccess.h>
#include <specific_includes/linux/types.h>
#include <specific_includes/linux/bitops.h>
#include <specific_includes/linux/slab.h>
//#include <linux/capability.h>
//#include <linux/cpu.h>
//#include <linux/kernel.h>
#include <specific_includes/linux/hash.h>
//#include <linux/slab.h>
//#include <linux/sched.h>
//#include <linux/mutex.h>
//#include <linux/string.h>
//#include <linux/mm.h>
#include <specific_includes/linux/socket.h>
#include <specific_includes/linux/sockios.h>
//#include <linux/errno.h>
//#include <linux/interrupt.h>
#include <specific_includes/linux/if_ether.h>
#include <specific_includes/linux/netdevice.h>
#include <specific_includes/linux/etherdevice.h>
#include <specific_includes/linux/ethtool.h>
//#include <linux/notifier.h>
#include <specific_includes/linux/skbuff.h>
#include <specific_includes/net/net_namespace.h>
#include <specific_includes/net/sock.h>
#include <specific_includes/linux/rtnetlink.h>
//#include <linux/stat.h>
#include <specific_includes/net/dst.h>
//#include <net/pkt_sched.h>
#include <specific_includes/net/checksum.h>
#include <specific_includes/linux/cpumask.h>
#include <specific_includes/linux/log2.h>
#include <rte_malloc.h>
//#include <net/xfrm.h>
//#include <linux/highmem.h>
//#include <linux/init.h>
//#include <linux/module.h>
//#include <linux/netpoll.h>
//#include <linux/rcupdate.h>
//#include <linux/delay.h>
//#include <net/iw_handler.h>
//#include <asm/current.h>
//#include <linux/audit.h>
//#include <linux/dmaengine.h>
#include <specific_includes/linux/err.h>
//#include <linux/ctype.h>
#include <specific_includes/linux/if_arp.h>
#include <specific_includes/linux/if_vlan.h>
#include <specific_includes/linux/ip.h>
#include <specific_includes/net/ip.h>
#include <specific_includes/linux/ipv6.h>
#include <specific_includes/linux/in.h>
//#include <specific_includes/linux/jhash.h>
#include <specific_includes/linux/random.h>
//#include <trace/events/napi.h>
//#include <trace/events/net.h>
//#include <trace/events/skb.h>
//#include <linux/pci.h>
#include <specific_includes/linux/inetdevice.h>
//#include <linux/cpu_rmap.h>
//#include <linux/static_key.h>
#include <specific_includes/linux/hashtable.h>
//#include <linux/vmalloc.h>
#include <specific_includes/linux/if_macvlan.h>
#include <specific_includes/net/rtnetlink.h>
#include <specific_includes/linux/cpumask.h>
#include <specific_includes/linux/tasklet.h>
#include <specific_includes/asm-generic/atomic.h>
#include <rte_config.h>
#include <rte_common.h>
//#include <rte_memcpy.h>
#include <rte_lcore.h>
#include <rte_mempool.h>
#include <rte_cycles.h>
#include <rte_hash.h>
#include <rte_byteorder.h>

void ip_mc_down(struct in_device *in_dev)
{
}

void ip_mc_unmap(struct in_device *in_dev)
{
}

void ip_mc_remap(struct in_device *in_dev)
{
}

void ip_mc_up(struct in_device *in_dev)
{
}
void ip_mc_destroy_dev(struct in_device *d)
{
}
void ip_mc_init_dev(struct in_device *d)
{
}
int netlink_rcv_skb(struct sk_buff *skb,
		    int (*cb)(struct sk_buff *, struct nlmsghdr *))
{
    return 0;
}
int netlink_unicast(struct sock *ssk, struct sk_buff *skb, __u32 portid, int nonblock)
{
    return 0;
}
void netlink_kernel_release(struct sock *sk)
{
}
int nlmsg_notify(struct sock *sk, struct sk_buff *skb, u32 portid,
		 unsigned int group, int report, gfp_t flags)
{
    return 0;
}

int nla_validate(const struct nlattr *head, int len, int maxtype,
		 const struct nla_policy *policy)
{
    return 0;
}
int nla_parse(struct nlattr **tb, int maxtype, const struct nlattr *head,
	      int len, const struct nla_policy *policy)
{
    return 0;
}
int nla_policy_len(const struct nla_policy *p, int a)
{
    return 0;
}
struct nlattr *nla_find(const struct nlattr *head, int len, int attrtype)
{
    return NULL;
}
size_t nla_strlcpy(char *dst, const struct nlattr *nla, size_t dstsize)
{
    return 0;
}
int nla_memcpy(void *dest, const struct nlattr *src, int count)
{
    return 0;
}
int nla_memcmp(const struct nlattr *nla, const void *data, size_t size)
{
    return 0;
}
int nla_strcmp(const struct nlattr *nla, const char *str)
{
    return 0;
}
struct nlattr *__nla_reserve(struct sk_buff *skb, int attrtype, int attrlen)
{
    return 0;
}
void *__nla_reserve_nohdr(struct sk_buff *skb, int attrlen)
{
    return NULL;
}
struct nlattr *nla_reserve(struct sk_buff *skb, int attrtype, int attrlen)
{
    return NULL;
}
void *nla_reserve_nohdr(struct sk_buff *skb, int attrlen)
{
    return NULL;
}
void __nla_put(struct sk_buff *skb, int attrtype, int attrlen,
	       const void *data)
{
}
void __nla_put_nohdr(struct sk_buff *skb, int attrlen, const void *data)
{
}
int nla_put(struct sk_buff *skb, int attrtype, int attrlen, const void *data)
{
    return 0;
}
int nla_put_nohdr(struct sk_buff *skb, int attrlen, const void *data)
{
    return 0;
}
int nla_append(struct sk_buff *skb, int attrlen, const void *data)
{
    return 0;
}

void rtnl_set_sk_err(struct net *net, u32 group, int error)
{
}
int rtnetlink_put_metrics(struct sk_buff *skb, u32 *metrics)
{
	return 0;
}
int rtnl_put_cacheinfo(struct sk_buff *skb, struct dst_entry *dst,
			      u32 id, long expires, u32 error)
{
	return 0;
}
int rtnl_unicast(struct sk_buff *skb, struct net *net, u32 pid)
{
	return 0;
}
int netdev_register_kobject(struct net_device *net)
{
	return 0;
}

void get_random_bytes(void *buf, int nbytes)
{
//	trace_get_random_bytes(nbytes, _RET_IP_);
//	extract_entropy(&nonblocking_pool, buf, nbytes, 0, 0);
    int idx;
    int r,intnum = nbytes/sizeof(int);
    char *p;

    if(intnum == 0) {
        r = rand();
        memcpy(buf,&r,nbytes);
        return;
    }
    p = (char *)buf;
    for(idx = 0;idx < intnum;idx+=sizeof(int)) {
        r = rand();
        memcpy(&p[idx],&r,sizeof(int));
    }
}

int net_ratelimit(void)
{
    return 1;
}

static struct rnd_state net_rand_state[MAXCPU];

/**
 *	prandom_u32_state - seeded pseudo-random number generator.
 *	@state: pointer to state structure holding seeded state.
 *
 *	This is used for pseudo-randomness with no outside seeding.
 *	For more random results, use prandom_u32().
 */
u32 prandom_u32_state(struct rnd_state *state)
{
#define TAUSWORTHE(s,a,b,c,d) ((s&c)<<d) ^ (((s <<a) ^ s)>>b)

	state->s1 = TAUSWORTHE(state->s1,  6U, 13U, 4294967294U, 18U);
	state->s2 = TAUSWORTHE(state->s2,  2U, 27U, 4294967288U,  2U);
	state->s3 = TAUSWORTHE(state->s3, 13U, 21U, 4294967280U,  7U);
	state->s4 = TAUSWORTHE(state->s4,  3U, 12U, 4294967168U, 13U);

	return (state->s1 ^ state->s2 ^ state->s3 ^ state->s4);
}
EXPORT_SYMBOL(prandom_u32_state);

/**
 *	prandom_u32 - pseudo random number generator
 *
 *	A 32 bit pseudo-random number is generated using a fast
 *	algorithm suitable for simulation. This algorithm is NOT
 *	considered safe for cryptographic use.
 */
u32 prandom_u32(void)
{
	unsigned long r;
	struct rnd_state *state = &net_rand_state[rte_lcore_id()];
	r = prandom_u32_state(state);
	put_cpu_var(state);
	return r;
}
EXPORT_SYMBOL(prandom_u32);
void security_inet_csk_clone(struct sock *newsk,
			const struct request_sock *req)
{
}
#if 0
void security_sk_classify_flow(struct sock *sk, struct flowi *fl)
{
}

void security_req_classify_flow(const struct request_sock *req, struct flowi *fl)
{
}
void security_skb_owned_by(struct sk_buff *skb, struct sock *sk)
{
}

void yield()
{
}

void smp_mb()
{
}

void smp_wmb()
{
}
#endif
int smp_processor_id()
{
    return 0;
}
void msleep(int ms)
{
}
void free_percpu(void __percpu *ptr)
{
}
void netif_carrier_on(struct net_device *dev)
{
}
void netif_carrier_off(struct net_device *dev)
{
}

int netpoll_receive_skb(struct sk_buff *skb)
{
    return 0;
}

int get_cpu()
{
    return 0;
}

//int num_online_cpus()
//{
 //   return 0;
//}

unsigned int full_name_hash(char *data,int len)
{
    return 0;
}
void linkwatch_init_dev(struct net_device *dev)
{
}
void linkwatch_fire_event(struct net_device *dev)
{
}
void linkwatch_forget_dev(struct net_device *dev)
{
}
void rtnl_lock(void)
{
}
void rtnl_unlock(void)
{
}
int rtnl_trylock(void)
{
    return 1;
}
int rtnl_is_locked(void)
{
    return 1;
}
void rtnl_register(int protocol, int msgtype,
		   rtnl_doit_func doit, rtnl_dumpit_func dumpit, rtnl_calcit_func calcit)
{
}
int rtnl_unregister(int protocol, int msgtype)
{
    return 0;
}
void rtnl_notify(struct sk_buff *skb, struct net *net, u32 pid,
			u32 group, struct nlmsghdr *nlh, gfp_t flags)
{
}
int raw_smp_processor_id()
{
    return 0;
}
#define ADDCARRY(x)  (x > 65535 ? x -= 65535 : x)
#define REDUCE {l_util.l = sum; sum = l_util.s[0] + l_util.s[1]; ADDCARRY(sum);}

__wsum csum_partial_generic(const void *src, int len, __wsum currsum,int *src_err_ptr, int *dst_err_ptr)
{
    register u_short *w = (u_short *)src;
    register int sum = currsum;
    register int mlen = len;
    int byte_swapped = 0;
    union {
          char    c[2];
          u_short s;
    } s_util;
    union {
          u_short s[2];
          long    l;
    } l_util;
    /*
     * Force to even boundary.
     */
    if ((1 & (uintptr_t) w) && (mlen > 0)) {
           REDUCE;
           sum <<= 8;
           s_util.c[0] = *(u_char *)w;
           w = (u_short *)((char *)w + 1);
           mlen--;
           byte_swapped = 1;
    }
    /*
     * Unroll the loop to make overhead from
     * branches &c small.
     */
    while ((mlen -= 32) >= 0) {
           sum += w[0]; sum += w[1]; sum += w[2]; sum += w[3];
           sum += w[4]; sum += w[5]; sum += w[6]; sum += w[7];
           sum += w[8]; sum += w[9]; sum += w[10]; sum += w[11];
           sum += w[12]; sum += w[13]; sum += w[14]; sum += w[15];
           w += 16;
    }
    mlen += 32;
    while ((mlen -= 8) >= 0) {
             sum += w[0]; sum += w[1]; sum += w[2]; sum += w[3];
             w += 4;
    }
    mlen += 8;
    if (mlen == 0 && byte_swapped == 0)
          goto finished;
    REDUCE;
    while ((mlen -= 2) >= 0) {
           sum += *w++;
    }
    if (byte_swapped) {
            REDUCE;
            sum <<= 8;
            byte_swapped = 0;
            if (mlen == -1) {
                   s_util.c[1] = *(char *)w;
                   sum += s_util.s;
                   mlen = 0;
            } else
                   mlen = -1;
    } else if (mlen == -1)
            s_util.c[0] = *(char *)w;
    if (mlen == -1) {
            /* The last mbuf has odd # of bytes. Follow the
               standard (the odd byte may be shifted left by 8 bits
               or not as determined by endian-ness of the machine) */
            s_util.c[1] = 0;
            sum += s_util.s;
     }
     REDUCE;
finished:
     return (sum & 0xffff);
}

int __must_check kstrtoul(const char *s, unsigned int base, unsigned long *res)
{
	/*
	 * We want to shortcut function call, but
	 * __builtin_types_compatible_p(unsigned long, unsigned long long) = 0.
	 */
	if (sizeof(unsigned long) == sizeof(unsigned long long) &&
	    __alignof__(unsigned long) == __alignof__(unsigned long long))
		return kstrtoull(s, base, (unsigned long long *)res);
	else
		return _kstrtoul(s, base, res);
}

int ip_mc_sf_allow(struct sock *sk, __be32 loc_addr, __be32 rmt_addr, int dif)
{
    return 0;
}
int ip_check_mc_rcu(struct in_device *in_dev, __be32 mc_addr, __be32 src_addr, u16 proto)
{
   return 0;
}
int ip_mc_join_group(struct sock *sk , struct ip_mreqn *imr)
{
    return 0;
}

int ip_mc_leave_group(struct sock *sk, struct ip_mreqn *imr)
{
    return 0;
}

int ip_mc_msfilter(struct sock *sk, struct ip_msfilter *msf, int ifindex)
{
    return 0;
}

int ip_mc_msfget(struct sock *sk, struct ip_msfilter *msf,
	struct ip_msfilter __user *optval, int __user *optlen)
{
    return 0;
}

int ip_mc_gsfget(struct sock *sk, struct group_filter *gsf,
	struct group_filter __user *optval, int __user *optlen)
{
    return 0;
}

void ip_mc_drop_socket(struct sock *sk)
{
}

int put_cmsg(struct msghdr * msg, int level, int type, int len, void *data)
{
    return 0;
}


#if 0
static inline u64 div_u64_rem(u64 dividend, u32 divisor, u32 *remainder)
{
	union {
		u64 v64;
		u32 v32[2];
	} d = { dividend };
	u32 upper;

	upper = d.v32[1];
	d.v32[1] = 0;
	if (upper >= divisor) {
		d.v32[1] = upper / divisor;
		upper %= divisor;
	}
	asm ("divl %2" : "=a" (d.v32[0]), "=d" (*remainder) :
		"rm" (divisor), "0" (d.v32[0]), "1" (upper));
	return d.v64;
}
s64 div_s64_rem(s64 dividend, s32 divisor, s32 *remainder)
{
	u64 quotient;

	if (dividend < 0) {
		quotient = div_u64_rem(-dividend, abs(divisor), (u32 *)remainder);
		*remainder = -*remainder;
		if (divisor > 0)
			quotient = -quotient;
	} else {
		quotient = div_u64_rem(dividend, abs(divisor), (u32 *)remainder);
		if (divisor < 0)
			quotient = -quotient;
	}
	return quotient;
}
EXPORT_SYMBOL(div_s64_rem);
#endif

void security_inet_conn_established(struct sock *sk,
			struct sk_buff *skb)
{
//	security_ops->inet_conn_established(sk, skb);
}

int security_inet_conn_request(struct sock *sk,
			struct sk_buff *skb, struct request_sock *req)
{
//	return security_ops->inet_conn_request(sk, skb, req);
	return 0;
}
#if 0
void security_sock_graft(struct sock *sk, struct socket *parent)
{
//	security_ops->sock_graft(sk, parent);
}
#endif
int security_socket_create(int family, int type, int protocol, int kern)
{
//	return security_ops->socket_create(family, type, protocol, kern);
	return 0;
}

int security_socket_post_create(struct socket *sock, int family,
				int type, int protocol, int kern)
{
//	return security_ops->socket_post_create(sock, family, type,
//						protocol, kern);
	return 0;
}
int security_sk_alloc(struct sock *sk, int family, gfp_t priority)
{
//	return security_ops->sk_alloc_security(sk, family, priority);
	return 0;
}

void security_sk_free(struct sock *sk)
{
//	security_ops->sk_free_security(sk);
}
void security_skb_classify_flow(struct sk_buff *skb, struct flowi *fl)
{
//	int rc = security_ops->xfrm_decode_session(skb, &fl->flowi_secid, 0);

//	BUG_ON(rc);
}
int security_secctx_to_secid(const char *secdata, u32 seclen, u32 *secid)
{
//	return security_ops->secctx_to_secid(secdata, seclen, secid);
	return 0;
}
int security_secid_to_secctx(u32 secid, char **secdata, u32 *seclen)
{
//	return security_ops->secid_to_secctx(secid, secdata, seclen);
	return 0;
}
EXPORT_SYMBOL(security_secid_to_secctx);
EXPORT_SYMBOL(security_secctx_to_secid);

void security_release_secctx(char *secdata, u32 seclen)
{
//	security_ops->release_secctx(secdata, seclen);
}
int security_socket_getpeersec_dgram(struct socket *sock, struct sk_buff *skb, u32 *secid)
{
//	return security_ops->socket_getpeersec_dgram(sock, skb, secid);
	return 0;
}
int security_socket_getpeersec_stream(struct socket *sock, char __user *optval,
				      int __user *optlen, unsigned len)
{
//	return security_ops->socket_getpeersec_stream(sock, optval, optlen, len);
	return 0;
}
EXPORT_SYMBOL(security_socket_getpeersec_dgram);
EXPORT_SYMBOL(security_release_secctx);
EXPORT_SYMBOL(security_skb_classify_flow);
EXPORT_SYMBOL(security_sock_graft);
EXPORT_SYMBOL(security_inet_conn_request);


int sysctl_igmp_max_msf __read_mostly = IP_MAX_MSF;

unsigned long __per_cpu_offset[NR_CPUS];
unsigned long *pcpu_unit_offsets = NULL;
struct cpumask *cpu_possible_mask;
struct cpumask *cpu_online_mask;
struct cpumask *cpu_present_mask;
struct cpumask *cpu_active_mask;
int nr_cpu_ids = 0;
void init_lcores()
{
	nr_cpu_ids = 0;
	cpu_possible_mask = rte_zmalloc(NULL,sizeof(struct cpumask),0);
	cpu_online_mask = rte_zmalloc(NULL,sizeof(struct cpumask),0);
	cpu_present_mask = rte_zmalloc(NULL,sizeof(struct cpumask),0);
	cpu_active_mask = rte_zmalloc(NULL,sizeof(struct cpumask),0);
	RTE_LCORE_FOREACH(nr_cpu_ids)
	{
		if(rte_lcore_is_enabled(nr_cpu_ids))
		{
			cpumask_set_cpu(nr_cpu_ids,cpu_possible_mask);
			cpumask_set_cpu(nr_cpu_ids,cpu_online_mask);
			cpumask_set_cpu(nr_cpu_ids,cpu_present_mask);
			cpumask_set_cpu(nr_cpu_ids,cpu_active_mask);
		}
	}
}
void *__alloc_percpu(int size, int align)
{
//	return pcpu_alloc(size, align, false);
	return rte_zmalloc(NULL,size,align);
}
void __init setup_per_cpu_areas(void)
{
	unsigned long delta;
	unsigned int cpu;
	int rc;
#if 0
	/*
	 * Always reserve area for module percpu variables.  That's
	 * what the legacy allocator did.
	 */
	rc = pcpu_embed_first_chunk(PERCPU_MODULE_RESERVE,
				    PERCPU_DYNAMIC_RESERVE, PAGE_SIZE, NULL,
				    pcpu_dfl_fc_alloc, pcpu_dfl_fc_free);
	if (rc < 0)
		panic("Failed to initialize percpu areas.");

	delta = (unsigned long)pcpu_base_addr - (unsigned long)__per_cpu_start;
#else
	delta = 0;
#endif
	for_each_possible_cpu(cpu)
		__per_cpu_offset[cpu] = delta + pcpu_unit_offsets[cpu];
}
void prandom_seed(u32 entropy)
{
}

uint16_t linux_dpdk_cpu_to_be16(__u16 x)
{
	return rte_cpu_to_be_16(x);
}

int strncpy_from_user ( char * destination, const char * source, int num )
{
	strncpy(destination,source,num);
	return num;
}
int net_msg_warn __read_mostly = 1;
