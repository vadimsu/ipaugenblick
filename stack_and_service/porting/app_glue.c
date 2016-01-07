/*
 * app_glue.c
 *
 *  Created on: Jul 6, 2014
 *      Author: Vadim Suraev vadim.suraev@gmail.com
 *  Contains API functions for building applications
 *  on the top of Linux TCP/IP ported to userland and integrated with DPDK 1.6
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
#include "service/ipaugenblick_common/ipaugenblick_common.h"
#include "service/ipaugenblick_service/ipaugenblick_server_side.h"
#include <user_callbacks.h>
#include <ipaugenblick_log.h>

TAILQ_HEAD(read_ready_socket_list_head, socket) read_ready_socket_list_head[MAXCPU];
TAILQ_HEAD(closed_socket_list_head, socket) closed_socket_list_head[MAXCPU];
TAILQ_HEAD(write_ready_socket_list_head, socket) write_ready_socket_list_head[MAXCPU];
TAILQ_HEAD(accept_ready_socket_list_head, socket) accept_ready_socket_list_head[MAXCPU];
uint64_t read_sockets_queue_len[MAXCPU];
uint64_t write_sockets_queue_len[MAXCPU];
uint64_t working_cycles_stat[MAXCPU];
uint64_t total_cycles_stat[MAXCPU];
uint64_t total_prev[MAXCPU];
uint64_t work_prev[MAXCPU];
uint64_t app_glue_sock_readable_called[MAXCPU];
uint64_t app_glue_sock_writable_called[MAXCPU];
/*
 * This callback function is invoked when data arrives to socket.
 * It inserts the socket into a list of readable sockets
 * which is processed in periodic function app_glue_periodic
 * Paramters: a pointer to struct sock, len (dummy)
 * Returns: void
 *
 */
void app_glue_sock_readable(struct sock *sk, int len)
{
	const struct tcp_sock *tp = tcp_sk(sk);
	
	if(!sk->sk_socket) {
		return;
	}
	if((sk->sk_state != TCP_ESTABLISHED)&&(sk->sk_socket->type == SOCK_STREAM)) {
		return;
	}
	if(sk->sk_socket->read_queue_present) {
		if(read_sockets_queue_len[rte_lcore_id()] == 0) {
			ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"%s %d\n",__FILE__,__LINE__);
			exit(0);
		}
		return;
	}
	app_glue_sock_readable_called[rte_lcore_id()]++;
	sock_hold(sk);
	sk->sk_socket->read_queue_present = 1;
	TAILQ_INSERT_TAIL(&read_ready_socket_list_head[rte_lcore_id()],sk->sk_socket,read_queue_entry);
        read_sockets_queue_len[rte_lcore_id()]++;
}
/*
 * This callback function is invoked when data canbe transmitted on socket.
 * It inserts the socket into a list of writable sockets
 * which is processed in periodic function app_glue_periodic
 * Paramters: a pointer to struct sock
 * Returns: void
 *
 */
void app_glue_sock_write_space(struct sock *sk)
{	
	if(!sk->sk_socket) {
		return;
	}
	if((sk->sk_state != TCP_ESTABLISHED)&&(sk->sk_socket->type == SOCK_STREAM)) {
		return;
	}
	if (sk_stream_is_writeable(sk)) {
		clear_bit(SOCK_NOSPACE, &sk->sk_socket->flags);
		if(sk->sk_socket->write_queue_present) {
			return;
		}
		app_glue_sock_writable_called[rte_lcore_id()]++;
		sock_hold(sk);
		sk->sk_socket->write_queue_present = 1;
		TAILQ_INSERT_TAIL(&write_ready_socket_list_head[rte_lcore_id()],sk->sk_socket,write_queue_entry);
                write_sockets_queue_len[rte_lcore_id()]++;
	}
}
/*
 * This callback function is invoked when an error occurs on socket.
 * It inserts the socket into a list of closable sockets
 * which is processed in periodic function app_glue_periodic
 * Paramters: a pointer to struct sock
 * Returns: void
 *
 */
void app_glue_sock_error_report(struct sock *sk)
{
	if(sk && sk->sk_socket) {
		if(sk->sk_socket->closed_queue_present) {
			return;
		}
		sock_hold(sk);
		sk->sk_socket->closed_queue_present = 1;
		TAILQ_INSERT_TAIL(&closed_socket_list_head[rte_lcore_id()],sk->sk_socket,closed_queue_entry);
	}
}
/*
 * This callback function is invoked when a new connection can be accepted on socket.
 * It looks up the parent (listening) socket for the newly established connection
 * and inserts it into the accept queue
 * which is processed in periodic function app_glue_periodic
 * Paramters: a pointer to struct sock
 * Returns: void
 *
 */
static void app_glue_sock_wakeup(struct sock *sk)
{
	struct sock *sock;
        struct tcp_sock *tp;
	tp = tcp_sk(sk);

	sock = __inet_lookup_listener(&init_net[rte_lcore_id()],
			&tcp_hashinfo[rte_lcore_id()],
			sk->sk_daddr,
			sk->sk_dport/*__be16 sport*/,
			sk->sk_rcv_saddr,
			ntohs(tp->inet_conn.icsk_inet.inet_sport),//sk->sk_num/*const unsigned short hnum*/,
			sk->sk_bound_dev_if);
	if(sock) {
		if(sock->sk_socket->accept_queue_present) {
			return;
		}
		sock_hold(sock);
		sock->sk_socket->accept_queue_present = 1;
		TAILQ_INSERT_TAIL((&accept_ready_socket_list_head[rte_lcore_id()]),sock->sk_socket,accept_queue_entry);
	}
        else {
	      app_glue_sock_write_space(sk);
              //ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"%s %d %x %d %x %d %d \n",__FILE__,__LINE__,sk->sk_daddr,sk->sk_dport,sk->sk_rcv_saddr,sk->sk_num,tp->inet_conn.icsk_inet.inet_sport);
        }
	sock_reset_flag(sk,SOCK_USE_WRITE_QUEUE);
	sk->sk_data_ready = app_glue_sock_readable;
	sk->sk_write_space = app_glue_sock_write_space;
	sk->sk_error_report = app_glue_sock_error_report;
//	sk->sk_destruct= app_glue_sock_error_report;
}

void *app_glue_create_socket(int family,int type)
{
	struct timeval tv;
	struct socket *sock = NULL;
	if(sock_create_kern(family,type,0,&sock)) {
		ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot create socket %s %d\n",__FILE__,__LINE__);
		return NULL;
	}
	tv.tv_sec = -1;
	tv.tv_usec = 0;
	if(sock_setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char *)&tv,sizeof(tv))) {
		ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"%s %d cannot set notimeout option\n",__FILE__,__LINE__);
	}
	tv.tv_sec = -1;
	tv.tv_usec = 0;
	if(sock_setsockopt(sock,SOL_SOCKET,SO_SNDTIMEO,(char *)&tv,sizeof(tv))) {
		ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"%s %d cannot set notimeout option\n",__FILE__,__LINE__);
	}
	if(type != SOCK_STREAM) {
		if(sock->sk) {
            		sock_reset_flag(sock->sk,SOCK_USE_WRITE_QUEUE);
            		sock->sk->sk_data_ready = app_glue_sock_readable;
            		sock->sk->sk_write_space = app_glue_sock_write_space;
            		app_glue_sock_write_space(sock->sk);
		}
	}
	return sock;
}

int app_glue_v4_bind(struct socket *sock,unsigned int ipaddr, unsigned short port)
{
	struct sockaddr_in sin;

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = ipaddr;
	sin.sin_port = port;

	return kernel_bind(sock,(struct sockaddr *)&sin,sizeof(sin));
}

int app_glue_v4_connect(struct socket *sock,unsigned int ipaddr,unsigned short port)
{
	struct sockaddr_in sin;

	if(!sock->sk)
		return -1;
        struct inet_sock *inet = inet_sk(sock->sk);
	while(!inet->inet_num) {
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = /*my_ip_addr*/0;
		sin.sin_port = htons(rand() & 0xffff);
		if(kernel_bind(sock,(struct sockaddr *)&sin,sizeof(sin))) {
			ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot bind %s %d %d\n",__FILE__,__LINE__,sin.sin_port);
			continue;
		}
		break;
	}
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = ipaddr;
	sin.sin_port = port;
	if(sock->sk) {
		sock->sk->sk_state_change = app_glue_sock_wakeup;
	}
	kernel_connect(sock, (struct sockaddr *)&sin,sizeof(sin), 0);
	return 0;
}

int app_glue_v4_listen(struct socket *sock)
{
	if(sock->sk) {
		sock->sk->sk_state_change = app_glue_sock_wakeup;
	}
	else {
		ipaugenblick_log(IPAUGENBLICK_LOG_CRIT,"FATAL %s %d\n",__FILE__,__LINE__);exit(0);
	}
	if(kernel_listen(sock,32000)) {
		ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot listen %s %d\n",__FILE__,__LINE__);
		return -1;
	}
	return 0;
}

/*
 * This function polls the driver for the received packets.Called from app_glue_periodic
 * Paramters: ethernet port number.
 * Returns: None
 *
 */
static inline void app_glue_poll(int port_num)
{
	struct net_device *netdev = (struct net_device *)get_dpdk_dev_by_port_num(port_num);

	if(!netdev) {
		ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"Cannot get netdev %s %d\n",__FILE__,__LINE__);
		return;
	}
	netdev->netdev_ops->ndo_poll_controller(netdev);
}

/*
 * This function must be called before app_glue_periodic is called for the first time.
 * It initializes the readable, writable and acceptable (listeners) lists
 * Paramters: None
 * Returns: None
 *
 */
void app_glue_init()
{
	TAILQ_INIT(&read_ready_socket_list_head[rte_lcore_id()]);
	TAILQ_INIT(&write_ready_socket_list_head[rte_lcore_id()]);
	TAILQ_INIT(&accept_ready_socket_list_head[rte_lcore_id()]);
	TAILQ_INIT(&closed_socket_list_head[rte_lcore_id()]);
}
/*
 * This function walks on closable, acceptable and readable lists and calls.
 * the application's (user's) function. Called from app_glue_periodic
 * Paramters: None
 * Returns: None
 *
 */
static inline void process_rx_ready_sockets()
{
	struct socket *sock;
        uint64_t idx,limit;

	while(!TAILQ_EMPTY(&closed_socket_list_head[rte_lcore_id()])) {
		sock = TAILQ_FIRST(&closed_socket_list_head[rte_lcore_id()]);
		user_on_socket_fatal(sock);
		sock->closed_queue_present = 0;
		TAILQ_REMOVE(&closed_socket_list_head[rte_lcore_id()],sock,closed_queue_entry);
		sock_put(sock->sk);
		kernel_close(sock);
	}
	while(!TAILQ_EMPTY(&accept_ready_socket_list_head[rte_lcore_id()])) {

		sock = TAILQ_FIRST(&accept_ready_socket_list_head[rte_lcore_id()]);

		user_on_accept(sock);
		sock->accept_queue_present = 0;
		TAILQ_REMOVE(&accept_ready_socket_list_head[rte_lcore_id()],sock,accept_queue_entry);
		sock_put(sock->sk);
	}
        idx = 0;
        limit = read_sockets_queue_len[rte_lcore_id()];
	while((idx < limit)&&(!TAILQ_EMPTY(&read_ready_socket_list_head[rte_lcore_id()]))) {
		sock = TAILQ_FIRST(&read_ready_socket_list_head[rte_lcore_id()]);
                sock->read_queue_present = 0;
		TAILQ_REMOVE(&read_ready_socket_list_head[rte_lcore_id()],sock,read_queue_entry);
                user_data_available_cbk(sock);
		sock_put(sock->sk);
                read_sockets_queue_len[rte_lcore_id()]--;
                idx++;	
	}
}
/*
 * This function walks on writable lists and calls.
 * the application's (user's) function. Called from app_glue_periodic
 * Paramters: None
 * Returns: None
 *
 */
static inline void process_tx_ready_sockets()
{
	struct socket *sock;
        uint64_t idx,limit;
 
        idx = 0;
        limit = write_sockets_queue_len[rte_lcore_id()];
	while((idx < limit)&&(!TAILQ_EMPTY(&write_ready_socket_list_head[rte_lcore_id()]))) {
		sock = TAILQ_FIRST(&write_ready_socket_list_head[rte_lcore_id()]);
		TAILQ_REMOVE(&write_ready_socket_list_head[rte_lcore_id()],sock,write_queue_entry);
                sock->write_queue_present = 0;
		user_on_transmission_opportunity(sock);
                set_bit(SOCK_NOSPACE, &sock->flags);
		sock_put(sock->sk);
                write_sockets_queue_len[rte_lcore_id()]--;
	        idx++;
	}
}
/* These are in translation of micros to cycles */
static uint64_t app_glue_drv_poll_interval[MAXCPU];
static uint64_t app_glue_timer_poll_interval[MAXCPU];
static uint64_t app_glue_tx_ready_sockets_poll_interval[MAXCPU];
static uint64_t app_glue_rx_ready_sockets_poll_interval[MAXCPU];

static uint64_t app_glue_drv_last_poll_ts[MAXCPU];
static uint64_t app_glue_timer_last_poll_ts[MAXCPU];
static uint64_t app_glue_tx_ready_sockets_last_poll_ts[MAXCPU];
static uint64_t app_glue_rx_ready_sockets_last_poll_ts[MAXCPU];

/*
 * This function must be called by application to initialize.
 * the rate of polling for driver, timer, readable & writable socket lists
 * Paramters: drv_poll_interval,timer_poll_interval,tx_ready_sockets_poll_interval,
 * rx_ready_sockets_poll_interval - all in micros
 * Returns: None
 *
 */
void app_glue_init_poll_intervals(int drv_poll_interval,
		                          int timer_poll_interval,
		                          int tx_ready_sockets_poll_interval,
		                          int rx_ready_sockets_poll_interval)
{
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"%s %d %d %d %d %d\n",__func__,__LINE__,
			drv_poll_interval,timer_poll_interval,tx_ready_sockets_poll_interval,
			rx_ready_sockets_poll_interval);
	float cycles_in_micro = rte_get_tsc_hz()/1000000;
	app_glue_drv_poll_interval[rte_lcore_id()] = cycles_in_micro*(float)drv_poll_interval;
	app_glue_timer_poll_interval[rte_lcore_id()] = cycles_in_micro*(float)timer_poll_interval;
	app_glue_tx_ready_sockets_poll_interval[rte_lcore_id()] = cycles_in_micro*(float)tx_ready_sockets_poll_interval;
	app_glue_rx_ready_sockets_poll_interval[rte_lcore_id()] = cycles_in_micro*(float)rx_ready_sockets_poll_interval;
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"%s %d %"PRIu64" %"PRIu64" %"PRIu64" %"PRIu64"\n",__func__,__LINE__,
			app_glue_drv_poll_interval[rte_lcore_id()],app_glue_timer_poll_interval[rte_lcore_id()],
			app_glue_tx_ready_sockets_poll_interval[rte_lcore_id()],app_glue_rx_ready_sockets_poll_interval[rte_lcore_id()]);
}
uint64_t app_glue_periodic_called[MAXCPU];
uint64_t app_glue_tx_queues_process[MAXCPU];
uint64_t app_glue_rx_queues_process[MAXCPU];
/*
 * This function must be called by application periodically.
 * This is the heart of the system, it performs all the driver/IP stack work
 * and timers
 * Paramters: call_flush_queues - if non-zero, the readable, closable and writable queues
 * are processed and user's functions are called.
 * Alternatively, call_flush_queues can be 0 and the application may call
 * app_glue_get_next* functions to get readable, acceptable, closable and writable sockets
 * Returns: None
 *
 */
inline void app_glue_periodic(int call_flush_queues,uint8_t *ports_to_poll,int ports_to_poll_count)
{
	uint64_t ts,ts2,ts3,ts4;
    uint8_t port_idx;

	app_glue_periodic_called[rte_lcore_id()]++;
	ts = rte_rdtsc();
	if((ts - app_glue_drv_last_poll_ts[rte_lcore_id()]) >= app_glue_drv_poll_interval[rte_lcore_id()]) {
		ts4 = rte_rdtsc();
		for(port_idx = 0;port_idx < ports_to_poll_count;port_idx++)
		    app_glue_poll(ports_to_poll[port_idx]);
		app_glue_drv_last_poll_ts[rte_lcore_id()] = ts;
		working_cycles_stat[rte_lcore_id()] += rte_rdtsc() - ts4;
	}
	if((ts - app_glue_timer_last_poll_ts[rte_lcore_id()]) >= app_glue_timer_poll_interval[rte_lcore_id()]) {
		ts3 = rte_rdtsc();
		rte_timer_manage();
		app_glue_timer_last_poll_ts[rte_lcore_id()] = ts;
		working_cycles_stat[rte_lcore_id()] += rte_rdtsc() - ts3;
	}
	if(call_flush_queues) {
		if((ts - app_glue_tx_ready_sockets_last_poll_ts[rte_lcore_id()]) >= app_glue_tx_ready_sockets_poll_interval[rte_lcore_id()]) {
			ts2 = rte_rdtsc();
			app_glue_tx_queues_process[rte_lcore_id()]++;
			process_tx_ready_sockets();
			working_cycles_stat[rte_lcore_id()] += rte_rdtsc() - ts2;
			app_glue_tx_ready_sockets_last_poll_ts[rte_lcore_id()] = ts;
		}
		if((ts - app_glue_rx_ready_sockets_last_poll_ts[rte_lcore_id()]) >= app_glue_rx_ready_sockets_poll_interval[rte_lcore_id()]) {
			ts2 = rte_rdtsc();
			app_glue_rx_queues_process[rte_lcore_id()]++;
			process_rx_ready_sockets();
			working_cycles_stat[rte_lcore_id()] += rte_rdtsc() - ts2;
			app_glue_rx_ready_sockets_last_poll_ts[rte_lcore_id()] = ts;
		}
	}
	else {
		app_glue_tx_ready_sockets_last_poll_ts[rte_lcore_id()] = ts;
		app_glue_rx_ready_sockets_last_poll_ts[rte_lcore_id()] = ts;
	}
	total_cycles_stat[rte_lcore_id()] += rte_rdtsc() - ts;
}
/*
 * This function may be called to attach user's data to the socket.
 * Paramters: a pointer  to socket (returned, for example, by create_*_socket)
 * a pointer to data to be attached to the socket
 * Returns: None
 *
 */
void app_glue_set_user_data(void *socket,void *data)
{
	struct socket *sock = (struct socket *)socket;

	if(!sock) {
		ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"PANIC: socket NULL %s %d \n",__FILE__,__LINE__);while(1);
	}
//	if(sock->sk)
		sock->sk->sk_user_data = data;
//	else
//		ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"PANIC: socket->sk is NULL\n");while(1);
}
/*
 * This function may be called to get attached to the socket user's data .
 * Paramters: a pointer  to socket (returned, for example, by create_*_socket,)
 * Returns: pointer to data to be attached to the socket
 *
 */
inline void *app_glue_get_user_data(void *socket)
{
	struct socket *sock = (struct socket *)socket;
	if(!sock) {
		ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"PANIC: socket NULL %s %d\n",__FILE__,__LINE__);while(1);
	}
	if(!sock->sk) {
		ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"PANIC: socket->sk NULL\n");while(1);
	}
	return sock->sk->sk_user_data;
}
/*
 * This function may be called to get next closable socket .
 * Paramters: None
 * Returns: pointer to socket to be closed
 *
 */
void *app_glue_get_next_closed()
{
	struct socket *sock;
	void *user_data;
	if(!TAILQ_EMPTY(&closed_socket_list_head[rte_lcore_id()])) {
		sock = TAILQ_FIRST(&closed_socket_list_head[rte_lcore_id()]);
		sock->closed_queue_present = 0;
		TAILQ_REMOVE(&closed_socket_list_head[rte_lcore_id()],sock,closed_queue_entry);
		if(sock->sk)
			user_data = sock->sk->sk_user_data;
			//kernel_close(sock);
		return user_data;
	}
	return NULL;
}
/*
 * This function may be called to get next writable socket .
 * Paramters: None
 * Returns: pointer to socket to be written
 *
 */
void *app_glue_get_next_writer()
{
	struct socket *sock;

	if(!TAILQ_EMPTY(&write_ready_socket_list_head[rte_lcore_id()])) {
		sock = TAILQ_FIRST(&write_ready_socket_list_head[rte_lcore_id()]);
		sock->write_queue_present = 0;
		TAILQ_REMOVE(&write_ready_socket_list_head[rte_lcore_id()],sock,write_queue_entry);
		if(sock->sk)
		    return sock->sk->sk_user_data;
  	    ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"PANIC: socket->sk is NULL\n");
	}
	return NULL;
}
/*
 * This function may be called to get next readable socket .
 * Paramters: None
 * Returns: pointer to socket to be read
 *
 */
void *app_glue_get_next_reader()
{
	struct socket *sock;
	if(!TAILQ_EMPTY(&read_ready_socket_list_head[rte_lcore_id()])) {
		sock = TAILQ_FIRST(&read_ready_socket_list_head[rte_lcore_id()]);
		sock->read_queue_present = 0;
		TAILQ_REMOVE(&read_ready_socket_list_head[rte_lcore_id()],sock,read_queue_entry);
		if(sock->sk)
		    return sock->sk->sk_user_data;
	    ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"PANIC: socket->sk is NULL\n");
	}
	return NULL;
}
/*
 * This function may be called to get next acceptable socket .
 * Paramters: None
 * Returns: pointer to socket on which to accept a new connection
 *
 */
void *app_glue_get_next_listener()
{
	struct socket *sock;
	if(!TAILQ_EMPTY(&accept_ready_socket_list_head[rte_lcore_id()]))
	{
		sock = TAILQ_FIRST(&accept_ready_socket_list_head[rte_lcore_id()]);
		sock->accept_queue_present = 0;
		TAILQ_REMOVE(&accept_ready_socket_list_head[rte_lcore_id()],sock,accept_queue_entry);
		if(sock->sk)
	        return sock->sk->sk_user_data;
	    ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"PANIC: socket->sk is NULL\n");
		return NULL;
	}
	return NULL;
}
void sock_def_wakeup(struct sock *sk);
void sock_def_readable(struct sock *sk, int len);
void sock_def_error_report(struct sock *sk);
void sock_def_write_space(struct sock *);
/*
 * This function may be called to close socket .
 * Paramters: a pointer to socket structure
 * Returns: None
 *
 */
void app_glue_close_socket(void *sk)
{
	struct socket *sock = (struct socket *)sk;

	if(sock->read_queue_present) {
		TAILQ_REMOVE(&read_ready_socket_list_head[rte_lcore_id()],sock,read_queue_entry);
		sock->read_queue_present = 0;
		read_sockets_queue_len[rte_lcore_id()]--;
	}
	if(sock->write_queue_present) {
		TAILQ_REMOVE(&write_ready_socket_list_head[rte_lcore_id()],sock,write_queue_entry);
		sock->write_queue_present = 0;
		write_sockets_queue_len[rte_lcore_id()]--;
	}
	if(sock->accept_queue_present) {
                struct socket *newsock = NULL;

	        while(kernel_accept(sock, &newsock, 0) == 0) {
                    kernel_close(newsock);
                }
		TAILQ_REMOVE(&accept_ready_socket_list_head[rte_lcore_id()],sock,accept_queue_entry);
		sock->accept_queue_present = 0;
	}
	if(sock->closed_queue_present) {
		TAILQ_REMOVE(&closed_socket_list_head[rte_lcore_id()],sock,closed_queue_entry);
		sock->closed_queue_present = 0;
	}
	if(sock->sk)
		sock->sk->sk_user_data = NULL;
	sock->sk->sk_write_space = (sock->type == SOCK_STREAM) ? sk_stream_write_space : sock_def_write_space;
	if (sock->sk) {
		sock->sk->sk_state_change = sock_def_wakeup;
		sock->sk->sk_data_ready = sock_def_readable;
		sock->sk->sk_error_report = sock_def_error_report;
	}
//	sock->sk->sk_destruct = (sock->sk->sk_socket->type == SOCK_STREAM) ? tcp_sock_destruct : sock_def_destruct;
	kernel_close(sock);
}
/*
 * This function may be called to estimate amount of data can be sent .
 * Paramters: a pointer to socket structure
 * Returns: number of bytes the application can send
 *
 */
int app_glue_calc_size_of_data_to_send(void *sock)
{
	int bufs_count1,bufs_count2,bufs_count3,stream_space,bufs_min;
	struct sock *sk = ((struct socket *)sock)->sk;
	if(!sk_stream_is_writeable(sk)) {
		return 0;
	}
	bufs_count1 = kmem_cache_get_free(get_fclone_cache());
	bufs_count2 = kmem_cache_get_free(get_header_cache());
	bufs_count3 = get_buffer_count();
	if(bufs_count1 > 2) {
		bufs_count1 -= 2;
	}
	if(bufs_count2 > 2) {
		bufs_count2 -= 2;
	}
	bufs_min = min(bufs_count1,bufs_count2);
	bufs_min = min(bufs_min,bufs_count3);
	if(bufs_min <= 0) {
		return 0;
	}
	stream_space = sk_stream_wspace(((struct socket *)sock)->sk);
	return min(bufs_min << 10,stream_space);
}
/*
 * This function may be called to allocate rte_mbuf from existing pool.
 * Paramters: None
 * Returns: a pointer to rte_mbuf, if succeeded, NULL if failed
 *
 */
struct rte_mbuf *app_glue_get_buffer()
{
	return get_buffer();
}
void app_glue_print_stats()
{
	float ratio;
	int cpu_idx;

	for(cpu_idx = 0;cpu_idx < 4;cpu_idx++) {
		ratio = (float)(total_cycles_stat[cpu_idx] - total_prev[cpu_idx])/(float)(working_cycles_stat[cpu_idx] - work_prev[cpu_idx]);
		total_prev[cpu_idx] = total_cycles_stat[cpu_idx];
		work_prev[cpu_idx] = working_cycles_stat[cpu_idx];
		ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"total %"PRIu64" work %"PRIu64" ratio %f\n",total_cycles_stat,working_cycles_stat,ratio);
		ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"app_glue_periodic_called %"PRIu64"\n",app_glue_periodic_called[cpu_idx]);
		ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"app_glue_tx_queues_process %"PRIu64"\n",app_glue_tx_queues_process[cpu_idx]);
		ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"app_glue_rx_queues_process %"PRIu64"\n",app_glue_rx_queues_process[cpu_idx]);
		ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"app_glue_sock_readable_called %"PRIu64" app_glue_sock_writable_called %"PRIu64"\n",app_glue_sock_readable_called[cpu_idx], app_glue_sock_writable_called[cpu_idx]);
	}
}
