
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

extern struct rte_ring *command_ring;
extern struct rte_ring *accepted_ring;
extern struct rte_ring *feedbacks_ring;
extern struct rte_ring *free_connections_ring;
extern struct rte_ring *rx_mbufs_ring;
extern struct rte_mempool *free_command_pool;
extern struct rte_mempool *free_accepted_pool;
extern struct ipaugenblick_ring_set ringsets[IPAUGENBLICK_CONNECTION_POOL_SIZE];

static inline struct ipaugenblick_memory *ipaugenblick_service_api_init(int command_bufs_count,
                                                          int rx_bufs_count,
                                                          int tx_bufs_count)
{   
    char ringname[1024];
    int ringset_idx,i;

    sprintf(ringname,COMMAND_RING_NAME);

    command_ring = rte_ring_create(ringname, command_bufs_count,rte_socket_id(), 0);

    sprintf(ringname,ACCEPTED_RING_NAME);

    accepted_ring = rte_ring_create(ringname, command_bufs_count,rte_socket_id(), 0); 

    sprintf(ringname,"rx_mbufs_ring");

    rx_mbufs_ring = rte_ring_create(ringname, rx_bufs_count*IPAUGENBLICK_CONNECTION_POOL_SIZE,rte_socket_id(), 0);

    sprintf(ringname,FREE_ACCEPTED_POOL_NAME);

    free_accepted_pool = rte_mempool_create(ringname, COMMAND_POOL_SIZE,
	    			           sizeof(ipaugenblick_cmd_t), 32,
				           0,
				           NULL, NULL,
				           NULL, NULL,
				           rte_socket_id(), 0);

    sprintf(ringname,FREE_COMMAND_POOL_NAME);

    free_command_pool = rte_mempool_create(ringname, COMMAND_POOL_SIZE,
	    			           sizeof(ipaugenblick_cmd_t), 32,
				           0,
				           NULL, NULL,
				           NULL, NULL,
				           rte_socket_id(), 0);

    free_connections_ring = rte_ring_create(FREE_CONNECTIONS_RING,IPAUGENBLICK_CONNECTION_POOL_SIZE,rte_socket_id(), 0);

    sprintf(ringname,FEEDBACKS_RING_NAME_BASE"%d",ringset_idx);
    feedbacks_ring = rte_ring_create(ringname, 1024,rte_socket_id(), 0);
    printf("%s %d %p\n",__FILE__,__LINE__,feedbacks_ring);

    for(ringset_idx = 0;ringset_idx < IPAUGENBLICK_CONNECTION_POOL_SIZE;ringset_idx++) { 
        rte_ring_enqueue(free_connections_ring,(void*)ringset_idx);
        sprintf(ringname,TX_RING_NAME_BASE"%d",ringset_idx);
        ringsets[ringset_idx].tx_ring = rte_ring_create(ringname, tx_bufs_count,rte_socket_id(), 0);
        printf("%s %d %p\n",__FILE__,__LINE__,ringsets[ringset_idx].tx_ring);
        sprintf(ringname,RX_RING_NAME_BASE"%d",ringset_idx);
        ringsets[ringset_idx].rx_ring = rte_ring_create(ringname, rx_bufs_count,rte_socket_id(), 0);
        printf("%s %d %p\n",__FILE__,__LINE__,ringsets[ringset_idx].rx_ring); 
    }
    return 0;
}
static inline ipaugenblick_cmd_t *ipaugenblick_dequeue_command_buf()
{
    ipaugenblick_cmd_t *cmd;
    if(rte_ring_dequeue(command_ring,(void **)&cmd)) {
        return NULL;
    }
    return cmd;
}

static inline void ipaugenblick_free_command_buf(ipaugenblick_cmd_t *cmd)
{
    rte_mempool_put(free_command_pool,(void *)cmd);
}

static inline void ipaugenblick_free_accepted_buf(ipaugenblick_cmd_t *cmd)
{
    rte_mempool_put(free_accepted_pool,(void *)cmd);
}

static inline void ipaugenblick_post_feedback(ipaugenblick_cmd_t *cmd)
{
    if(rte_ring_enqueue(feedbacks_ring,(void *)cmd)) { 
        ipaugenblick_free_command_buf(cmd);
    }
}

static inline ipaugenblick_cmd_t *ipaugenblick_get_free_accepted_buf()
{
    ipaugenblick_cmd_t *cmd;
    if(rte_mempool_get(free_accepted_pool,(void **)&cmd))
        return NULL;
    return cmd;
}

static inline void ipaugenblick_post_accepted(ipaugenblick_cmd_t *cmd)
{
    if(rte_ring_enqueue(accepted_ring,(void *)cmd)) { 
        ipaugenblick_free_accepted_buf(cmd);
    }
}

static inline struct rte_mbuf *ipaugenblick_dequeue_tx_buf(int ringset_idx)
{
    struct rte_mbuf *mbuf;
    if(rte_ring_dequeue(ringsets[ringset_idx].tx_ring,(void **)&mbuf)) { 
        return NULL;
    }
    return mbuf;
}

static inline int ipaugenblick_tx_buf_count(int ringset_idx)
{
    return rte_ring_count(ringsets[ringset_idx].tx_ring);
}

int ipaugenblick_submit_rx_buf(struct rte_mbuf *mbuf,int ringset_idx)
{
    return rte_ring_enqueue(ringsets[ringset_idx].rx_ring,(void *)mbuf);
}

#endif
