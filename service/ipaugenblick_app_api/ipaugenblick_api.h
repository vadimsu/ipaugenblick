
#ifndef __IPAUGENBLICK_API_H__
#define __IPAUGENBLICK_API_H__

#define IPAUGENBLICK_MAX_SOCKETS 1000

typedef void (*on_send_complete_t)(void *);
typedef int  (*on_accepted_t)(void *,int,unsigned int *,unsigned short *);

/* must be called per process */
int ipaugenblick_app_init(void);

/* open asynchronous TCP client socket */
int ipaugenblick_open_tcp_client(unsigned int ipaddr,unsigned short port);

/* open listener */
int ipaugenblick_open_tcp_server(unsigned int ipaddr,unsigned short port,on_accepted_t on_accepted_callback,void *arg);

/* open UDP socket */
int ipaugenblick_open_udp(unsigned int ipaddr,unsigned short port);

/* close any socket */
void ipaugenblick_close(int sock);

/* TCP or connected UDP */
int ipaugenblick_send(int sock,void *buffer,int offset,int length,on_send_complete_t send_complete_callback,void *arg);

/* UDP or RAW */
int ipaugenblick_sendto(int sock,void *buffer,int offset,int length,unsigned int ipaddr,unsigned short port,on_send_complete_t send_complete_callback,void *arg);

/* TCP */
int ipaugenblick_receive(int sock,void **pbuffer,int *len);

/* UDP or RAW */
int ipaugenblick_receive_from(int sock,void **buffer,int *len,unsigned int *ipaddr,unsigned short *port);

/* Allocate buffer to use later in *send* APIs */
void *ipaugenblick_get_buffer(int length);

/* release buffer when either send is complete or receive has done with the buffer */
void ipaugenblick_release_buffer(void *buffer);

/* heartbeat. should be called as frequent as possible */
void ipaugenblick_poll(void);

#endif
