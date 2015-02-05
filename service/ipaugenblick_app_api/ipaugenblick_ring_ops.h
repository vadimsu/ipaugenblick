#ifndef __IPAUGENBLICK_RING_OPS_H__
#define __IPAUGENBLICK_RING_OPS_H__

/* This holds mapping to connection's tx/rx rings,
 * index of selector
 * parent socket (when accepted)
*/
typedef struct 
{
    struct rte_ring *tx_ring;
    struct rte_ring *rx_ring;
    //unsigned long sock;
    ipaugenblick_socket_t *socket;
    int select;
}local_socket_descriptor_t;

extern struct rte_ring *free_connections_ring;
extern struct rte_mempool *tx_bufs_pool;
extern struct rte_mempool *free_command_pool;
extern struct rte_ring *command_ring;
extern local_socket_descriptor_t local_socket_descriptors[IPAUGENBLICK_CONNECTION_POOL_SIZE];

static inline void ipaugenblick_enqueue_command_buf(ipaugenblick_cmd_t *cmd)
{
    rte_ring_enqueue(command_ring,(void *)cmd);
}

static inline ipaugenblick_cmd_t *ipaugenblick_get_free_command_buf()
{
    ipaugenblick_cmd_t *cmd;
    if(rte_mempool_get(free_command_pool,(void **)&cmd))
        return NULL;
    return cmd;
}

static inline int ipaugenblick_enqueue_tx_buf(int ringset_idx,struct rte_mbuf *mbuf)
{
    return rte_ring_sp_enqueue_bulkl(local_socket_descriptors[ringset_idx].tx_ring,(void *)&mbuf,1); 
}

static struct rte_mbuf *ipaugenblick_dequeue_rx_buf(int ringset_idx)
{
    struct rte_mbuf *mbuf;
    if(rte_ring_sc_dequeue_bulk(local_socket_descriptors[ringset_idx].rx_ring,(void **)&mbuf,1)) {
        return NULL;
    }
    return mbuf;
}

#endif /* __IPAUGENBLICK_RING_OPS_H__ */
