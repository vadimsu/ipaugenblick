#ifndef __IPAUGENBLICK_RING_OPS_H__
#define __IPAUGENBLICK_RING_OPS_H__

typedef struct _socket_descriptor
{
    int ringset_idx;
/*    union {
        on_send_complete_t on_send_complete;
        on_accepted_t on_accepted;
    }u;*/
    struct rte_ring *tx_ring;
    struct rte_ring *rx_ring;
    struct rte_ring *feedbacks_ring;
    unsigned long sock;
}socket_descriptor_t;

extern struct rte_ring *free_connections_ring;
extern struct rte_mempool *tx_bufs_pool;
extern struct rte_mempool *free_command_pool;
extern struct rte_ring *command_ring;
extern socket_descriptor_t socket_descriptors[IPAUGENBLICK_CONNECTION_POOL_SIZE];

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
    return rte_ring_enqueue(socket_descriptors[ringset_idx].tx_ring,(void *)mbuf); 
}

static struct rte_mbuf *ipaugenblick_dequeue_rx_buf(int ringset_idx)
{
    struct rte_mbuf *mbuf;
    if(rte_ring_dequeue(socket_descriptors[ringset_idx].rx_ring,(void **)&mbuf)) {
        return NULL;
    }
    return mbuf;
}

#endif /* __IPAUGENBLICK_RING_OPS_H__ */
