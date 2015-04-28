
#ifndef __IPAUGENBLICK_API_H__
#define __IPAUGENBLICK_API_H__

#define IPAUGENBLICK_MAX_SOCKETS 1000

typedef void (*ipaugenblick_update_cbk_t)(unsigned char command,unsigned char *buffer,int len);

/* must be called per process */
/* it is a responsibility of the caller to provide app_unique_id */
/* note that if an application may close and restart, it is highly desirable
   to provide the same unique id (process id is not good for this purpose since
   each time it will create a different set of rings so the memory will leak) */
extern int ipaugenblick_app_init(int argc, char **argv,char *app_unique_id);

extern int ipaugenblick_create_client(ipaugenblick_update_cbk_t update_cbk);

extern int ipaugenblick_read_updates(void);

extern int ipaugenblick_open_socket(int family,int type,int parent);

extern int ipaugenblick_v4_connect_bind_socket(int sock,unsigned int ipaddr,unsigned short port,int is_connect);

extern int ipaugenblick_listen_socket(int sock);

/* close any socket */
extern void ipaugenblick_close(int sock);

/* how many buffers can be submitted */
extern int ipaugenblick_get_socket_tx_space(int sock);

/* TCP or connected UDP */
extern int ipaugenblick_send(int sock,void *buffer,int offset,int length);

extern int ipaugenblick_send_bulk(int sock,void **buffers,int *offsets,int *lengths,int buffer_count);

/* UDP or RAW */
extern int ipaugenblick_sendto(int sock,void *buffer,int offset,int length,unsigned int ipaddr,unsigned short port);
extern int ipaugenblick_sendto_bulk(int sock,void **buffers,int *offsets,int *lengths,unsigned int *ipaddrs,unsigned short *ports,int buffer_count);

/* TCP */
extern int ipaugenblick_receive(int sock,void **pbuffer,int *len,int *first_segment_len);

/* UDP or RAW */
extern int ipaugenblick_receivefrom(int sock,void **buffer,int *len,unsigned int *ipaddr,unsigned short *port);

/* Allocate buffer to use later in *send* APIs */
extern void *ipaugenblick_get_buffer(int length,int owner_sock);

extern int ipaugenblick_get_buffers_bulk(int length,int owner_sock,int count,void **bufs);

/* release buffer when either send is complete or receive has done with the buffer */
extern void ipaugenblick_release_tx_buffer(void *buffer);

extern void ipaugenblick_release_rx_buffer(void *buffer,int sock);

extern int ipaugenblick_socket_kick(int sock);

extern int ipaugenblick_accept(int sock,unsigned int *ipaddr,unsigned short *port);

extern int ipaugenblick_open_select(void);

extern int ipaugenblick_set_socket_select(int sock,int select);

extern int ipaugenblick_is_connected(int sock);

extern int ipaugenblick_select(int selector,unsigned short *mask,void* timeout);

extern int ipaugenblick_socket_connect(int sock,unsigned int ipaddr,unsigned short port);

/* receive functions return a chained buffer. this function
   retrieves a next chunk and its length */
extern void *ipaugenblick_get_next_buffer_segment(void *buffer,int *len);

extern int ipaugenblick_add_v4_route(unsigned int ipaddr,unsigned int mask,unsigned int nexthop,short metric);
extern int ipaugenblick_del_v4_route(unsigned int ipaddr,unsigned int mask,unsigned int nexthop);

extern void ipaugenblick_getsockname(int sock, int is_local,unsigned int *ipaddr,unsigned short *port);

extern int ipaugenblick_setsockopt(int sock, int level, int optname,char *optval, unsigned int optlen);

#endif
