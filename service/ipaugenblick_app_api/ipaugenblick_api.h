
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

/* TCP or connected UDP */
int ipaugenblick_send(int sock,void *buffer,int offset,int length);

/* UDP or RAW */
int ipaugenblick_sendto(int sock,void *buffer,int offset,int length,unsigned int ipaddr,unsigned short port);

/* TCP */
int ipaugenblick_receive(int sock,void **pbuffer,int *len);

/* UDP or RAW */
int ipaugenblick_receive_from(int sock,void **buffer,int *len,unsigned int *ipaddr,unsigned short *port);

/* Allocate buffer to use later in *send* APIs */
void *ipaugenblick_get_buffer(int length);

/* release buffer when either send is complete or receive has done with the buffer */
void ipaugenblick_release_tx_buffer(void *buffer);

void ipaugenblick_release_rx_buffer(void *buffer);

void ipaugenblick_socket_kick(int sock);

int ipaugenblick_get_connected();

int ipaugenblick_accept();

#endif
