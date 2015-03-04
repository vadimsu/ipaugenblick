
#ifndef __IPAUGENBLICK_API_H__
#define __IPAUGENBLICK_API_H__

#define IPAUGENBLICK_MAX_SOCKETS 1000

/* must be called per process */
extern int ipaugenblick_app_init(int argc, char **argv);

/* open asynchronous TCP client socket */
int ipaugenblick_open_tcp_client(unsigned int ipaddr,unsigned short port,unsigned int myipaddr,unsigned short myport);

/* open listener */
int ipaugenblick_open_tcp_server(unsigned int ipaddr,unsigned short port);

/* open UDP socket */
int ipaugenblick_open_udp(unsigned int ipaddr,unsigned short port);

/* close any socket */
void ipaugenblick_close(int sock);

/* how many buffers can be submitted */
int ipaugenblick_get_socket_tx_space(int sock);

/* TCP or connected UDP */
int ipaugenblick_send(int sock,void *buffer,int offset,int length);

inline int ipaugenblick_send_bulk(int sock,void **buffers,int *offsets,int *lengths,int buffer_count);

/* UDP or RAW */
int ipaugenblick_sendto(int sock,void *buffer,int offset,int length,unsigned int ipaddr,unsigned short port);
inline int ipaugenblick_sendto_bulk(int sock,void **buffers,int *offsets,int *lengths,unsigned int *ipaddrs,unsigned short *ports,int buffer_count);

/* TCP */
int ipaugenblick_receive(int sock,void **pbuffer,int *len,int *nb_segs);

/* UDP or RAW */
int ipaugenblick_receivefrom(int sock,void **buffer,int *len,int *nb_segs,unsigned int *ipaddr,unsigned short *port);

/* Allocate buffer to use later in *send* APIs */
void *ipaugenblick_get_buffer(int length,int owner_sock);

int ipaugenblick_get_buffers_bulk(int length,int owner_sock,int count,void **bufs);

/* release buffer when either send is complete or receive has done with the buffer */
void ipaugenblick_release_tx_buffer(void *buffer);

void ipaugenblick_release_rx_buffer(void *buffer);

int ipaugenblick_socket_kick(int sock);

int ipaugenblick_accept(int sock);

int ipaugenblick_open_select(void);

int ipaugenblick_set_socket_select(int sock,int select);

int ipaugenblick_select(int selector,unsigned short *mask,int timeout);

int ipaugenblick_socket_connect(int sock,unsigned int ipaddr,unsigned short port);

/* receive functions return a chained buffer. this function
   retrieves a next chunk and its length */
void *ipaugenblick_get_next_buffer_segment(void *buffer,int *len);

#endif
