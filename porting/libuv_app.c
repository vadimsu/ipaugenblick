/*
 * libuv_app.c
 *
 *  Created on: Oct 4, 2014
 *      Author: Vadim Suraev vadim.suraev@gmail.com
 *  Contains API functions for building applications
 *  on the top of Linux TCP/IP ported to userland and integrated with DPDK 1.6 & libuv
 */
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
#include <specific_includes/net/tcp.h>
#include <specific_includes/linux/err.h>
#include <specific_includes/linux/if_arp.h>
#include <specific_includes/linux/if_vlan.h>
#include <specific_includes/linux/ip.h>
#include <specific_includes/net/ip.h>
#include <specific_includes/linux/ipv6.h>
#include <specific_includes/linux/in.h>
#include <specific_includes/linux/inetdevice.h>
#include <specific_includes/linux/hashtable.h>
#include <specific_includes/linux/if_macvlan.h>
#include <specific_includes/linux/if_arp.h>
#include <specific_includes/dpdk_drv_iface.h>
#include <string.h>
#include <rte_config.h>
#include <rte_common.h>
#include <rte_cycles.h>
#include <rte_timer.h>
#include <api.h>
#include <porting/libinit.h>
#include <user_callbacks.h>

#define MAX_FDS 1024

static struct socket *fd_2_socket[MAX_FDS];

void libuv_app_init()
{
    memset(fd_2_socket,0,sizeof(fd_2_socket));
}

static int get_free_fd()
{
    int idx;

    for(idx = 0;idx < MAX_FDS;idx++)
        if(fd_2_socket[idx] == NULL)
            break;
    return idx;
}
int libuv_is_fd_known(int fd)
{
   if((fd >= MAX_FDS)||(fd_2_socket[fd] == NULL))
        return 0;
   return 1;
}
int libuv_app_socket(int family,int type,int port)
{
    struct socket *sock;
    int fd = get_free_fd();

    if(family != AF_INET)
        return -3;
    if((type != SOCK_STREAM)&&(type != SOCK_DGRAM)&&(type != SOCK_RAW))
        return -4;

    if(fd == MAX_FDS)
        return -1;
    
    if(sock_create_kern(family,type,port,&sock)) {
	printf("cannot create socket %s %d\n",__FILE__,__LINE__);
	return -2;
    }
    fd_2_socket[fd] = sock;
    if(type == SOCK_STREAM) {
        struct timeval tv;
        tv.tv_sec = -1;
	tv.tv_usec = 0;
	if(sock_setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char *)&tv,sizeof(tv))) {
		printf("%s %d cannot set notimeout option\n",__FILE__,__LINE__);
	}
	tv.tv_sec = -1;
	tv.tv_usec = 0;
	if(sock_setsockopt(sock,SOL_SOCKET,SO_SNDTIMEO,(char *)&tv,sizeof(tv))) {
		printf("%s %d cannot set notimeout option\n",__FILE__,__LINE__);
	}
        if(sock->sk) {
		sock->sk->sk_state_change = app_glue_sock_wakeup;
	}
    }
    return fd;
}

int libuv_app_bind(int fd,struct sockaddr *addr,int addr_len)
{
    struct sockaddr_in sin;
   
    if(!libuv_is_fd_known(fd))
        return -1;
    if(kernel_bind(fd_2_socket[fd],addr,addr_len)) {
	printf("cannot bind %s %d\n",__FILE__,__LINE__);
	return -2;
    }
    return 0;
}

int libuv_app_connect(int fd,struct sockaddr *addr,int addr_len)
{
    if(!libuv_is_fd_known(fd))
        return -1;
    while(1) {
        struct sockaddr_in sin;
        sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(get_first_ip_from_config());
	sin.sin_port = htons(rand() & 0xffff);
	if(kernel_bind(fd_2_socket[fd],(struct sockaddr *)&sin,sizeof(sin))) {
		printf("cannot bind %s %d\n",__FILE__,__LINE__);
		continue;
	}
	break;
    }
    if(fd_2_socket[fd]->sk) {
	fd_2_socket[fd]->sk->sk_state_change = app_glue_sock_wakeup;
    }
    kernel_connect(fd_2_socket[fd], addr,addr_len, 0);
}

int libuv_app_listen(int fd,int backlog)
{
    if(!libuv_is_fd_known(fd))
        return -1;
    if(kernel_listen(fd_2_socket[fd],32000)) {
	printf("cannot listen %s %d\n",__FILE__,__LINE__);
	return -2;
    }
    return 0;
}

int libuv_app_close(int fd)
{
    if(!libuv_is_fd_known(fd))
        return -1;
    kernel_close(fd_2_socket[fd]);
    fd_2_socket[fd] = NULL;
    return 0;
}

int libuv_app_setsockopt(int fd,int scope,int optname,void *val,int valsize)
{
    if(!libuv_is_fd_known(fd))
        return -1;
    if(sock_setsockopt(fd_2_socket[fd],scope,optname,(char *)val,valsize)) {
	printf("%s %d cannot set sock option\n",__FILE__,__LINE__);
    }
    return 0;
}

int libuv_app_getsockopt(int fd,int scope,int optname,void *val,int valsize)
{
    if(!libuv_is_fd_known(fd))
        return -1;
    if(sock_getsockopt(fd_2_socket[fd],scope,optname,(char *)val,valsize)) {
	printf("%s %d cannot get sock option\n",__FILE__,__LINE__);
    }
    return 0;
}

void libuv_app_set_user_data(int fd,void *data)
{
    if(!libuv_is_fd_known(fd))
        return;
    app_glue_set_user_data(fd_2_socket[fd],data);
}

int libuv_app_recvmsg(int fd, void *arg, int len,int flags,void (*copy_to_iovec)(void *,char *,int))
{
    struct msghdr msg;
    struct iovec vec;
    struct rte_mbuf *mbuf;
    int i;
    user_on_rx_opportunity_called++;
    memset(&vec,0,sizeof(vec));
    
    if(!libuv_is_fd_known(fd))
        return -1;
    
    if(unlikely((i = kernel_recvmsg(fd_2_socket[fd], &msg,&vec, 1 /*vec size*/, len, 0 /*flags*/)) > 0)) {
	while(unlikely(mbuf = msg.msg_iov->head)) {
            copy_to_iovec(arg,mbuf->pkt.data,mbuf->pkt.data_len);
 	    msg.msg_iov->head = msg.msg_iov->head->pkt.next;
            user_rx_mbufs++;
	    rte_pktmbuf_free_seg(mbuf);
	}
	memset(&vec,0,sizeof(vec));
        return i;
    }
    user_on_rx_opportunity_called_wo_result++;
    return 0;
}
typedef struct
{
    void *arg;
    int (*copy_from_iovec)(void *,char *,int);
    int to_copy;
}tcp_sendpage_arg_wrapper_t;

struct rte_mbuf *user_get_buffer(struct sock *sk,int *copy,void *arg)
{
	struct rte_mbuf *mbuf, *first = NULL,*prev;
        tcp_sendpage_arg_wrapper_t *tcp_sendpage_arg_wrapper = (tcp_sendpage_arg_wrapper_t *)arg;
	user_on_tx_opportunity_getbuff_called++;

        while((*copy != 0)&&(tcp_sendpage_arg_wrapper->to_copy > 0)) {
  	    mbuf = app_glue_get_buffer();
	    if (unlikely(mbuf == NULL)) {
		user_on_tx_opportunity_cannot_get_buff++;
		return first;
	    }
            mbuf->pkt.data_len = 
                tcp_sendpage_arg_wrapper->copy_from_iovec(tcp_sendpage_arg_wrapper->arg,mbuf->pkt.data,min(tcp_sendpage_arg_wrapper->to_copy,(*copy)));
            tcp_sendpage_arg_wrapper->to_copy -= mbuf->pkt.data_len;
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
int libuv_app_tcp_sendmsg(int fd,void *arg,int len,int flags, int (*copy_from_iovec)(void *,char *,int))
{
    tcp_sendpage_arg_wrapper_t tcp_sendpage_arg_wrapper;
    if(!libuv_is_fd_known(fd))
        return -1;
    tcp_sendpage_arg_wrapper.arg = arg;
    tcp_sendpage_arg_wrapper.copy_from_iovec = copy_from_iovec;
    tcp_sendpage_arg_wrapper.to_copy = len;
    return kernel_sendpage(fd_2_socket[fd],(void *)&tcp_sendpage_arg_wrapper , 0,/*offset*/len /* size*/, 0 /*flags*/);
}

int libuv_app_udp_sendmsg(int fd,void *arg,int len,int flags, unsigned int addr,unsigned short port,int (*copy_from_iovec)(void *,char *,int))
{
    struct page page;
    if(!libuv_is_fd_known(fd))
        return -1;
    if(len > 1460) {
        return -2;
    }
    mbuf = app_glue_get_buffer();
    if (unlikely(mbuf == NULL)) {
       user_on_tx_opportunity_cannot_get_buff++;
       return 0;
    }
    mbuf->pkt.data_len = copy_from_iovec(arg,mbuf->pkt.data,len);
    sockaddrin.sin_family = AF_INET;
    sockaddrin.sin_addr.s_addr = addr;
    sockaddrin.sin_port = port;
    msghdr.msg_namelen = sizeof(sockaddrin);
    msghdr.msg_name = &sockaddrin;
    msghdr.msg_iov = &iov;
    iov.head = mbuf;
    msghdr.msg_iovlen = 1;
    msghdr.msg_controllen = 0;
    msghdr.msg_control = 0;
    msghdr.msg_flags = 0;
    sock->sk->sk_route_caps |= NETIF_F_SG | NETIF_F_ALL_CSUM;
    i = kernel_sendmsg(sock, &msghdr, mbuf->pkt.data_len);
    if(i <= 0) {
       rte_pktmbuf_free(mbuf);
       user_on_tx_opportunity_api_failed++;
       return 0
    }
    return i;
}

int libuv_app_tcp_receive(int fd,char *buf,int len)
{
    struct msghdr msg;
    struct iovec vec;
    struct rte_mbuf *mbuf;
    int i,copied = 0,to_copy;

    if(!libuv_is_fd_known(fd))
        return -1;
   
    memset(&vec,0,sizeof(vec));
    i = kernel_recvmsg(fd_2_socket[fd], &msg, &vec, 1 /*num*/, len /*size*/, 0 /*flags*/);
    if(i <= 0)
        return i;
    while(unlikely(mbuf = msg.msg_iov->head)) {
	msg.msg_iov->head = msg.msg_iov->head->pkt.next;
        if(copied < len) {
            to_copy = min(len - copied, mbuf->pkt.data_len);
            memcpy(&buf[copied],mbuf->pkt.data,to_copy);
            copied += to_copy;
        }
        user_rx_mbufs++;
	rte_pktmbuf_free_seg(mbuf);
    }
    return copied;
}

int libuv_app_getsockname(int fd,struct sockaddr *addr,int *addrlen)
{
    if(!libuv_is_fd_known(fd))
        return -1;
    return kernel_getsockname(fd_2_socket[fd], addr,addrlen);
}

int libuv_app_getpeername(int fd,struct sockaddr *addr,int *addrlen)
{
    if(!libuv_is_fd_known(fd))
        return -1;
    return kernel_getpeername(fd_2_socket[fd], addr,addrlen);
}

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

void app_main_loop()
{
    uint8_t ports_to_poll[1] = { 0 };
    int drv_poll_interval = get_max_drv_poll_interval_in_micros(0);
    app_glue_init_poll_intervals(drv_poll_interval/(2*MAX_PKT_BURST),
                             1000 /*timer_poll_interval*/,
                             drv_poll_interval/(10*MAX_PKT_BURST),
                             drv_poll_interval/(60*MAX_PKT_BURST));
    while(1) {
	app_glue_periodic(1,ports_to_poll,1);
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
