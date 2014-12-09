#include <stdlib.h>
#include <sys/errno.h>
#include <stdio.h>
#include "../ipaugenblick_memory_common/ipaugenblick_ring.h"
#include "../ipaugenblick_memory_common/ipaugenblick_memory_layout.h"

static void dump_rings()
{
}

struct ipaugenblick_memory *ipaugenblick_memory_init(void *base_address,
                                                     unsigned int length,
                                                     int command_bufs_count,
                                                     int rx_bufs_count,
                                                     int tx_bufs_count)
{
    struct ipaugenblick_memory *memory = (struct ipaugenblick_memory *)base_address;
    int rx_rings_size = ipaugenblick_ring_get_memsize(rx_bufs_count);
    int tx_rings_size = ipaugenblick_ring_get_memsize(tx_bufs_count);
    int command_rings_size = ipaugenblick_ring_get_memsize(command_bufs_count);
    unsigned int rx_buffs_offset;
    unsigned int tx_buffs_offset;
    struct ipaugenblick_ring *r;
    struct ipaugenblick_buffer_desc *bufdesc;
    int i,ringset_idx,offset = 0;
    unsigned int requested_size = (command_bufs_count*IPAUGENBLICK_BUFSIZE+
        rx_bufs_count*IPAUGENBLICK_BUFSIZE+
        tx_bufs_count*IPAUGENBLICK_BUFSIZE)*IPAUGENBLICK_RINGSETS_COUNT;

    if(requested_size > length) {
        printf("overall requested size %u is greater than available %u\n",requested_size,length);
        return NULL;
    }
    printf("cmd_count %u rx_count %u tx_count %u requested_size %u\n",
           command_bufs_count,rx_bufs_count,tx_bufs_count,requested_size);

    ipaugenblick_set_command_ring(memory,offset);
    offset += command_rings_size;
    ipaugenblick_set_free_command_ring(memory,offset);
    offset += command_rings_size;
    ipaugenblick_set_command_bufs_base(memory,offset);
    offset += command_bufs_count*IPAUGENBLICK_BUFSIZE;
    /* COMMAND RING */
    r = ipaugenblick_get_command_ring(memory);
    ipaugenblick_ring_init(r,command_bufs_count,0);
    /* FREE COMMAND RING */
    r = ipaugenblick_get_free_command_ring(memory);
    ipaugenblick_ring_init(r,command_bufs_count,0);
    /* COMMAND BUFFERS */
    bufdesc = ipaugenblick_get_command_bufs_base(memory);
    r = ipaugenblick_get_free_command_ring(memory);
    for(i = 0;i < command_bufs_count;i++) {
        bufdesc->connection_handle = 0;
        bufdesc->buffer_length = (IPAUGENBLICK_BUFSIZE - PKTMBUF_HEADROOM);
        ipaugenblick_free_command_buf(memory,bufdesc);
        printf("%s %d\n",__FILE__,__LINE__);
        bufdesc = (struct ipaugenblick_buffer_desc *)((char *)bufdesc + IPAUGENBLICK_BUFSIZE);
    }
    for(ringset_idx = 0;ringset_idx < IPAUGENBLICK_RINGSETS_COUNT;ringset_idx++) { 
        ipaugenblick_set_rx_ring(memory,offset,ringset_idx);
        offset += rx_rings_size; 
        ipaugenblick_set_tx_ring(memory,offset,ringset_idx);
        offset += tx_rings_size;
        ipaugenblick_set_rx_free_bufs_ring(memory,offset,ringset_idx);
        offset += rx_rings_size;
        ipaugenblick_set_tx_free_bufs_ring(memory,offset,ringset_idx);
        offset += tx_rings_size; 
        ipaugenblick_set_rx_bufs_base(memory,offset,ringset_idx);
        offset += rx_bufs_count*IPAUGENBLICK_BUFSIZE;
        ipaugenblick_set_tx_bufs_base(memory,offset,ringset_idx);
        offset += tx_bufs_count*IPAUGENBLICK_BUFSIZE; 
     
        /* RX RING */
        r = ipaugenblick_get_rx_ring(memory,ringset_idx);
        ipaugenblick_ring_init(r,rx_bufs_count,0);
        /* TX RING */
        r = ipaugenblick_get_tx_ring(memory,ringset_idx);
        ipaugenblick_ring_init(r,tx_bufs_count,0);
        /* RX FREE RING */
        r = ipaugenblick_get_rx_free_bufs_ring(memory,ringset_idx);
        ipaugenblick_ring_init(r,rx_bufs_count,0);
        /* TX FREE RING */
        r = ipaugenblick_get_tx_free_bufs_ring(memory,ringset_idx);
        ipaugenblick_ring_init(r,rx_bufs_count,0);
        /* RX BUFFERS */
        bufdesc = ipaugenblick_get_rx_bufs_base(memory,ringset_idx);
        r = ipaugenblick_get_rx_free_bufs_ring(memory,ringset_idx);
        for(i = 0;i < rx_bufs_count;i++) {
            bufdesc->connection_handle = 0;
            bufdesc->buffer_length = (IPAUGENBLICK_BUFSIZE - PKTMBUF_HEADROOM);
            ipaugenblick_free_rx_buf(memory,bufdesc,ringset_idx);
            bufdesc = (struct ipaugenblick_buffer_desc *)((char *)bufdesc + IPAUGENBLICK_BUFSIZE);
        }
        /* TX BUFFERS */
        bufdesc = ipaugenblick_get_tx_bufs_base(memory,ringset_idx);
        r = ipaugenblick_get_tx_free_bufs_ring(memory,ringset_idx);
        for(i = 0;i < rx_bufs_count;i++) {
            bufdesc->connection_handle = 0;
            bufdesc->buffer_length = (IPAUGENBLICK_BUFSIZE - PKTMBUF_HEADROOM);
            ipaugenblick_free_tx_buf(memory,bufdesc,ringset_idx);
            bufdesc = (struct ipaugenblick_buffer_desc *)((char *)bufdesc + IPAUGENBLICK_BUFSIZE);
        }
        printf("%s %d %d %d %d %d %d %d %p %p %p\n",__FILE__,__LINE__,
           memory->ring_sets[ringset_idx].rx_ring_offset,
           memory->ring_sets[ringset_idx].tx_ring_offset,
           memory->ring_sets[ringset_idx].rx_free_bufs_ring_offset,
           memory->ring_sets[ringset_idx].tx_free_bufs_ring_offset,
           memory->ring_sets[ringset_idx].rx_bufs_base_offset,
           memory->ring_sets[ringset_idx].tx_bufs_base_offset,
           memory,
           ipaugenblick_get_rx_ring(memory,ringset_idx),
           ipaugenblick_get_tx_ring(memory,ringset_idx));
    }
    printf("%s %d %p\n",__FILE__,__LINE__,memory);
    return memory;
}
 
