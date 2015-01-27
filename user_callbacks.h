/*
 * user_callbacks.h
 *
 *  Created on: Jul 6, 2014
 *      Author: Vadim Suraev vadim.suraev@gmail.com
 *  Contains function prototypes for building applications
 *  on the top of Linux TCP/IP ported to userland and integrated with DPDK 1.6
 *  These functions have to be implemented by the application(s)
 */

#ifndef __USER_CALLBACKS_H_
#define __USER_CALLBACKS_H_

inline void user_on_socket_fatal(struct socket *sock);

inline int user_on_transmission_opportunity(struct socket *sock);

inline int user_data_available_cbk(struct socket *sock);

inline void user_accept_pending_cbk(struct socket *sock);

extern void print_user_stats();

#endif /* API_H_ */
