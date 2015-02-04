#include <sys/errno.h>
#include <stdlib.h>
#include <rte_atomic.h>
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
#include <netinet/in.h> 
#if 0
#define offsetof(TYPE, MEMBER) ((size_t)&((TYPE*)0)->MEMBER)
#endif

#define container_of(ptr, type, member)({ \
const typeof(((type*) 0)->member)*__mptr=(ptr); \
(type*)((char*)__mptr-offsetof(type, member));})

#define PKT(d) container_of(d,struct rte_pktmbuf,data)

#define RTE_MBUF(d) container_of(PKT(d),struct rte_mbuf,pkt)

socket_descriptor_t socket_descriptors[IPAUGENBLICK_CONNECTION_POOL_SIZE];
struct rte_mempool *free_connections_pool = NULL;
struct rte_ring *free_connections_ring = NULL;
struct rte_mempool *tx_bufs_pool = NULL;
struct rte_ring *rx_bufs_ring = NULL;
struct rte_mempool *free_command_pool = NULL;
struct rte_ring *command_ring = NULL;
struct rte_ring *selectors_ring = NULL;

typedef struct
{
    struct rte_ring *ready_connections;
}selector_t;

static selector_t selectors[IPAUGENBLICK_CONNECTION_POOL_SIZE];

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

    free_connections_pool = rte_mempool_lookup(FREE_CONNECTIONS_POOL_NAME);

    if(!free_connections_pool) {
        printf("cannot find free connections pool\n");
        return -1;
    }

    memset(socket_descriptors,0,sizeof(socket_descriptors));
    for(i = 0;i < IPAUGENBLICK_CONNECTION_POOL_SIZE;i++) {
        sprintf(ringname,RX_RING_NAME_BASE"%d",i);
        socket_descriptors[i].rx_ring = rte_ring_lookup(ringname);
        if(!socket_descriptors[i].rx_ring) {
            printf("%s %d\n",__FILE__,__LINE__);
            exit(0);
        }
        sprintf(ringname,TX_RING_NAME_BASE"%d",i);
        socket_descriptors[i].tx_ring = rte_ring_lookup(ringname);
        if(!socket_descriptors[i].tx_ring) {
            printf("%s %d\n",__FILE__,__LINE__);
            exit(0);
        }
        socket_descriptors[i].select = -1;
        rte_ring_enqueue(free_connections_ring,&socket_descriptors[i]);
    }
    tx_bufs_pool = rte_mempool_lookup("mbufs_mempool");
    if(!tx_bufs_pool) {
        printf("cannot find tx bufs pool\n");
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
    rx_bufs_ring = rte_ring_lookup("rx_mbufs_ring");
    if(!rx_bufs_ring) {
        printf("cannot find rx bufs ring\n");
        return -1;
    }
    selectors_ring = rte_ring_lookup(SELECTOR_RING_NAME);
    
    for(i = 0;i < IPAUGENBLICK_SELECTOR_POOL_SIZE;i++) {
        sprintf(ringname,"SELECTOR_RING_NAME%d",i);
        selectors[i].ready_connections = rte_ring_lookup(ringname);
        if(!selectors[i].ready_connections) {
            printf("cannot find ring %s %d\n",__FILE__,__LINE__);
            exit(0);
        }
    }
    return ((tx_bufs_pool == NULL)||(command_ring == NULL)||(free_command_pool == NULL));
}

int ipaugenblick_open_select(void)
{
    int ringset_idx;

    if(rte_ring_dequeue(selectors_ring,(void **)&ringset_idx)) {
        printf("%s %d\n",__FILE__,__LINE__);
        return -1;
    }
    return (int)ringset_idx;
}

int ipaugenblick_set_socket_select(int sock,int select)
{
   ipaugenblick_cmd_t *cmd;

    if(sock < 0)
        return -1;

   cmd = ipaugenblick_get_free_command_buf();
   if(!cmd) {
       printf("%s %d\n",__FILE__,__LINE__);
       return -2;
   }

   cmd->cmd = IPAUGENBLICK_SET_SOCKET_SELECT_COMMAND;
   cmd->ringset_idx = sock;
   cmd->u.set_socket_select.socket_select = select;
   ipaugenblick_enqueue_command_buf(cmd);
   socket_descriptors[sock].select = select;
}

static inline void ipaugenblick_free_command_buf(ipaugenblick_cmd_t *cmd)
{
    rte_mempool_put(free_command_pool,(void *)cmd);
}
/* open asynchronous TCP client socket */
int ipaugenblick_open_tcp_client(unsigned int ipaddr,unsigned short port,unsigned int myipaddr,unsigned short myport)
{
    ipaugenblick_socket_t *ipaugenblick_socket;
    ipaugenblick_cmd_t *cmd; 

    if(rte_ring_dequeue(free_connections_ring,(void **)&ipaugenblick_socket)) {
        printf("%s %d\n",__FILE__,__LINE__);
        return -1;
    }

    /* allocate a ringset (cmd/tx/rx) here */
    cmd = ipaugenblick_get_free_command_buf();
    if(!cmd) {
        printf("%s %d\n",__FILE__,__LINE__);
        return -2;
    }

    cmd->cmd = IPAUGENBLICK_OPEN_CLIENT_SOCKET_COMMAND;
    cmd->ringset_idx = ipaugenblick_socket->connection_idx;
    cmd->parent_idx = 0;
    cmd->u.open_client_sock.my_ipaddress = myipaddr;
    cmd->u.open_client_sock.my_port = myport;
    cmd->u.open_client_sock.peer_ipaddress = ipaddr;
    cmd->u.open_client_sock.peer_port = port;

    ipaugenblick_enqueue_command_buf(cmd);

    socket_descriptors[ipaugenblick_socket->connection_idx].socket = ipaugenblick_socket;

    return ipaugenblick_socket->connection_idx;
}

/* open listener */
int ipaugenblick_open_tcp_server(unsigned int ipaddr,unsigned short port)
{
    ipaugenblick_socket_t *ipaugenblick_socket;
    ipaugenblick_cmd_t *cmd;

    if(rte_ring_dequeue(free_connections_ring,(void **)&ipaugenblick_socket)) {
        return -1;
    }

    cmd = ipaugenblick_get_free_command_buf();
    if(!cmd) {
        return -2;
    }

    cmd->cmd = IPAUGENBLICK_OPEN_LISTENING_SOCKET_COMMAND;
    cmd->ringset_idx = ipaugenblick_socket->connection_idx;
    cmd->parent_idx = 0;
    cmd->u.open_listening_sock.ipaddress = ipaddr;
    cmd->u.open_listening_sock.port = port;

    ipaugenblick_enqueue_command_buf(cmd);

    socket_descriptors[ipaugenblick_socket->connection_idx].socket = ipaugenblick_socket;

    return ipaugenblick_socket->connection_idx;
}

/* open UDP socket */
int ipaugenblick_open_udp(unsigned int ipaddr,unsigned short port)
{
    ipaugenblick_socket_t *ipaugenblick_socket;
    ipaugenblick_cmd_t *cmd;

    if(rte_ring_dequeue(free_connections_ring,(void **)&ipaugenblick_socket)) {
        return -1;
    }

    cmd = ipaugenblick_get_free_command_buf();
    if(!cmd) {
        return -2;
    }

    cmd->cmd = IPAUGENBLICK_OPEN_UDP_SOCKET_COMMAND;
    cmd->ringset_idx = ipaugenblick_socket->connection_idx;
    cmd->parent_idx = 0;
    cmd->u.open_listening_sock.ipaddress = ipaddr;
    cmd->u.open_listening_sock.port = port;

    ipaugenblick_enqueue_command_buf(cmd);

    socket_descriptors[ipaugenblick_socket->connection_idx].socket = ipaugenblick_socket;

    return ipaugenblick_socket->connection_idx;
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
inline int ipaugenblick_send(int sock,void *buffer,int offset,int length)
{
    struct rte_mbuf *mbuf = RTE_MBUF(buffer);
    mbuf->pkt.data_len = length;
    return ipaugenblick_enqueue_tx_buf(sock,mbuf);
}

/* UDP or RAW */
inline int ipaugenblick_sendto(int sock,void *buffer,int offset,int length,unsigned int ipaddr,unsigned short port)
{
    struct rte_mbuf *mbuf = RTE_MBUF(buffer);
    char *p_addr = mbuf->pkt.data;
    struct sockaddr_in *p_addr_in;
    mbuf->pkt.data_len = length;
    p_addr -= sizeof(struct sockaddr_in);
    p_addr_in = (struct sockaddr_in *)p_addr;
    p_addr_in->sin_family = AF_INET;
    p_addr_in->sin_port = port;
    p_addr_in->sin_addr.s_addr = ipaddr;
    return ipaugenblick_enqueue_tx_buf(sock,mbuf);
}

/* TCP */
inline int ipaugenblick_receive(int sock,void **pbuffer,int *len)
{
    struct rte_mbuf *mbuf = ipaugenblick_dequeue_rx_buf(sock);
    if(!mbuf)
        return -1;
    *pbuffer = &(mbuf->pkt.data);
    *len = mbuf->pkt.data_len;
    return 0;
}

/* UDP or RAW */
inline int ipaugenblick_receivefrom(int sock,void **buffer,int *len,unsigned int *ipaddr,unsigned short *port)
{
    struct rte_mbuf *mbuf = ipaugenblick_dequeue_rx_buf(sock);
    if(!mbuf)
        return -1;
    *buffer = &(mbuf->pkt.data);
    *len = mbuf->pkt.data_len;
    char *p_addr = mbuf->pkt.data;
    p_addr -= sizeof(struct sockaddr_in);
    struct sockaddr_in *p_addr_in = (struct sockaddr_in *)p_addr;
    *port = p_addr_in->sin_port;
    *ipaddr = p_addr_in->sin_addr.s_addr;
    return 0;
}

/* Allocate buffer to use later in *send* APIs */
inline void *ipaugenblick_get_buffer(int length)
{
    struct rte_mbuf *mbuf;
    mbuf = rte_pktmbuf_alloc(tx_bufs_pool);
    if(!mbuf) {
        return NULL;
    }
    return &(mbuf->pkt.data);
}

/* release buffer when either send is complete or receive has done with the buffer */
void ipaugenblick_release_tx_buffer(void *buffer)
{
    struct rte_mbuf *mbuf = RTE_MBUF(buffer);

    rte_pktmbuf_free_seg(mbuf);
}

inline void ipaugenblick_release_rx_buffer(void *buffer)
{
    struct rte_mbuf *mbuf = RTE_MBUF(buffer);
    rte_pktmbuf_free(mbuf); 
}

void ipaugenblick_socket_kick(int sock)
{
    ipaugenblick_cmd_t *cmd;
    cmd = ipaugenblick_get_free_command_buf();
    if(!cmd) {
        return;
    }
    cmd->cmd = IPAUGENBLICK_SOCKET_KICK_COMMAND;
    cmd->ringset_idx = sock;
    ipaugenblick_enqueue_command_buf(cmd);
}

int ipaugenblick_accept(int sock)
{
    ipaugenblick_cmd_t *cmd;
    ipaugenblick_socket_t *ipaugenblick_socket;

    if(rte_ring_dequeue(socket_descriptors[sock].rx_ring,(void **)&cmd)) {
        return -1;
    }
    
    if(rte_ring_dequeue(free_connections_ring,(void **)&ipaugenblick_socket)) {
        ipaugenblick_free_command_buf(cmd);
        return -1;
    } 
    socket_descriptors[ipaugenblick_socket->connection_idx].sock = cmd->u.accepted_socket.socket_descr;
    cmd->cmd = IPAUGENBLICK_SET_SOCKET_RING_COMMAND;
    cmd->ringset_idx = ipaugenblick_socket->connection_idx;
    cmd->u.set_socket_ring.socket_descr = socket_descriptors[ipaugenblick_socket->connection_idx].sock;
    ipaugenblick_enqueue_command_buf(cmd); 
    return ipaugenblick_socket->connection_idx;
}

int ipaugenblick_select(int selector,unsigned short *mask)
{
    uint32_t ringset_idx_and_ready_mask;

    if(rte_ring_dequeue(selectors[selector].ready_connections,(void **)&ringset_idx_and_ready_mask)) {
        return -1;
    }
    *mask = ringset_idx_and_ready_mask >> SOCKET_READY_SHIFT;
    if((*mask) & SOCKET_READABLE_BIT)
        rte_atomic16_set(&(socket_descriptors[ringset_idx_and_ready_mask & SOCKET_READY_MASK].socket->read_ready),0);
    if((*mask) & SOCKET_WRITABLE_BIT)
        rte_atomic16_set(&(socket_descriptors[ringset_idx_and_ready_mask & SOCKET_READY_MASK].socket->write_ready),0);
    return ringset_idx_and_ready_mask & SOCKET_READY_MASK;
}

int ipaugenblick_socket_connect(int sock,unsigned int ipaddr,unsigned short port)
{
    ipaugenblick_cmd_t *cmd;
    cmd = ipaugenblick_get_free_command_buf();
    if(!cmd) {
        return -1;
    }
    cmd->cmd = IPAUGENBLICK_SOCKET_CONNECT_COMMAND;
    cmd->u.socket_connect.ipaddr = ipaddr;
    cmd->u.socket_connect.port = port;
    cmd->ringset_idx = sock;
    ipaugenblick_enqueue_command_buf(cmd);
    return 0;
}
