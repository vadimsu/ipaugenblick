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
#include <user_callbacks.h>

TAILQ_HEAD(read_ready_socket_list_head, socket) read_ready_socket_list_head[MAXCPU];
TAILQ_HEAD(closed_socket_list_head, socket) closed_socket_list_head[MAXCPU];
TAILQ_HEAD(write_ready_socket_list_head, socket) write_ready_socket_list_head[MAXCPU];
TAILQ_HEAD(accept_ready_socket_list_head, socket) accept_ready_socket_list_head[MAXCPU];
/* These are in translation of micros to cycles */
static uint64_t app_glue_drv_poll_interval[MAXCPU];
static uint64_t app_glue_timer_poll_interval[MAXCPU];
static uint64_t app_glue_tx_ready_sockets_poll_interval[MAXCPU];
static uint64_t app_glue_rx_ready_sockets_poll_interval[MAXCPU];

static uint64_t app_glue_drv_last_poll_ts[MAXCPU];
static uint64_t app_glue_timer_last_poll_ts[MAXCPU];
static uint64_t app_glue_tx_ready_sockets_last_poll_ts[MAXCPU];
static uint64_t app_glue_rx_ready_sockets_last_poll_ts[MAXCPU];
uint64_t working_cycles_stat[MAXCPU];
uint64_t total_cycles_stat[MAXCPU];
uint64_t total_prev[MAXCPU];
uint64_t work_prev[MAXCPU];
uint64_t app_glue_periodic_called[MAXCPU];
uint64_t app_glue_tx_queues_process[MAXCPU];
uint64_t app_glue_rx_queues_process[MAXCPU];
/*
 * This callback function is invoked when data arrives to socket.
 * It inserts the socket into a list of readable sockets
 * which is processed in periodic function app_glue_periodic
 * Paramters: a pointer to struct sock, len (dummy)
 * Returns: void
 *
 */
static void app_glue_sock_readable(struct sock *sk, int len)
{
	const struct tcp_sock *tp = tcp_sk(sk);
	int target = sock_rcvlowat(sk, 0, INT_MAX);

	if((sk->sk_state != TCP_ESTABLISHED)&&(sk->sk_socket->type == SOCK_STREAM)) {
		return;
	}
	if(!sk->sk_socket) {
		return;
	}
	if(sk->sk_socket->read_queue_present) {
		return;
	}
	sk->sk_socket->read_queue_present = 1;
	TAILQ_INSERT_TAIL(&read_ready_socket_list_head[rte_lcore_id()],sk->sk_socket,read_queue_entry);
}
/*
 * This callback function is invoked when data canbe transmitted on socket.
 * It inserts the socket into a list of writable sockets
 * which is processed in periodic function app_glue_periodic
 * Paramters: a pointer to struct sock
 * Returns: void
 *
 */
static void app_glue_sock_write_space(struct sock *sk)
{
	if((sk->sk_state != TCP_ESTABLISHED)&&(sk->sk_socket->type == SOCK_STREAM)) {
		return;
	}
	if (sk_stream_is_writeable(sk) && sk->sk_socket) {
		clear_bit(SOCK_NOSPACE, &sk->sk_socket->flags);
		if(sk->sk_socket->write_queue_present) {
			return;
		}
		sk->sk_socket->write_queue_present = 1;
		TAILQ_INSERT_TAIL(&write_ready_socket_list_head[rte_lcore_id()],sk->sk_socket,write_queue_entry);
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
static void app_glue_sock_error_report(struct sock *sk)
{
	if(sk->sk_socket) {
		if(sk->sk_socket->closed_queue_present) {
			return;
		}
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

	sock = __inet_lookup_listener(&init_net/*sk->sk_net*/,
			&tcp_hashinfo[rte_lcore_id()],
			sk->sk_daddr,
			sk->sk_dport/*__be16 sport*/,
			sk->sk_rcv_saddr,
			sk->sk_num/*const unsigned short hnum*/,
			sk->sk_bound_dev_if);

	if(sock) {
		if(sock->sk_socket->accept_queue_present) {
			return;
		}
		sock->sk_socket->accept_queue_present = 1;
		TAILQ_INSERT_TAIL((&accept_ready_socket_list_head[rte_lcore_id()]),sock->sk_socket,accept_queue_entry);
	}
	sock_reset_flag(sk,SOCK_USE_WRITE_QUEUE);
	sk->sk_data_ready = app_glue_sock_readable;
	sk->sk_write_space = app_glue_sock_write_space;
	sk->sk_error_report	=	app_glue_sock_error_report;
}
/*
 * This is a wrapper function for RAW socket creation.
 * Paramters: IP address & port (protocol number) to bind
 * Returns: a pointer to socket structure (handle)
 * or NULL if failed
 *
 */
void *create_raw_socket(const char *ip_addr,unsigned short port)
{
	struct sockaddr_in sin;
	struct timeval tv;
	struct socket *raw_sock = NULL;
	if(sock_create_kern(AF_INET,SOCK_RAW,port,&raw_sock)) {
		printf("cannot create socket %s %d\n",__FILE__,__LINE__);
		return NULL;
	}

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(ip_addr);
	sin.sin_port = htons(port);
	if(kernel_bind(raw_sock,(struct sockaddr *)&sin,sizeof(sin))) {
		printf("cannot bind %s %d\n",__FILE__,__LINE__);
		return NULL;
	}
	return raw_sock;
}
/*
 * This is a wrapper function for UDP socket creation.
 * Paramters: IP address & port to bind
 * Returns: a pointer to socket structure (handle)
 * or NULL if failed
 *
 */
void *create_udp_socket(const char *ip_addr,unsigned short port)
{
	struct sockaddr_in sin;
	struct timeval tv;
	struct socket *udp_sock = NULL;
	if(sock_create_kern(AF_INET,SOCK_DGRAM,0,&udp_sock)) {
		printf("cannot create socket %s %d\n",__FILE__,__LINE__);
		return NULL;
	}

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(ip_addr);
	sin.sin_port = htons(port);
	if(kernel_bind(udp_sock,(struct sockaddr *)&sin,sizeof(sin))) {
		printf("cannot bind %s %d\n",__FILE__,__LINE__);
		return NULL;
	}
	if(udp_sock->sk) {
//		app_glue_sock_readable(udp_sock->sk,0);
//		app_glue_sock_write_space(udp_sock->sk);
	}
	return udp_sock;
}
/*
 * This is a wrapper function for TCP connecting socket creation.
 * Paramters: IP address & port to bind, IP address & port to connect
 * Returns: a pointer to socket structure (handle)
 * or NULL if failed
 *
 */
void *create_client_socket(const char *my_ip_addr,unsigned short my_port,
		                   const char *peer_ip_addr,unsigned short port)
{
	struct sockaddr_in sin;
	struct timeval tv;
	struct socket *client_sock = NULL;
	if(sock_create_kern(AF_INET,SOCK_STREAM,0,&client_sock)) {
		printf("cannot create socket %s %d\n",__FILE__,__LINE__);
		return NULL;
	}
	tv.tv_sec = -1;
	tv.tv_usec = 0;
	if(sock_setsockopt(client_sock,SOL_SOCKET,SO_RCVTIMEO,(char *)&tv,sizeof(tv))) {
		printf("%s %d cannot set notimeout option\n",__FILE__,__LINE__);
	}
	tv.tv_sec = -1;
	tv.tv_usec = 0;
	if(sock_setsockopt(client_sock,SOL_SOCKET,SO_SNDTIMEO,(char *)&tv,sizeof(tv))) {
		printf("%s %d cannot set notimeout option\n",__FILE__,__LINE__);
	}
	while(1) {
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = inet_addr(my_ip_addr);
		if(my_port) {
			sin.sin_port = htons(my_port);
		}
		else {
			sin.sin_port = htons(rand() & 0xffff);
		}
		if(kernel_bind(client_sock,(struct sockaddr *)&sin,sizeof(sin))) {
			printf("cannot bind %s %d\n",__FILE__,__LINE__);
			if(my_port) {
				break;
			}
			continue;
		}
		break;
	}
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(peer_ip_addr);
	sin.sin_port = htons(port);
	if(client_sock->sk) {
		client_sock->sk->sk_state_change = app_glue_sock_wakeup;
	}
	kernel_connect(client_sock, (struct sockaddr *)&sin,sizeof(sin), 0);

	return client_sock;
}
/*
 * This is a wrapper function for TCP listening socket creation.
 * Paramters: IP address & port to bind
 * Returns: a pointer to socket structure (handle)
 * or NULL if failed
 *
 */
void *create_server_socket(const char *my_ip_addr,unsigned short port)
{
	struct sockaddr_in sin;
	struct timeval tv;
	struct socket *server_sock = NULL;
	uint32_t bufsize;

	if(sock_create_kern(AF_INET,SOCK_STREAM,0,&server_sock)) {
		printf("cannot create socket %s %d\n",__FILE__,__LINE__);
		return NULL;
	}
	tv.tv_sec = -1;
	tv.tv_usec = 0;
	if(sock_setsockopt(server_sock,SOL_SOCKET,SO_RCVTIMEO,(char *)&tv,sizeof(tv))) {
		printf("%s %d cannot set notimeout option\n",__FILE__,__LINE__);
	}
	tv.tv_sec = -1;
	tv.tv_usec = 0;
	if(sock_setsockopt(server_sock,SOL_SOCKET,SO_SNDTIMEO,(char *)&tv,sizeof(tv))) {
		printf("%s %d cannot set notimeout option\n",__FILE__,__LINE__);
	}
	bufsize = 0x4000;
	if(sock_setsockopt(server_sock,SOL_SOCKET,SO_SNDBUF,(char *)&bufsize,sizeof(bufsize))) {
		printf("%s %d cannot set bufsize\n",__FILE__,__LINE__);
	}
	if(sock_setsockopt(server_sock,SOL_SOCKET,SO_RCVBUF,(char *)&bufsize,sizeof(bufsize))) {
		printf("%s %d cannot set bufsize\n",__FILE__,__LINE__);
	}
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(my_ip_addr);
	sin.sin_port = htons(port);

	if(kernel_bind(server_sock,(struct sockaddr *)&sin,sizeof(sin))) {
		printf("cannot bind %s %d\n",__FILE__,__LINE__);
		return NULL;
	}
	if(server_sock->sk) {
		server_sock->sk->sk_state_change = app_glue_sock_wakeup;
	}
	else {
		printf("FATAL %s %d\n",__FILE__,__LINE__);exit(0);
	}
	if(kernel_listen(server_sock,1000)) {
		printf("cannot listen %s %d\n",__FILE__,__LINE__);
		return NULL;
	}
	return server_sock;
}
/*
 * This function polls the driver for the received packets.Called from app_glue_periodic
 * Paramters: ethernet port number.
 * Returns: None
 *
 */
static void app_glue_poll(int port_num)
{
	struct net_device *netdev = (struct net_device *)get_dpdk_dev_by_port_num(port_num);

	if(!netdev) {
		printf("Cannot get netdev %s %d\n",__FILE__,__LINE__);
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
	printf("%s %d %p\n",__FILE__,__LINE__,&accept_ready_socket_list_head[rte_lcore_id()]);
	TAILQ_INIT(&read_ready_socket_list_head[rte_lcore_id()]);
	TAILQ_INIT(&write_ready_socket_list_head[rte_lcore_id()]);
	TAILQ_INIT(&accept_ready_socket_list_head[rte_lcore_id()]);
	TAILQ_INIT(&closed_socket_list_head[rte_lcore_id()]);
	app_glue_drv_last_poll_ts[rte_lcore_id()] = 0;
	app_glue_timer_last_poll_ts[rte_lcore_id()] = 0;
	app_glue_tx_ready_sockets_last_poll_ts[rte_lcore_id()] = 0;
	app_glue_rx_ready_sockets_last_poll_ts[rte_lcore_id()] = 0;
	app_glue_periodic_called[rte_lcore_id()] = 0;
	app_glue_tx_queues_process[rte_lcore_id()] = 0;
	app_glue_rx_queues_process[rte_lcore_id()] = 0;
	working_cycles_stat[rte_lcore_id()] = 0;
	total_cycles_stat[rte_lcore_id()] = 0;
	total_prev[rte_lcore_id()] = 0;
	work_prev[rte_lcore_id()] = 0;
}
/*
 * This function walks on closable, acceptable and readable lists and calls.
 * the application's (user's) function. Called from app_glue_periodic
 * Paramters: None
 * Returns: None
 *
 */
static void process_rx_ready_sockets()
{
	struct socket *sock;

	while(!TAILQ_EMPTY(&closed_socket_list_head[rte_lcore_id()])) {
		sock = TAILQ_FIRST(&closed_socket_list_head[rte_lcore_id()]);
		user_on_socket_fatal(sock);
		sock->closed_queue_present = 0;
		TAILQ_REMOVE(&closed_socket_list_head[rte_lcore_id()],sock,closed_queue_entry);
		kernel_close(sock);
	}
	while(!TAILQ_EMPTY(&accept_ready_socket_list_head[rte_lcore_id()])) {

		sock = TAILQ_FIRST(&accept_ready_socket_list_head[rte_lcore_id()]);
//		printf("%s %d %d\n",__FILE__,__LINE__,rte_lcore_id());
		user_on_accept(sock);
		sock->accept_queue_present = 0;
		TAILQ_REMOVE(&accept_ready_socket_list_head[rte_lcore_id()],sock,accept_queue_entry);
	}

	while(!TAILQ_EMPTY(&read_ready_socket_list_head[rte_lcore_id()])) {
		sock = TAILQ_FIRST(&read_ready_socket_list_head[rte_lcore_id()]);
		user_data_available_cbk(sock);
		sock->read_queue_present = 0;
		TAILQ_REMOVE(&read_ready_socket_list_head[rte_lcore_id()],sock,read_queue_entry);
	}
}
/*
 * This function walks on writable lists and calls.
 * the application's (user's) function. Called from app_glue_periodic
 * Paramters: None
 * Returns: None
 *
 */
static void process_tx_ready_sockets()
{
	struct socket *sock;

	if(!TAILQ_EMPTY(&write_ready_socket_list_head[rte_lcore_id()])) {
		sock = TAILQ_FIRST(&write_ready_socket_list_head[rte_lcore_id()]);
		TAILQ_REMOVE(&write_ready_socket_list_head[rte_lcore_id()],sock,write_queue_entry);
		if(user_on_transmission_opportunity(sock) > 0) {
		    sock->write_queue_present = 0;
		    set_bit(SOCK_NOSPACE, &sock->flags);
		}
		else {
			TAILQ_INSERT_TAIL(&write_ready_socket_list_head[rte_lcore_id()],sock,write_queue_entry);
			clear_bit(SOCK_NOSPACE, &sock->flags);
		}
	}
}

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
	printf("%s %d %d %d %d %d\n",__func__,__LINE__,
			drv_poll_interval,timer_poll_interval,tx_ready_sockets_poll_interval,
			rx_ready_sockets_poll_interval);
	float cycles_in_micro = rte_get_tsc_hz()/1000000;
	app_glue_drv_poll_interval[rte_lcore_id()] = cycles_in_micro*(float)drv_poll_interval;
	app_glue_timer_poll_interval[rte_lcore_id()] = cycles_in_micro*(float)timer_poll_interval;
	app_glue_tx_ready_sockets_poll_interval[rte_lcore_id()] = cycles_in_micro*(float)tx_ready_sockets_poll_interval;
	app_glue_rx_ready_sockets_poll_interval[rte_lcore_id()] = cycles_in_micro*(float)rx_ready_sockets_poll_interval;
	printf("%s %d %"PRIu64" %"PRIu64" %"PRIu64" %"PRIu64"\n",__func__,__LINE__,
			app_glue_drv_poll_interval[rte_lcore_id()],app_glue_timer_poll_interval[rte_lcore_id()],
			app_glue_tx_ready_sockets_poll_interval[rte_lcore_id()],app_glue_rx_ready_sockets_poll_interval[rte_lcore_id()]);
}

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
void app_glue_periodic(int call_flush_queues,uint8_t *ports_to_poll,int ports_to_poll_count)
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
		printf("PANIC: socket NULL\n");while(1);
	}
	if(sock->sk)
		sock->sk->user_data = data;
	else
		printf("PANIC: socket->sk is NULL\n");while(1);
}
/*
 * This function may be called to get attached to the socket user's data .
 * Paramters: a pointer  to socket (returned, for example, by create_*_socket,)
 * Returns: pointer to data to be attached to the socket
 *
 */
void *app_glue_get_user_data(void *socket)
{
	struct socket *sock = (struct socket *)socket;
	if(!sock) {
		printf("PANIC: socket NULL\n");while(1);
	}
	if(!sock->sk) {
		printf("PANIC: socket->sk NULL\n");while(1);
	}
	return sock->sk->user_data;
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
			user_data = sock->sk->user_data;
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
		    return sock->sk->user_data;
  	    printf("PANIC: socket->sk is NULL\n");
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
		    return sock->sk->user_data;
	    printf("PANIC: socket->sk is NULL\n");
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
	        return sock->sk->user_data;
	    printf("PANIC: socket->sk is NULL\n");
		return NULL;
	}
	return NULL;
}
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
	}
	if(sock->write_queue_present) {
		TAILQ_REMOVE(&write_ready_socket_list_head[rte_lcore_id()],sock,write_queue_entry);
		sock->write_queue_present = 0;
	}
	if(sock->accept_queue_present) {
		TAILQ_REMOVE(&accept_ready_socket_list_head[rte_lcore_id()],sock,accept_queue_entry);
		sock->accept_queue_present = 0;
	}
	if(sock->closed_queue_present) {
		TAILQ_REMOVE(&closed_socket_list_head[rte_lcore_id()],sock,closed_queue_entry);
		sock->closed_queue_present = 0;
	}
	if(sock->sk)
		sock->sk->user_data = NULL;
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
		printf("total %"PRIu64" work %"PRIu64" ratio %f\n",total_cycles_stat[cpu_idx],working_cycles_stat[cpu_idx],ratio);
		printf("app_glue_periodic_called %"PRIu64"\n",app_glue_periodic_called[cpu_idx]);
		printf("app_glue_tx_queues_process %"PRIu64"\n",app_glue_tx_queues_process[cpu_idx]);
		printf("app_glue_rx_queues_process %"PRIu64"\n",app_glue_rx_queues_process[cpu_idx]);
	}
}
