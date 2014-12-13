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
#include "ipaugenblick_common/ipaugenblick_common.h"
#include "ipaugenblick_service/ipaugenblick_server_side.h"

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

struct rte_ring *command_ring = NULL;
struct rte_ring *free_connections_ring = NULL;
struct rte_ring *rx_mbufs_ring = NULL;
struct rte_mempool *free_command_pool = NULL;
struct ipaugenblick_ring_set ringsets[IPAUGENBLICK_CONNECTION_POOL_SIZE];

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
    unsigned int ringset_idx;

    user_on_tx_opportunity_getbuff_called++;
    while(*copy != 0) {
#if 0
        raw_buffer = app_glue_get_buffer();
#else
        ringset_idx = (unsigned int)app_glue_get_user_data(sk->sk_socket);

        mbuf = ipaugenblick_dequeue_tx_buf(ringset_idx);
        if(unlikely(mbuf == NULL)) {
            user_on_tx_opportunity_cannot_get_buff++;
            return first;
        }
#endif
        (*copy) -= mbuf->pkt.data_len;
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

	while(likely((to_send_this_time = app_glue_calc_size_of_data_to_send(sock)) > 0)) {
		sock->sk->sk_route_caps |= NETIF_F_SG | NETIF_F_ALL_CSUM;
		i = kernel_sendpage(sock, &page, 0,/*offset*/to_send_this_time /* size*/, 0 /*flags*/);
		if(i <= 0)
			user_on_tx_opportunity_api_failed++;
                else
                    sent += i;
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
    unsigned int ringset_idx;
    user_on_rx_opportunity_called++;
    memset(&vec,0,sizeof(vec));
    if(unlikely(sock == NULL)) {
	return;
    }
    while(unlikely((i = kernel_recvmsg(sock, &msg,&vec, 1 /*num*/, 1448 /*size*/, 0 /*flags*/)) > 0)) {
	dummy = 0;
        ringset_idx = (unsigned int)app_glue_get_user_data(sock);
        ipaugenblick_submit_rx_buf(msg.msg_iov->head,ringset_idx);
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
                user_on_transmission_opportunity(newsock);
	}
}

static void process_commands()
{
    int ringset_idx;
    ipaugenblick_cmd_t *cmd;
    struct rte_mbuf *mbuf;
    struct socket *sock;
    char *p;

    cmd = ipaugenblick_dequeue_command_buf();
    if(!cmd)
        return;
    switch(cmd->cmd) {
        case IPAUGENBLICK_OPEN_CLIENT_SOCKET_COMMAND:
           printf("open_client_sock %x %x %x %x\n",cmd->u.open_client_sock.my_ipaddress,cmd->u.open_client_sock.my_port,
                                             cmd->u.open_client_sock.peer_ipaddress,cmd->u.open_client_sock.peer_port);
           sock = create_client_socket2(cmd->u.open_client_sock.my_ipaddress,cmd->u.open_client_sock.my_port,
                                        cmd->u.open_client_sock.peer_ipaddress,cmd->u.open_client_sock.peer_port);
           if(sock) {
               app_glue_set_user_data(sock,(void *)cmd->ringset_idx);
           }
           break;
        case IPAUGENBLICK_OPEN_LISTENING_SOCKET_COMMAND:
           printf("open_listening_sock %x %x\n",
                  cmd->u.open_listening_sock.ipaddress,cmd->u.open_listening_sock.port);
           break;
        case IPAUGENBLICK_OPEN_UDP_SOCKET_COMMAND:
           printf("open_udp_sock %x %x\n",cmd->u.open_udp_sock.ipaddress,cmd->u.open_udp_sock.port);
           break;
        case IPAUGENBLICK_OPEN_RAW_SOCKET_COMMAND:
           printf("open_raw_sock %x %x\n",cmd->u.open_raw_sock.ipaddress,cmd->u.open_raw_sock.protocol);
           break;
        default:
           printf("unknown cmd %d\n",cmd->cmd);
           break;
    }
    ipaugenblick_free_command_buf(cmd);
}

void ipaugenblick_main_loop()
{
    struct rte_mbuf *mbuf;
    uint8_t ports_to_poll[1] = { 0 };
    int drv_poll_interval = get_max_drv_poll_interval_in_micros(0);
    app_glue_init_poll_intervals(/*drv_poll_interval/(2*MAX_PKT_BURST)*/0,
                                 1000 /*timer_poll_interval*/,
                                 /*drv_poll_interval/(10*MAX_PKT_BURST)*/0,
                                /*drv_poll_interval/(60*MAX_PKT_BURST)*/0);
    ipaugenblick_service_api_init(10,1024,1024);
    while(1) {
        process_commands();
	app_glue_periodic(1,ports_to_poll,1);	
        if(rte_ring_dequeue(rx_mbufs_ring,&mbuf) == 0) {
            rte_pktmbuf_free_seg(mbuf);
        }
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
