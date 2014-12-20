#include <sys/errno.h>
#include <stdlib.h>
#include "../ipaugenblick_common/ipaugenblick_common.h"
#include <rte_config.h>
#include <rte_common.h>
#include <rte_memcpy.h>
#include <rte_lcore.h>
#include <rte_mempool.h>
#include <rte_cycles.h>
#include <rte_ring.h>
#include <rte_mbuf.h>
#include <rte_byteorder.h>
#include "ipaugenblick_ring_ops.h"
#include "ipaugenblick_api.h"
#if 0
#define offsetof(TYPE, MEMBER) ((size_t)&((TYPE*)0)->MEMBER)
#endif

#define container_of(ptr, type, member)({ \
const typeof(((type*) 0)->member)*__mptr=(ptr); \
(type*)((char*)__mptr-offsetof(type, member));})

#define PKT(d) container_of(d,struct rte_pktmbuf,data)

#define RTE_MBUF(d) container_of(PKT(d),struct rte_mbuf,pkt)

socket_descriptor_t socket_descriptors[IPAUGENBLICK_CONNECTION_POOL_SIZE];
struct rte_ring *free_connections_ring = NULL;
struct rte_mempool *tx_bufs_pool = NULL;
struct rte_ring *rx_bufs_ring = NULL;
struct rte_mempool *free_command_pool = NULL;
struct rte_ring *command_ring = NULL;

/* must be called per process */
int ipaugenblick_app_init(int argc,char **argv)
{
    int i;
    char ringname[1024];

    if(rte_eal_init(argc, argv) < 0) {
        printf("%s %d\n",__FILE__,__LINE__);
	return -1;
    }
    printf("EAL initialized\n");
    free_connections_ring = rte_ring_lookup(FREE_CONNECTIONS_RING);

    if(!free_connections_ring) {
        printf("cannot find free connections ring\n");
        return -1;
    }

    memset(socket_descriptors,0,sizeof(socket_descriptors));
    for(i = 0;i < IPAUGENBLICK_CONNECTION_POOL_SIZE;i++) {
        sprintf(ringname,RX_RING_NAME_BASE"%d",i);
        socket_descriptors[i].rx_ring = rte_ring_lookup(ringname);
        sprintf(ringname,TX_RING_NAME_BASE"%d",i);
        socket_descriptors[i].tx_ring = rte_ring_lookup(ringname);
        rte_ring_enqueue(free_connections_ring,&socket_descriptors[i]);
    }
    tx_bufs_pool = rte_mempool_lookup("mbufs_mempool");
    if(!tx_bufs_pool) {
        printf("cannot find tx bufs pool\n");
        return -1;
    }
    rx_bufs_ring = rte_ring_lookup("rx_mbufs_ring");
    if(!rx_bufs_ring) {
        printf("cannot find rx bufs ring\n");
        return -1;
    }
    free_command_pool = rte_mempool_lookup(FREE_COMMAND_POOL_NAME);
    if(!free_command_pool) {
        printf("cannot find free command pool\n");
        return -1;
    }
    command_ring = rte_ring_lookup(COMMAND_RING_NAME);
    if(!command_ring) {
        printf("cannot find command ring\n");
        return -1;
    }
    return ((tx_bufs_pool == NULL)||(command_ring == NULL)||(free_command_pool == NULL));
}

/* open asynchronous TCP client socket */
int ipaugenblick_open_tcp_client(unsigned int ipaddr,unsigned short port,unsigned int myipaddr,unsigned short myport)
{
    struct _socket_descriptor *descr;
    int idx;
    ipaugenblick_cmd_t *cmd; 

    if(rte_ring_dequeue(free_connections_ring,(void **)&descr)) {
        printf("%s %d\n",__FILE__,__LINE__);
        return -1;
    }

//    idx = ((descr - &socket_descriptors[0])/sizeof(socket_descriptors[0]));
    idx = (int)descr;

    /* allocate a ringset (cmd/tx/rx) here */
    cmd = ipaugenblick_get_free_command_buf();
    if(!cmd) {
        printf("%s %d\n",__FILE__,__LINE__);
        return -2;
    }

    cmd->cmd = IPAUGENBLICK_OPEN_CLIENT_SOCKET_COMMAND;
    cmd->u.open_client_sock.my_ipaddress = myipaddr;
    cmd->u.open_client_sock.my_port = myport;
    cmd->u.open_client_sock.peer_ipaddress = ipaddr;
    cmd->u.open_client_sock.peer_port = port;

    ipaugenblick_enqueue_command_buf(cmd);

    return idx;
}

/* open listener */
int ipaugenblick_open_tcp_server(unsigned int ipaddr,unsigned short port,on_accepted_t on_accepted_callback,void *arg)
{
    struct _socket_descriptor *descr;
    int idx;
    ipaugenblick_cmd_t *cmd;

    if(rte_ring_dequeue(free_connections_ring,(void **)&descr)) {
        return -1;
    }

//    idx = ((descr - &socket_descriptors[0])/sizeof(socket_descriptors[0]));
    idx = (int)descr;

    cmd = ipaugenblick_get_free_command_buf();
    if(!cmd) {
        return -2;
    }

    ipaugenblick_enqueue_command_buf(cmd);

    return idx;
}

/* open UDP socket */
int ipaugenblick_open_udp(unsigned int ipaddr,unsigned short port)
{
    struct _socket_descriptor *descr;
    int idx;
    ipaugenblick_cmd_t *cmd;

    if(rte_ring_dequeue(free_connections_ring,(void **)&descr)) {
        return -1;
    }

//    idx = ((descr - &socket_descriptors[0])/sizeof(socket_descriptors[0]));
    idx = (int)descr;

    cmd = ipaugenblick_get_free_command_buf();
    if(!cmd) {
        return -2;
    }

    ipaugenblick_enqueue_command_buf(cmd);

    return idx;
}

/* close any socket */
void ipaugenblick_close(int sock)
{
    ipaugenblick_cmd_t *cmd;
    cmd = ipaugenblick_get_free_command_buf();
    if(!cmd) {
        return;
    }
    ipaugenblick_enqueue_command_buf(cmd);
    rte_ring_enqueue(free_connections_ring,&socket_descriptors[sock]);
}

/* TCP or connected UDP */
int ipaugenblick_send(int sock,void *buffer,int offset,int length,on_send_complete_t send_complete_callback,void *arg)
{
    struct rte_mbuf *mbuf = RTE_MBUF(buffer);
    mbuf->pkt.data_len = length;
    return ipaugenblick_enqueue_tx_buf(sock,mbuf);
}

/* UDP or RAW */
int ipaugenblick_sendto(int sock,void *buffer,int offset,int length,unsigned int ipaddr,unsigned short port,on_send_complete_t send_complete_callback,void *arg)
{
    struct rte_mbuf *mbuf = RTE_MBUF(buffer);
    return ipaugenblick_enqueue_tx_buf(sock,mbuf);
}

/* TCP */
int ipaugenblick_receive(int sock,void **pbuffer,int *len)
{
    struct rte_mbuf *mbuf = ipaugenblick_dequeue_rx_buf(sock);
    if(!mbuf)
        return -1;
    printf("%s %d received %d\n",__FILE__,__LINE__,mbuf->pkt.data_len);
    *pbuffer = mbuf->pkt.data;
    *len = mbuf->pkt.data_len;
    return 0;
}

/* UDP or RAW */
int ipaugenblick_receive_from(int sock,void **buffer,int *len,unsigned int *ipaddr,unsigned short *port)
{
    struct rte_mbuf *mbuf = ipaugenblick_dequeue_rx_buf(sock);
    if(!mbuf)
        return -1;
    *buffer = mbuf->pkt.data;
    *len = mbuf->pkt.data_len;
    return 0;
}

/* Allocate buffer to use later in *send* APIs */
void *ipaugenblick_get_buffer(int length)
{
    struct rte_mbuf *mbuf;
    if(rte_mempool_get(tx_bufs_pool,(void **)&mbuf)) {
        return NULL;
    }
    return &mbuf->pkt.data;
}

/* release buffer when either send is complete or receive has done with the buffer */
void ipaugenblick_release_tx_buffer(void *buffer)
{
    struct rte_mbuf *mbuf = RTE_MBUF(buffer);

    rte_mempool_put(tx_bufs_pool,(void *)mbuf);
}

void ipaugenblick_release_rx_buffer(void *buffer)
{
    struct rte_mbuf *mbuf = RTE_MBUF(buffer);

    rte_ring_enqueue(rx_bufs_ring,(void *)mbuf);
}

/* heartbeat. should be called as frequent as possible */
void ipaugenblick_poll(void)
{
}

