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
//#include <specific_includes/net/tcp.h>
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
//#include <linux/random.h>
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
#include <specific_includes/linux/if_arp.h>
//#include <specific_includes/asm-generic/bitops/le.h>
//#include "net-sysfs.h"
#include <string.h>
#include <rte_config.h>
#include <rte_common.h>
#include <rte_cycles.h>
#include <rte_timer.h>
#include <api.h>
#include <porting/libinit.h>

uint64_t user_on_tx_opportunity_cycles = 0;
uint64_t user_on_tx_opportunity_called = 0;
uint64_t user_on_tx_opportunity_getbuff_called = 0;
uint64_t user_on_tx_opportunity_api_not_called = 0;
uint64_t user_on_tx_opportunity_api_failed = 0;
uint64_t user_on_tx_opportunity_cannot_get_buff = 0;
uint64_t user_on_rx_opportunity_called = 0;
uint64_t user_on_rx_opportunity_called_wo_result = 0;

#ifdef OPTIMIZE_SENDPAGES
struct rte_mbuf *user_get_buffer(struct sock *sk,int *copy)
{
	struct rte_mbuf *mbuf;
	user_on_tx_opportunity_getbuff_called++;

	mbuf = app_glue_get_buffer();
	if (unlikely(mbuf == NULL)) {
		user_on_tx_opportunity_cannot_get_buff++;
		return NULL;
	}

	mbuf->pkt.data_len = (*copy) > 1448 ? 1448 : (*copy);
	*copy = mbuf->pkt.data_len;
	if(unlikely(mbuf->pkt.data_len == 0))
	{
		rte_pktmbuf_free_seg(mbuf);
		return NULL;
	}
	return mbuf;
}
#endif
int user_on_transmission_opportunity(struct socket *sock)
{
	struct page page;
	int i = 0;
	uint32_t to_send_this_time;
	uint64_t ts = rte_rdtsc();
	user_on_tx_opportunity_called++;
#ifdef OPTIMIZE_SENDPAGES

	to_send_this_time = app_glue_calc_size_of_data_to_send(sock);

	if(likely(to_send_this_time > 0))
	{
		sock->sk->sk_route_caps |= NETIF_F_SG | NETIF_F_ALL_CSUM;
		i = kernel_sendpage(sock, &page, 0,/*offset*/to_send_this_time /* size*/, 0 /*flags*/);
		if(i <= 0)
			user_on_tx_opportunity_api_failed++;
	}
	else
	{
		user_on_tx_opportunity_api_not_called++;
	}
#else
	while(likely((sk_stream_memory_free(sock->sk))&&
		         (kmem_cache_get_free(get_fclone_cache()) > 2)&&
		         (kmem_cache_get_free(get_header_cache()) > 2)))
	{
		page.mbuf = app_glue_get_buffer();
		if (unlikely(page.mbuf == NULL)) {
			printf("%s %d\n",__FILE__,__LINE__);
			return = 0;
		}
		strcpy(page.mbuf->pkt.data,"SEKTOR GAZA FOREVER");
		page.mbuf->pkt.data_len = 1448;
		i = kernel_sendpage(sock, &page, 0/*offset*/,page.mbuf->pkt.data_len /* size*/, 0 /*flags*/);
		if(unlikely(i <= 0))
		{
			rte_pktmbuf_free_seg(page.mbuf);
			break;
		}
	//		printf("SENT %d\n",i);
	}/*while(sock->sk->sk_forward_alloc > 1024);*/
#endif
	user_on_tx_opportunity_cycles += rte_rdtsc() - ts;
	return i;
}

void user_data_available_cbk(struct socket *sock)
{
	struct msghdr msg;
	struct iovec vec;
	struct rte_mbuf *mbuf;
	int i,dummy = 1;
	user_on_rx_opportunity_called++;
	memset(&vec,0,sizeof(vec));
	if(unlikely(sock == NULL))
	{
		return;
	}
	while(unlikely((i = kernel_recvmsg(sock, &msg,&vec, 1 /*num*/, 1448 /*size*/, 0 /*flags*/)) > 0))
	{
		dummy = 0;
		while(unlikely(mbuf = msg.msg_iov->head))
		{
			msg.msg_iov->head = msg.msg_iov->head->pkt.next;

			rte_pktmbuf_free_seg(mbuf);
		}
		memset(&vec,0,sizeof(vec));
	}
	if(dummy) {
		user_on_rx_opportunity_called_wo_result++;
	}
}
void user_on_socket_fatal(struct socket *sock)
{
	;
}
int user_on_accept(struct socket *sock)
{
	struct socket *newsock = NULL;
	while(likely(kernel_accept(sock, &newsock, 0) == 0)) {
		newsock->sk->sk_route_caps |= NETIF_F_SG |NETIF_F_ALL_CSUM;
		//user_accept_pending_cbk(newsock);
	}
}

void app_main_loop()
{
	int drv_poll_interval = get_max_drv_poll_interval_in_micros(0);
	app_glue_init_poll_intervals(drv_poll_interval/2,
			1000 /*timer_poll_interval*/,
			drv_poll_interval/10,drv_poll_interval/10);
	while(1) {
		app_glue_periodic(1);
	}
}
/*this is called in non-data-path thread */
void print_user_stats()
{
	printf("user_on_tx_opportunity_called %"PRIu64"\n",user_on_tx_opportunity_called);
	printf("user_on_tx_opportunity_api_not_called %"PRIu64"\n",user_on_tx_opportunity_api_not_called);
	printf("user_on_tx_opportunity_cannot_get_buff %"PRIu64"\n",user_on_tx_opportunity_cannot_get_buff);
	printf("user_on_tx_opportunity_getbuff_called %"PRIu64"\n",user_on_tx_opportunity_getbuff_called);
	printf("user_on_tx_opportunity_api_failed %"PRIu64"\n",	user_on_tx_opportunity_api_failed);
	printf("user_on_rx_opportunity_called %"PRIu64"\n",user_on_rx_opportunity_called);
	printf("user_on_rx_opportunity_called_wo_result %"PRIu64"\n",user_on_rx_opportunity_called_wo_result);
}
