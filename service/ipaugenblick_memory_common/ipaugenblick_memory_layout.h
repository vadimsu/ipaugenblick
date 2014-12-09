
#ifndef __IPAUGENBLICK_MEMORY_LAYOUT_H__
#define __IPAUGENBLICK_MEMORY_LAYOUT_H__

#include "../ipaugenblick_memory_common/ipaugenblick_ring.h"

struct ipaugenblick_buffer_desc
{
    int offset;
    int buffer_length;
    unsigned int ringset_idx;
}__attribute__((packed));
#define PKTMBUF_HEADROOM 128
#define IPAUGENBLICK_BUFSIZE (PKTMBUF_HEADROOM+1448)
#define IPAUGENBLICK_RINGSETS_COUNT 10

struct ipaugenblick_ring_set
{ 
    unsigned int rx_ring_offset;
    unsigned int tx_ring_offset;
    unsigned int rx_free_bufs_ring_offset;
    unsigned int tx_free_bufs_ring_offset; 
    unsigned int rx_bufs_base_offset;
    unsigned int tx_bufs_base_offset;
}__attribute__((packed));

struct ipaugenblick_memory
{
    unsigned int command_ring_offset;
    unsigned int free_command_ring_offset;
    unsigned int command_bufs_base_offset;
    struct ipaugenblick_ring_set ring_sets[IPAUGENBLICK_RINGSETS_COUNT];
}__attribute__((packed));

static inline struct ipaugenblick_ring *ipaugenblick_get_command_ring(struct ipaugenblick_memory *memory)
{
    return (struct ipaugenblick_ring *)((char *)(memory+1) + memory->command_ring_offset);
}

static inline struct ipaugenblick_ring *ipaugenblick_get_free_command_ring(struct ipaugenblick_memory *memory)
{
    return (struct ipaugenblick_ring *)((char *)(memory+1) + memory->free_command_ring_offset);
}

static inline struct ipaugenblick_ring *ipaugenblick_get_rx_ring(struct ipaugenblick_memory *memory,int idx)
{
    return (struct ipaugenblick_ring *)((char *)(memory+1) + memory->ring_sets[idx].rx_ring_offset);
}

static inline struct ipaugenblick_ring *ipaugenblick_get_tx_ring(struct ipaugenblick_memory *memory,int idx)
{
    return (struct ipaugenblick_ring *)((char *)(memory+1) + memory->ring_sets[idx].tx_ring_offset);
}

static inline struct ipaugenblick_ring *ipaugenblick_get_rx_free_bufs_ring(struct ipaugenblick_memory *memory,int idx)
{
    return (struct ipaugenblick_ring *)((char *)(memory+1) + memory->ring_sets[idx].rx_free_bufs_ring_offset);
}

static inline struct ipaugenblick_ring *ipaugenblick_get_tx_free_bufs_ring(struct ipaugenblick_memory *memory,int idx)
{
    return (struct ipaugenblick_ring *)((char *)(memory+1) + memory->ring_sets[idx].tx_free_bufs_ring_offset);
}

static inline struct ipaugenblick_buffer_desc *ipaugenblick_get_command_bufs_base(struct ipaugenblick_memory *memory)
{
    return (struct ipaugenblick_buffer_desc *)((char *)(memory+1) + memory->command_bufs_base_offset);
}

static inline struct ipaugenblick_buffer_desc *ipaugenblick_get_rx_bufs_base(struct ipaugenblick_memory *memory,int idx)
{
    return (struct ipaugenblick_buffer_desc *)((char *)(memory+1) + memory->ring_sets[idx].rx_bufs_base_offset);
}

static inline struct ipaugenblick_buffer_desc *ipaugenblick_get_tx_bufs_base(struct ipaugenblick_memory *memory,int idx)
{
    return (struct ipaugenblick_buffer_desc *)((char *)(memory+1) + memory->ring_sets[idx].tx_bufs_base_offset);
}

static inline void ipaugenblick_set_command_ring(struct ipaugenblick_memory *memory,unsigned long offset)
{
    memory->command_ring_offset = offset;
}

static inline void ipaugenblick_set_free_command_ring(struct ipaugenblick_memory *memory,unsigned long offset)
{
    memory->free_command_ring_offset = offset;
}

static inline void ipaugenblick_set_rx_ring(struct ipaugenblick_memory *memory,unsigned long offset,int idx)
{
    memory->ring_sets[idx].rx_ring_offset = offset;
}

static inline void ipaugenblick_set_tx_ring(struct ipaugenblick_memory *memory,unsigned long offset,int idx)
{
    memory->ring_sets[idx].tx_ring_offset = offset;
}

static inline void ipaugenblick_set_rx_free_bufs_ring(struct ipaugenblick_memory *memory,unsigned long offset,int idx)
{
    memory->ring_sets[idx].rx_free_bufs_ring_offset = offset;
}

static inline void ipaugenblick_set_tx_free_bufs_ring(struct ipaugenblick_memory *memory,unsigned long offset,int idx)
{
    memory->ring_sets[idx].tx_free_bufs_ring_offset = offset;
}

static inline void ipaugenblick_set_command_bufs_base(struct ipaugenblick_memory *memory,unsigned long offset)
{
    memory->command_bufs_base_offset = offset;
}

static inline void ipaugenblick_set_rx_bufs_base(struct ipaugenblick_memory *memory,unsigned long offset,int idx)
{
    memory->ring_sets[idx].rx_bufs_base_offset = offset;
}

static inline void ipaugenblick_set_tx_bufs_base(struct ipaugenblick_memory *memory,unsigned long offset,int idx)
{
    memory->ring_sets[idx].tx_bufs_base_offset = offset;
}

static inline struct ipaugenblick_buffer_desc *ipaugenblick_get_free_command_buf(struct ipaugenblick_memory *memory)
{
    unsigned long buffidx;
    struct ipaugenblick_buffer_desc *bufdesc;
    char *p;
    struct ipaugenblick_ring *r = ipaugenblick_get_free_command_ring(memory);
    if(ipaugenblick_ring_dequeue(r,(void **)&buffidx) != 0) {
        return NULL;
    }
    p = (char *)ipaugenblick_get_command_bufs_base(memory);
    bufdesc = (struct ipaugenblick_buffer_desc *)(p + buffidx*IPAUGENBLICK_BUFSIZE);
    return bufdesc;
}

static inline struct ipaugenblick_buffer_desc *ipaugenblick_get_rx_free_buf(struct ipaugenblick_memory *memory,
                                                                            int idx)
{
    unsigned long buffidx;
    struct ipaugenblick_buffer_desc *bufdesc;
    char *p;
    struct ipaugenblick_ring *r = ipaugenblick_get_rx_free_bufs_ring(memory,idx);
    if(ipaugenblick_ring_dequeue(r,(void **)&buffidx) != 0) {
        return NULL;
    }
    p = (char *)ipaugenblick_get_rx_bufs_base(memory,idx);
    bufdesc = (struct ipaugenblick_buffer_desc *)(p + buffidx*IPAUGENBLICK_BUFSIZE);
    return bufdesc;
}

static inline struct ipaugenblick_buffer_desc *ipaugenblick_get_tx_free_buf(struct ipaugenblick_memory *memory,
                                                                            int idx)
{
    unsigned long buffidx;
    struct ipaugenblick_buffer_desc *bufdesc;
    char *p;
    struct ipaugenblick_ring *r = ipaugenblick_get_tx_free_bufs_ring(memory,idx);
    if(ipaugenblick_ring_dequeue(r,(void **)&buffidx) != 0) {
        return NULL;
    }
    p = (char *)ipaugenblick_get_tx_bufs_base(memory,idx);
    bufdesc = (struct ipaugenblick_buffer_desc *)(p + buffidx*IPAUGENBLICK_BUFSIZE);
    return bufdesc;
}

static inline int ipaugenblick_submit_command_buf(struct ipaugenblick_memory *memory,
                                                  struct ipaugenblick_buffer_desc *obj)
{
    struct ipaugenblick_buffer_desc *base = ipaugenblick_get_command_bufs_base(memory);
    unsigned long buffidx = ((char *)obj - (char *)base)/IPAUGENBLICK_BUFSIZE;
    struct ipaugenblick_ring *r = ipaugenblick_get_command_ring(memory);
    if(ipaugenblick_ring_enqueue(r,(void *)buffidx) != 0) {
        return -1;
    }
    return 0;
}

static inline int ipaugenblick_submit_rx_buf(struct ipaugenblick_memory *memory,
                                             struct ipaugenblick_buffer_desc *obj,
                                             int idx)
{
    struct ipaugenblick_buffer_desc *base = ipaugenblick_get_rx_bufs_base(memory,idx);
    unsigned long buffidx = ((char *)obj - (char *)base)/IPAUGENBLICK_BUFSIZE;
    struct ipaugenblick_ring *r = ipaugenblick_get_rx_ring(memory,idx);
    if(ipaugenblick_ring_enqueue(r,(void *)buffidx) != 0) {
        return -1;
    }
    return 0;
}

static inline int ipaugenblick_submit_tx_buf(struct ipaugenblick_memory *memory,
                                             struct ipaugenblick_buffer_desc *obj,
                                             int idx)
{
    struct ipaugenblick_buffer_desc *base = ipaugenblick_get_tx_bufs_base(memory,idx);
    unsigned long buffidx = ((char *)obj - (char *)base)/IPAUGENBLICK_BUFSIZE;
    struct ipaugenblick_ring *r = ipaugenblick_get_tx_ring(memory,idx);
    if(ipaugenblick_ring_enqueue(r,(void *)buffidx) != 0) {
        return -1;
    }
    return 0;
}

static inline struct ipaugenblick_buffer_desc *ipaugenblick_dequeue_command_buf(struct ipaugenblick_memory *memory)
{
    unsigned long buffidx;
    struct ipaugenblick_buffer_desc *bufdesc;
    char *p;
    struct ipaugenblick_ring *r = ipaugenblick_get_command_ring(memory);
    if(ipaugenblick_ring_dequeue(r,(void **)&buffidx) != 0) {
        return NULL;
    }
    p = (char *)ipaugenblick_get_command_bufs_base(memory);
    bufdesc = (struct ipaugenblick_buffer_desc *)(p + buffidx*IPAUGENBLICK_BUFSIZE);
    return bufdesc;
}

static inline struct ipaugenblick_buffer_desc * ipaugenblick_dequeue_rx_buf(struct ipaugenblick_memory *memory,int idx)
{
    unsigned long buffidx;
    struct ipaugenblick_buffer_desc *bufdesc;
    char *p;
    struct ipaugenblick_ring *r = ipaugenblick_get_rx_ring(memory,idx);
    if(ipaugenblick_ring_dequeue(r,(void *)&buffidx) != 0) {
        return NULL;
    }
    p = (char *)ipaugenblick_get_rx_bufs_base(memory,idx);
    bufdesc = (struct ipaugenblick_buffer_desc *)(p + buffidx*IPAUGENBLICK_BUFSIZE);
    return bufdesc;
}

static inline struct ipaugenblick_buffer_desc *ipaugenblick_dequeue_tx_buf(struct ipaugenblick_memory *memory,int idx)
{
    unsigned long buffidx;
    struct ipaugenblick_buffer_desc *bufdesc;
    char *p;
    struct ipaugenblick_ring *r = ipaugenblick_get_tx_ring(memory,idx);
    if(ipaugenblick_ring_dequeue(r,(void *)&buffidx) != 0) {
        return NULL;
    }
    p = (char *)ipaugenblick_get_tx_bufs_base(memory,idx);
    bufdesc = (struct ipaugenblick_buffer_desc *)(p + buffidx*IPAUGENBLICK_BUFSIZE);
    return bufdesc;
}

static inline int ipaugenblick_free_command_buf(struct ipaugenblick_memory *memory,
                                                struct ipaugenblick_buffer_desc *obj)
{
    struct ipaugenblick_buffer_desc *base = ipaugenblick_get_command_bufs_base(memory);
    unsigned long buffidx = ((char *)obj - (char *)base)/IPAUGENBLICK_BUFSIZE;
    struct ipaugenblick_ring *r = ipaugenblick_get_free_command_ring(memory);
    if(ipaugenblick_ring_enqueue(r,(void *)buffidx) != 0) {
        return -1;
    }
    return 0;
}

static inline int ipaugenblick_free_rx_buf(struct ipaugenblick_memory *memory,
                                           struct ipaugenblick_buffer_desc *obj,
                                           int idx)
{
    struct ipaugenblick_buffer_desc *base = ipaugenblick_get_rx_bufs_base(memory,idx);
    unsigned long buffidx = ((char *)obj - (char *)base)/IPAUGENBLICK_BUFSIZE;
    struct ipaugenblick_ring *r = ipaugenblick_get_rx_free_bufs_ring(memory,idx);
    if(ipaugenblick_ring_enqueue(r,(void *)buffidx) != 0) {
        return -1;
    }
    return 0;
}

static inline int ipaugenblick_free_tx_buf(struct ipaugenblick_memory *memory,
                                           struct ipaugenblick_buffer_desc *obj,
                                           int idx)
{
    struct ipaugenblick_buffer_desc *base = ipaugenblick_get_tx_bufs_base(memory,idx);
    unsigned long buffidx = ((char *)obj - (char *)base)/IPAUGENBLICK_BUFSIZE;
    struct ipaugenblick_ring *r = ipaugenblick_get_tx_free_bufs_ring(memory,idx);
    if(ipaugenblick_ring_enqueue(r,(void *)buffidx) != 0) {
        return -1;
    }
    return 0;
}

struct ipaugenblick_memory *ipaugenblick_memory_init(void *base_address,
                                                     unsigned int length,
                                                     int command_bufs_count,
                                                     int rx_bufs_count,
                                                     int tx_bufs_count);

#endif
