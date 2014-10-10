/*
 * api.h
 *
 *  Created on: Jul 6, 2014
 *      Author: Vadim Suraev vadim.suraev@gmail.com
 *  Contains function prototypes for building applications
 *  on the top of Linux TCP/IP ported to userland and integrated with DPDK 1.6
 */

#ifndef __API_H_
#define __API_H_
/*
 * This is a wrapper function for RAW socket creation.
 * Paramters: IP address & port (protocol number) to bind
 * Returns: a pointer to socket structure (handle)
 * or NULL if failed
 *
 */
extern void *create_raw_socket(const char *ip_addr,unsigned short port);
/*
 * This is a wrapper function for UDP socket creation.
 * Paramters: IP address & port to bind
 * Returns: a pointer to socket structure (handle)
 * or NULL if failed
 *
 */
extern void *create_udp_socket(const char *ip_addr,unsigned short port);
/*
 * This is a wrapper function for TCP connecting socket creation.
 * Paramters: IP address & port to bind, IP address & port to connect
 * Returns: a pointer to socket structure (handle)
 * or NULL if failed
 *
 */
extern void *create_client_socket(const char *my_ip_addr,unsigned short my_port,
		                          const char *peer_ip_addr,unsigned short port);
/*
 * This is a wrapper function for TCP listening socket creation.
 * Paramters: IP address & port to bind
 * Returns: a pointer to socket structure (handle)
 * or NULL if failed
 *
 */
extern void *create_server_socket(const char *my_ip_addr,unsigned short port);
/*
 * This function must be called by application to initialize.
 * the rate of polling for driver, timer, readable & writable socket lists
 * Paramters: drv_poll_interval,timer_poll_interval,tx_ready_sockets_poll_interval,
 * rx_ready_sockets_poll_interval - all in micros
 * Returns: None
 *
 */
extern void app_glue_init_poll_intervals(int drv_poll_interval,
		int timer_poll_interval,
        int tx_ready_sockets_poll_interval,
        int rx_ready_sockets_poll_interval);
/*
 * This function must be called by application periodically.
 * This is the heart of the system, it performs all the driver/IP stack work
 * and timers
 * Paramters: call_flush_queues - if non-zero, the readable, closable and writable queues
 * are processed and user's functions are called.
 * Alternatively, call_flush_queues can be 0 and the application may call
 * app_glue_get_next* functions to get readable, acceptable, closable and writable sockets
 * ports_to_poll - an array of port numbers to poll
 * ports_to_poll_count - asize of array of ports to poll
 * Returns: None
 *
 */
extern void app_glue_periodic(int call_flush_queues,uint8_t *ports_to_poll,int ports_to_poll_count);
/*
 * This function may be called to attach user's data to the socket.
 * Paramters: a pointer  to socket (returned, for example, by create_*_socket)
 * a pointer to data to be attached to the socket
 * Returns: None
 *
 */
extern void app_glue_set_user_data(void *socket,void *data);
/*
 * This function may be called to get attached to the socket user's data .
 * Paramters: a pointer  to socket (returned, for example, by create_*_socket,)
 * Returns: pointer to data to be attached to the socket
 *
 */
extern void *app_glue_get_user_data(void *socket);
/*
 * This function may be called to get next closable socket .
 * Paramters: None
 * Returns: pointer to socket to be closed
 *
 */
extern void *app_glue_get_next_closed();
/*
 * This function may be called to get next writable socket .
 * Paramters: None
 * Returns: pointer to socket to be written
 *
 */
extern void *app_glue_get_next_writer();
/*
 * This function may be called to get next readable socket .
 * Paramters: None
 * Returns: pointer to socket to be read
 *
 */
extern void *app_glue_get_next_reader();
/*
 * This function may be called to get next acceptable socket .
 * Paramters: None
 * Returns: pointer to socket on which to accept a new connection
 *
 */
extern void *app_glue_get_next_listener();
/*
 * This function may be called to close socket .
 * Paramters: a pointer to socket structure
 * Returns: None
 *
 */
extern void app_glue_close_socket(void *socket);
/*
 * This function may be called to estimate amount of data can be sent .
 * Paramters: a pointer to socket structure
 * Returns: number of bytes the application can send
 *
 */
extern int app_glue_calc_size_of_data_to_send(void *sock);
/*
 * This function must be called prior any other in this package.
 * It initializes all the DPDK libs, reads the configuration, initializes the stack's
 * subsystems, allocates mbuf pools etc.
 * Parameters: refer to DPDK EAL parameters.
 * For example -c <core mask> -n <memory channels> -- -p <port mask>
 */
extern int dpdk_linux_tcpip_init(int argc,char **argv);
/*
 * This function may be called to allocate rte_mbuf from existing pool.
 * Paramters: None
 * Returns: a pointer to rte_mbuf, if succeeded, NULL if failed
 *
 */
extern struct rte_mbuf *app_glue_get_buffer();

/*
 * This function may be called to calculate driver's optimal polling interval .
 * Paramters: a pointer to socket structure
 * Returns: None
 *
 */
extern int get_max_drv_poll_interval_in_micros(int port_num);

/*
 * This callback function is invoked when data arrives to socket.
 * It inserts the socket into a list of readable sockets
 * which is processed in periodic function app_glue_periodic
 * Paramters: a pointer to struct sock, len (dummy)
 * Returns: void
 *
 */
void app_glue_sock_readable(struct sock *sk, int len);

/*
 * This callback function is invoked when data canbe transmitted on socket.
 * It inserts the socket into a list of writable sockets
 * which is processed in periodic function app_glue_periodic
 * Paramters: a pointer to struct sock
 * Returns: void
 *
 */
void app_glue_sock_write_space(struct sock *sk);

/*
 * This callback function is invoked when an error occurs on socket.
 * It inserts the socket into a list of closable sockets
 * which is processed in periodic function app_glue_periodic
 * Paramters: a pointer to struct sock
 * Returns: void
 *
 */
void app_glue_sock_error_report(struct sock *sk);

/*
 * This callback function is invoked when a new connection can be accepted on socket.
 * It looks up the parent (listening) socket for the newly established connection
 * and inserts it into the accept queue
 * which is processed in periodic function app_glue_periodic
 * Paramters: a pointer to struct sock
 * Returns: void
 *
 */
void app_glue_sock_wakeup(struct sock *sk);


#endif /* __API_H_ */
