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
    ipaugenblick_socket_t *socket;
    int select;
    struct rte_ring *local_cache;
}local_socket_descriptor_t;

extern struct rte_ring *free_connections_ring;
extern struct rte_mempool *tx_bufs_pool;
extern struct rte_mempool *free_command_pool;
extern struct rte_ring *command_ring;
extern local_socket_descriptor_t local_socket_descriptors[IPAUGENBLICK_CONNECTION_POOL_SIZE];

static inline int ipaugenblick_enqueue_command_buf(ipaugenblick_cmd_t *cmd)
{
    returb (rte_ring_enqueue(command_ring,(void *)cmd) == -ENOBUFS);
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
    return (rte_ring_sp_enqueue_bulk(local_socket_descriptors[ringset_idx].tx_ring,(void *)&mbuf,1) == -ENOBUFS);
}

static inline int ipaugenblick_socket_tx_space(int ringset_idx)
{
    return rte_ring_free_count(local_socket_descriptors[ringset_idx].tx_ring);
}

static struct rte_mbuf *ipaugenblick_dequeue_rx_buf(int ringset_idx)
{
    struct rte_mbuf *mbuf = NULL,*mbufs[MAX_PKT_BURST];
    int send_kick = 0,dequeued;
    ipaugenblick_cmd_t *cmd;
 
    if(rte_ring_free_count(local_socket_descriptors[ringset_idx].rx_ring) == 0)
        send_kick = 1;
    if(rte_ring_count(local_socket_descriptors[ringset_idx].rx_ring) > 0) {
        dequeued = rte_ring_free_count(local_socket_descriptors[ringset_idx].local_cache) > MAX_PKT_BURST ? MAX_PKT_BURST : 
                     rte_ring_free_count(local_socket_descriptors[ringset_idx].local_cache);
        if(dequeued > 0) 
            dequeued = rte_ring_sc_dequeue_burst(
                        local_socket_descriptors[ringset_idx].rx_ring,
                        (void **)mbufs,
                        dequeued);
        if(dequeued > 0) {
            rte_ring_sp_enqueue_burst(local_socket_descriptors[ringset_idx].local_cache,
                                      (void **)mbufs,dequeued);
        }
    } 
    if(rte_ring_dequeue(local_socket_descriptors[ringset_idx].local_cache,(void **)&mbuf)) {
        mbuf = NULL;
    }
    if(send_kick) {
        cmd = ipaugenblick_get_free_command_buf();
        if(cmd) {
            cmd->cmd = IPAUGENBLICK_SOCKET_RX_KICK_COMMAND;
            cmd->ringset_idx = ringset_idx;
            ipaugenblick_enqueue_command_buf(cmd);
        }
    }
    return mbuf;
}

#endif /* __IPAUGENBLICK_RING_OPS_H__ */
