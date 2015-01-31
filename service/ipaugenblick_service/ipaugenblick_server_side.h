
#ifndef __IPAUGENBLICK_SERVER_SIDE_H__
#define __IPAUGENBLICK_SERVER_SIDE_H__

#define PKTMBUF_HEADROOM 128
#define IPAUGENBLICK_BUFSIZE (PKTMBUF_HEADROOM+1448)
#define COMMAND_POOL_SIZE 100

struct ipaugenblick_ring_set
{ 
    struct rte_ring *tx_ring;
    struct rte_ring *rx_ring; 
};

typedef struct
{
    union {
        struct {
            int ring_idx;
            int parent_idx;
        }rings_ids;
        unsigned long data;
    }u; 
}sock_and_selector_idx_t;

#define PARENT_IDX(sock_and_selector_idx) sock_and_selector_idx.u.rings_ids.parent_idx
#define RINGSET_IDX(sock_and_selector_idx) sock_and_selector_idx.u.rings_ids.ring_idx

extern struct rte_ring *command_ring;
extern struct rte_ring *selectors_ring;
extern struct rte_ring *free_connections_ring;
extern struct rte_mempool *free_connections_pool;
extern struct rte_mempool *selectors_pool;
extern struct rte_ring *rx_mbufs_ring;
extern struct rte_mempool *free_command_pool;
extern struct ipaugenblick_ring_set ringsets[IPAUGENBLICK_CONNECTION_POOL_SIZE];
extern void *ringidx_to_socket[IPAUGENBLICK_CONNECTION_POOL_SIZE];
extern ipaugenblick_socket_t *g_ipaugenblick_sockets;
extern ipaugenblick_selector_t *g_ipaugenblick_selectors;

#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

static inline struct ipaugenblick_memory *ipaugenblick_service_api_init(int command_bufs_count,
                                                          int rx_bufs_count,
                                                          int tx_bufs_count)
{   
    char ringname[1024];
    int ringset_idx,i; 
    ipaugenblick_socket_t *ipaugenblick_socket;
    ipaugenblick_selector_t *ipaugenblick_selector;

    sprintf(ringname,COMMAND_RING_NAME);

    command_ring = rte_ring_create(ringname, command_bufs_count,rte_socket_id(), RING_F_SC_DEQ);
    if(!command_ring) {
        printf("cannot create ring %s %d\n",__FILE__,__LINE__);
        exit(0);
    }
    printf("COMMAND RING CREATED\n");
    sprintf(ringname,"rx_mbufs_ring");

    rx_mbufs_ring = rte_ring_create(ringname, rx_bufs_count*IPAUGENBLICK_CONNECTION_POOL_SIZE,rte_socket_id(), 0);
    if(!rx_mbufs_ring) {
        printf("cannot create ring %s %d\n",__FILE__,__LINE__);
        exit(0);
    }
    printf("RX RING CREATED\n");
    sprintf(ringname,FREE_COMMAND_POOL_NAME);

    free_command_pool = rte_mempool_create(ringname, COMMAND_POOL_SIZE,
	    			           sizeof(ipaugenblick_cmd_t), 0,
				           0,
				           NULL, NULL,
				           NULL, NULL,
				           rte_socket_id(), 0);
    if(!free_command_pool) {
        printf("cannot create mempool %s %d\n",__FILE__,__LINE__);
        exit(0);
    }
    printf("COMMAND POOL CREATED\n");
    sprintf(ringname,FREE_CONNECTIONS_POOL_NAME);

    free_connections_pool = rte_mempool_create(ringname, 1,
	    	  		               sizeof(ipaugenblick_socket_t)*IPAUGENBLICK_CONNECTION_POOL_SIZE, 0,
				               0,
				               NULL, NULL,
				               NULL, NULL,
				               rte_socket_id(), 0);
    if(!free_connections_pool) {
        printf("cannot create mempool %s %d\n",__FILE__,__LINE__);
        exit(0);
    }

    printf("FREE CONNECTIONS POOL CREATED\n");

    if(rte_mempool_get(free_connections_pool,(void **)&g_ipaugenblick_sockets)) {
        printf("cannot get from mempool %s %d \n",__FILE__,__LINE__);
        exit(0);
    }

    ipaugenblick_socket = g_ipaugenblick_sockets;

    free_connections_ring = rte_ring_create(FREE_CONNECTIONS_RING,IPAUGENBLICK_CONNECTION_POOL_SIZE,rte_socket_id(), 0);
    if(!free_connections_ring) {
        printf("cannot create ring %s %d\n",__FILE__,__LINE__);
        exit(0);
    }

    printf("FREE CONNECTIONS RING CREATED\n");

    for(ringset_idx = 0;ringset_idx < IPAUGENBLICK_CONNECTION_POOL_SIZE;ringset_idx++,ipaugenblick_socket++) { 
          
        memset(ipaugenblick_socket,0,sizeof(ipaugenblick_socket_t));
        ipaugenblick_socket->connection_idx = ringset_idx;
        rte_atomic16_init(&ipaugenblick_socket->is_in_select);
        rte_ring_enqueue(free_connections_ring,(void*)ipaugenblick_socket);
        sprintf(ringname,TX_RING_NAME_BASE"%d",ringset_idx);
        ringsets[ringset_idx].tx_ring = rte_ring_create(ringname, tx_bufs_count,rte_socket_id(), RING_F_SP_ENQ | RING_F_SC_DEQ);
        if(!ringsets[ringset_idx].tx_ring) {
            printf("cannot create ring %s %d\n",__FILE__,__LINE__);
            exit(0);
        }
        sprintf(ringname,RX_RING_NAME_BASE"%d",ringset_idx);
        ringsets[ringset_idx].rx_ring = rte_ring_create(ringname, rx_bufs_count,rte_socket_id(), RING_F_SP_ENQ | RING_F_SC_DEQ);
        if(!ringsets[ringset_idx].rx_ring) {
            printf("cannot create ring %s %d\n",__FILE__,__LINE__);
            exit(0);
        }
    }
    printf("CONNECTIONS Tx/Rx RINGS CREATED\n");
    memset(ringidx_to_socket,0,sizeof(void *)*IPAUGENBLICK_CONNECTION_POOL_SIZE);
    sprintf(ringname,SELECTOR_POOL_NAME);

    selectors_pool = rte_mempool_create(ringname, 1,
	     		                sizeof(ipaugenblick_selector_t)*IPAUGENBLICK_SELECTOR_POOL_SIZE, 0,
				               0,
				               NULL, NULL,
				               NULL, NULL,
				               rte_socket_id(), 0);
    if(!selectors_pool) {
        printf("cannot create mempool %s %d\n",__FILE__,__LINE__);
        exit(0);
    }
    printf("SELECTOR POOL CREATED\n");
    if(rte_mempool_get(selectors_pool,(void **)&g_ipaugenblick_selectors)) {
        printf("cannot get from mempool %s %d \n",__FILE__,__LINE__);
        exit(0);
    }
    selectors_ring = rte_ring_create(SELECTOR_RING_NAME,IPAUGENBLICK_SELECTOR_POOL_SIZE,rte_socket_id(), 0);
    if(!selectors_ring) {
        printf("cannot create ring %s %d \n",__FILE__,__LINE__);
        exit(0);
    }
    printf("SELECTOR RING CREATED\n");
    ipaugenblick_selector = g_ipaugenblick_selectors;
    for(ringset_idx = 0;ringset_idx < IPAUGENBLICK_SELECTOR_POOL_SIZE;ringset_idx++) {
        sprintf(ringname,"SELECTOR_RING_NAME%d",ringset_idx);
        ipaugenblick_selector[ringset_idx].ringset_idx = ringset_idx;
        ipaugenblick_selector[ringset_idx].ready_connections = rte_ring_create(ringname, tx_bufs_count,rte_socket_id(), RING_F_SP_ENQ | RING_F_SC_DEQ);
        if(!ipaugenblick_selector[ringset_idx].ready_connections) {
            printf("cannot create ring %s %d\n",__FILE__,__LINE__);
            exit(0);
        }
        printf("SELECTOR READY RING#%d CREATED\n",ringset_idx);
        rte_ring_enqueue(selectors_ring,(void*)ringset_idx);
    }
    printf("DONE\n");
    return 0;
}
static inline ipaugenblick_cmd_t *ipaugenblick_dequeue_command_buf()
{
    ipaugenblick_cmd_t *cmd;
    if(!rte_ring_count(command_ring))
        return NULL;
    if(rte_ring_sc_dequeue_bulk(command_ring,(void **)&cmd,1)) {
        return NULL;
    }
    return cmd;
}

static inline void ipaugenblick_free_command_buf(ipaugenblick_cmd_t *cmd)
{
    rte_mempool_put(free_command_pool,(void *)cmd);
}

static inline ipaugenblick_cmd_t *ipaugenblick_get_free_command_buf()
{
    ipaugenblick_cmd_t *cmd;
    if(rte_mempool_get(free_command_pool,(void **)&cmd))
        return NULL;
    return cmd;
}

static inline void ipaugenblick_post_accepted(ipaugenblick_cmd_t *cmd)
{
    if(rte_ring_enqueue(ringsets[cmd->ringset_idx].rx_ring,(void *)cmd)) { 
        ipaugenblick_free_command_buf(cmd);
    }
}

static inline struct rte_mbuf *ipaugenblick_dequeue_tx_buf(int ringset_idx)
{
    struct rte_mbuf *mbuf;
    if(rte_ring_sc_dequeue_bulk(ringsets[ringset_idx].tx_ring,(void **)&mbuf,1)) { 
        return NULL;
    }
    return mbuf;
}

static inline int ipaugenblick_dequeue_tx_buf_burst(int ringset_idx,struct rte_mbuf **mbufs,int max_count)
{
    return rte_ring_sc_dequeue_burst(ringsets[ringset_idx].tx_ring,(void **)mbufs,max_count);
}

static inline int ipaugenblick_tx_buf_count(int ringset_idx)
{
    return rte_ring_count(ringsets[ringset_idx].tx_ring);
}

static inline int ipaugenblick_rx_buf_free_count(int ringset_idx)
{
    return rte_ring_free_count(ringsets[ringset_idx].rx_ring);
}

static inline int ipaugenblick_submit_rx_buf(struct rte_mbuf *mbuf,int ringset_idx)
{
    return rte_ring_sp_enqueue_bulk(ringsets[ringset_idx].rx_ring,(void *)&mbuf,1);
}

#endif
