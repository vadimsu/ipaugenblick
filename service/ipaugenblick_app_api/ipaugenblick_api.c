#include <sys/errno.h>
#include <sys/queue.h>
#include <stdlib.h>
#include "../ipaugenblick_memory_common/ipaugenblick_memory_layout.h"
#include "../ipaugenblick_memory_common/ipaugenblick_memory_common.h"

#include "ipaugenblick_api.h"

#define MB_1 (1024*1024)
#define MB_8 (8*MB_1)

typedef struct _socket_descriptor
{
    int ringset_idx;
    union {
        on_send_complete_t on_send_complete;
        on_accepted_t on_accepted;
    }u;
    TAILQ_ENTRY(_socket_descriptor) entry;
}socket_descriptor_t;

socket_descriptor_t socket_descriptors[IPAUGENBLICK_MAX_SOCKETS];
static TAILQ_HEAD(FreeDescr,_socket_descriptor) free_descriptors_list;

static struct ipaugenblick_memory *memory_base_address = NULL;

/* must be called per process */
void *ipaugenblick_app_init(int *memory_size)
{
    int i;

    TAILQ_INIT(&free_descriptors_list);

    memset(socket_descriptors,0,sizeof(socket_descriptors));
    for(i = 0;i < IPAUGENBLICK_MAX_SOCKETS;i++) {
        TAILQ_INSERT_TAIL(&free_descriptors_list,&socket_descriptors[i],entry);
    }
    *memory_size = MB_8;
    memory_base_address = ipaugenblick_shared_memory_init(*memory_size);
    return memory_base_address;
}

/* open asynchronous TCP client socket */
int ipaugenblick_open_tcp_client(unsigned int ipaddr,unsigned short port)
{
    struct _socket_descriptor *descr;
    int idx;

    if(TAILQ_EMPTY(&free_descriptors_list)) {
        return -1;
    }
    descr = TAILQ_FIRST(&free_descriptors_list);
    TAILQ_REMOVE(&free_descriptors_list,descr,entry);

    idx = ((descr - &socket_descriptors[0])/sizeof(socket_descriptors[0]));

    /* allocate a ringset (cmd/tx/rx) here */

    return idx;
}

/* open listener */
int ipaugenblick_open_tcp_server(unsigned int ipaddr,unsigned short port,on_accepted_t on_accepted_callback,void *arg)
{
    struct _socket_descriptor *descr;
    int idx;

    if(TAILQ_EMPTY(&free_descriptors_list)) {
        return -1;
    }
    descr = TAILQ_FIRST(&free_descriptors_list);
    TAILQ_REMOVE(&free_descriptors_list,descr,entry);

    idx = ((descr - &socket_descriptors[0])/sizeof(socket_descriptors[0]));

    return idx;
}

/* open UDP socket */
int ipaugenblick_open_udp(unsigned int ipaddr,unsigned short port)
{
    struct _socket_descriptor *descr;
    int idx;

    if(TAILQ_EMPTY(&free_descriptors_list)) {
        return -1;
    }
    descr = TAILQ_FIRST(&free_descriptors_list);
    TAILQ_REMOVE(&free_descriptors_list,descr,entry);

    idx = ((descr - &socket_descriptors[0])/sizeof(socket_descriptors[0]));

    /* allocate a ringset (cmd/tx/rx) here */

    return idx;
}

/* close any socket */
void ipaugenblick_close(int sock)
{
    TAILQ_INSERT_TAIL(&free_descriptors_list,&socket_descriptors[sock],entry);
}

/* TCP or connected UDP */
int ipaugenblick_send(int sock,void *buffer,int offset,int length,on_send_complete_t send_complete_callback,void *arg)
{
    return -1;
}

/* UDP or RAW */
int ipaugenblick_sendto(int sock,void *buffer,int offset,int length,unsigned int ipaddr,unsigned short port,on_send_complete_t send_complete_callback,void *arg)
{
    return -1;
}

/* TCP */
int ipaugenblick_receive(int sock,void **pbuffer,int *len)
{
    return -1;
}

/* UDP or RAW */
int ipaugenblick_receive_from(int sock,void **buffer,int *len,unsigned int *ipaddr,unsigned short *port)
{
    return -1;
}

/* Allocate buffer to use later in *send* APIs */
void *ipaugenblick_get_buffer(int length)
{
    return NULL;
}

/* release buffer when either send is complete or receive has done with the buffer */
void ipaugenblick_release_buffer(void *buffer)
{
}

/* heartbeat. should be called as frequent as possible */
void ipaugenblick_poll(void)
{
}

