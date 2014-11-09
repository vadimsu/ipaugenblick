#include <specific_includes/dummies.h>
#include <specific_includes/linux/types.h>
#include <specific_includes/linux/bitops.h>
#include <specific_includes/linux/slab.h>
#include <specific_includes/linux/hash.h>
#include <specific_includes/linux/socket.h>
#include <specific_includes/linux/sockios.h>
#include <specific_includes/linux/if_ether.h>
#include <specific_includes/linux/netdevice.h>
#include <specific_includes/linux/etherdevice.h>
#include <specific_includes/linux/ethtool.h>
#include <specific_includes/linux/skbuff.h>
#include <specific_includes/net/net_namespace.h>
#include <specific_includes/net/sock.h>
#include <specific_includes/linux/rtnetlink.h>
#include <specific_includes/net/dst.h>
#include <specific_includes/net/checksum.h>
#include <specific_includes/linux/err.h>
#include <specific_includes/linux/if_arp.h>
#include <specific_includes/linux/if_vlan.h>
#include <specific_includes/linux/ip.h>
#include <specific_includes/net/ip.h>
#include <specific_includes/linux/ipv6.h>
#include <specific_includes/linux/in.h>
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
uint64_t user_on_tx_opportunity_api_mbufs_sent = 0;
uint64_t user_on_tx_opportunity_cannot_get_buff = 0;
uint64_t user_on_rx_opportunity_called = 0;
uint64_t user_on_rx_opportunity_called_wo_result = 0;
uint64_t user_rx_mbufs = 0;

#ifdef OPTIMIZE_SENDPAGES
/* this is called from tcp_sendpages when tcp knows exactly
 * how much data to  send. copy contains a max buffer size,
 * it must be updated by the callee
 * A user can also set a socket's user private data
 * for its applicative needs
 */
struct rte_mbuf *user_get_buffer(struct sock *sk,int *copy)
{
	struct rte_mbuf *mbuf, *first = NULL,*prev;
	user_on_tx_opportunity_getbuff_called++;

        while(*copy != 0) {
  	    mbuf = app_glue_get_buffer();
	    if (unlikely(mbuf == NULL)) {
		user_on_tx_opportunity_cannot_get_buff++;
		return first;
	    }
	    mbuf->pkt.data_len = (*copy) > 1448 ? 1448 : (*copy);
	    (*copy) -= mbuf->pkt.data_len;
	    if(unlikely(mbuf->pkt.data_len == 0)) {
	    	    rte_pktmbuf_free_seg(mbuf);
		    return first;
	    }
            if(!first)
                first = mbuf;
            else
                prev->pkt.next = mbuf;
            prev = mbuf;
            user_on_tx_opportunity_api_mbufs_sent++;
        }
	return first;
}
#endif
int user_on_transmission_opportunity(struct socket *sock)
{
	struct page page;
	int i = 0,sent = 0;
	uint32_t to_send_this_time;
	uint64_t ts = rte_rdtsc();
#ifdef OPTIMIZE_SENDPAGES
	user_on_tx_opportunity_called++;
	to_send_this_time = app_glue_calc_size_of_data_to_send(sock);

	if(likely(to_send_this_time > 0)) {
		sock->sk->sk_route_caps |= NETIF_F_SG | NETIF_F_ALL_CSUM;
		i = kernel_sendpage(sock, &page, 0,/*offset*/to_send_this_time /* size*/, 0 /*flags*/);
		if(i <= 0)
			user_on_tx_opportunity_api_failed++;
	}
	else {
		user_on_tx_opportunity_api_not_called++;
	}
#else
	/* this does not know at the moment to deal with partially sent mbuf */
	int nothing_to_send = 0;
	user_on_tx_opportunity_called++;
	to_send_this_time = app_glue_calc_size_of_data_to_send(sock);
	to_send_this_time /= 1448;/* how many buffers to send */
	if(to_send_this_time == 0) {
		user_on_tx_opportunity_api_not_called++;
		nothing_to_send = 1;
	}
	while(to_send_this_time > 0) {
		page.mbuf = app_glue_get_buffer();
		if (unlikely(page.mbuf == NULL)) {
			printf("%s %d\n",__FILE__,__LINE__);
			return = 0;
		}
		strcpy(page.mbuf->pkt.data,"SEKTOR GAZA FOREVER");
		page.mbuf->pkt.data_len = 1448;

		i = kernel_sendpage(sock, &page, 0/*offset*/,page.mbuf->pkt.data_len /* size*/, 0 /*flags*/);
		if(unlikely(i <= 0)) {
			rte_pktmbuf_free_seg(page.mbuf);
			break;
		}
		to_send_this_time--;
		sent += i;
	}
	if((!nothing_to_send)&&(!sent)){
		user_on_tx_opportunity_api_failed++;
	}
#endif
	user_on_tx_opportunity_cycles += rte_rdtsc() - ts;
	return sent;
}

void user_data_available_cbk(struct socket *sock)
{
	struct msghdr msg;
	struct iovec vec;
	struct rte_mbuf *mbuf;
	int i,dummy = 1;
	user_on_rx_opportunity_called++;
	memset(&vec,0,sizeof(vec));
	if(unlikely(sock == NULL)) {
		return;
	}
	while(unlikely((i = kernel_recvmsg(sock, &msg,&vec, 1 /*num*/, 1448 /*size*/, 0 /*flags*/)) > 0)) {
		dummy = 0;
		while(unlikely(mbuf = msg.msg_iov->head)) {
                    {
                        int j;
                        unsigned char *b = (unsigned char *)mbuf->pkt.data;
                        for(j = 0;j < 40;j++) {
                            if(!(j%8))
                                printf("\n");
                            printf("  %x",b[j]);
                        }
                    }
		    msg.msg_iov->head = msg.msg_iov->head->pkt.next;
                    user_rx_mbufs++;
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
	user_data_available_cbk(sock);/* flush data */
}
int user_on_accept(struct socket *sock)
{
	struct socket *newsock = NULL;
	while(likely(kernel_accept(sock, &newsock, 0) == 0)) {
		newsock->sk->sk_route_caps |= NETIF_F_SG |NETIF_F_ALL_CSUM|NETIF_F_GSO;
	}
}
extern struct socket *packet_sock;
void app_main_loop()
{
    uint8_t ports_to_poll[1] = { 0 };
	int drv_poll_interval = get_max_drv_poll_interval_in_micros(0);
	app_glue_init_poll_intervals(drv_poll_interval/(2*MAX_PKT_BURST),
	                             1000 /*timer_poll_interval*/,
	                             drv_poll_interval/(10*MAX_PKT_BURST),
	                             drv_poll_interval/(60*MAX_PKT_BURST));
	while(1) {
		app_glue_periodic(0,ports_to_poll,1);
                user_data_available_cbk(packet_sock);
//                user_on_transmission_opportunity(packet_sock);
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
	printf("user_rx_mbufs %"PRIu64"\n",user_rx_mbufs,user_rx_mbufs);
        printf("user_on_tx_opportunity_api_mbufs_sent %"PRIu64"\n",user_on_tx_opportunity_api_mbufs_sent);
}
