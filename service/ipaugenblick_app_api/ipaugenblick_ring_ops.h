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
    int any_event_received;
    struct rte_mbuf *shadow;
    struct rte_mbuf *shadow_next;
    int shadow_len_remainder;
    int shadow_len_delievered;
    unsigned int local_ipaddr;
    unsigned int local_port;
    unsigned int remote_ipaddr;
    unsigned int remote_port;
}local_socket_descriptor_t;

extern struct rte_ring *free_connections_ring;
extern struct rte_mempool *tx_bufs_pool;
extern struct rte_mempool *free_command_pool;
extern struct rte_ring *command_ring;
extern local_socket_descriptor_t local_socket_descriptors[IPAUGENBLICK_CONNECTION_POOL_SIZE];

extern uint64_t ipaugenblick_stats_rx_kicks_sent;
extern uint64_t ipaugenblick_stats_rx_full;
extern uint64_t ipaugenblick_stats_rx_dequeued;
extern uint64_t ipaugenblick_stats_rx_dequeued_local;

static inline int ipaugenblick_enqueue_command_buf(ipaugenblick_cmd_t *cmd)
{
    return (rte_ring_enqueue(command_ring,(void *)cmd) == -ENOBUFS);
}

static inline int ipaugenblick_enqueue_tx_buf(int ringset_idx,struct rte_mbuf *mbuf)
{
    return (rte_ring_sp_enqueue_bulk(local_socket_descriptors[ringset_idx].tx_ring,(void **)&mbuf,1) == -ENOBUFS);
}

static inline int ipaugenblick_enqueue_tx_bufs_bulk(int ringset_idx,struct rte_mbuf **mbufs,int buffer_count)
{
    return (rte_ring_sp_enqueue_bulk(local_socket_descriptors[ringset_idx].tx_ring,(void **)mbufs,buffer_count) == -ENOBUFS);
}

static inline int ipaugenblick_socket_tx_space(int ringset_idx)
{
    return rte_ring_free_count(local_socket_descriptors[ringset_idx].tx_ring);
}

static struct rte_mbuf *ipaugenblick_dequeue_rx_buf(int ringset_idx)
{
    struct rte_mbuf *mbuf = NULL,*mbufs[MAX_PKT_BURST];
    int send_kick = 1,dequeued;
    ipaugenblick_cmd_t *cmd;
 
    if(rte_ring_free_count(local_socket_descriptors[ringset_idx].rx_ring) == 0) {
        send_kick = 1;
        ipaugenblick_stats_rx_full++;
    }
    rte_atomic16_set(&(local_socket_descriptors[ringset_idx & SOCKET_READY_MASK].socket->read_ready_to_app),0);
    if(rte_ring_count(local_socket_descriptors[ringset_idx].rx_ring) > 0) {
        dequeued = rte_ring_free_count(local_socket_descriptors[ringset_idx].local_cache) > MAX_PKT_BURST ? MAX_PKT_BURST : 
                     rte_ring_free_count(local_socket_descriptors[ringset_idx].local_cache);
        if(dequeued > 0) 
            dequeued = rte_ring_sc_dequeue_burst(
                        local_socket_descriptors[ringset_idx].rx_ring,
                        (void **)mbufs,
                        dequeued);
        if(dequeued > 0) {
            ipaugenblick_stats_rx_dequeued++;
            send_kick = 1;
            if(rte_ring_count(local_socket_descriptors[ringset_idx].local_cache) > 0) {
                rte_ring_sp_enqueue_burst(local_socket_descriptors[ringset_idx].local_cache,
                                          (void **)mbufs,dequeued);
            }
            else {
                mbuf = mbufs[0];
                if(dequeued > 1) {
                    rte_ring_sp_enqueue_burst(local_socket_descriptors[ringset_idx].local_cache,
                                          (void **)&mbufs[1],dequeued - 1);
                }
                goto skip_local;
            }
        }
    } 
    if(rte_ring_dequeue(local_socket_descriptors[ringset_idx].local_cache,(void **)&mbuf)) {
        mbuf = NULL;
    }
    else {
        ipaugenblick_stats_rx_dequeued_local++;
    }
skip_local:
    if(send_kick) {
        cmd = ipaugenblick_get_free_command_buf();
        if(cmd) {
            cmd->cmd = IPAUGENBLICK_SOCKET_RX_KICK_COMMAND;
            cmd->ringset_idx = ringset_idx;
            ipaugenblick_enqueue_command_buf(cmd);
            ipaugenblick_stats_rx_kicks_sent++;
        }
    }
    return mbuf;
}

#endif /* __IPAUGENBLICK_RING_OPS_H__ */
