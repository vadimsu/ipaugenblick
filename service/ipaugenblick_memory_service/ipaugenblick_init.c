#include <stdlib.h>
#include <sys/errno.h>
#include <stdio.h>
#include "../ipaugenblick_memory_common/ipaugenblick_memory_layout.h"

struct ipaugenblick_memory *ipaugenblick_memory_init(void *base_address,
                                                     unsigned int length,
                                                     int command_bufs_count,
                                                     int rx_bufs_count,
                                                     int tx_bufs_count)
{
    struct ipaugenblick_memory *memory = (struct ipaugenblick_memory *)base_address;
    int rx_rings_size = ipaugenblick_ring_get_memsize(rx_bufs_count);
    int tx_rings_size = ipaugenblick_ring_get_memsize(tx_bufs_count);
    unsigned int rx_buffs_offset;
    unsigned int tx_buffs_offset;
    struct ipaugenblick_ring *r;
    struct ipaugenblick_buffer_desc *bufdesc;
    int i;
    
    ipaugenblick_set_command_ring(memory,0);
    ipaugenblick_set_free_command_ring(memory,command_bufs_count);
    ipaugenblick_set_rx_ring(memory,command_bufs_count*2);
    ipaugenblick_set_tx_ring(memory,command_bufs_count*2 + rx_rings_size);
    ipaugenblick_set_rx_free_bufs_ring(memory,memory->tx_ring_offset + tx_rings_size);
    ipaugenblick_set_tx_free_bufs_ring(memory,memory->rx_free_bufs_ring_offset + rx_rings_size);
    ipaugenblick_set_command_bufs_base(memory,memory->tx_free_bufs_ring_offset + tx_rings_size);
    ipaugenblick_set_rx_bufs_base(memory,command_bufs_count*IPAUGENBLICK_BUFSIZE + memory->tx_free_bufs_ring_offset + tx_rings_size);
    ipaugenblick_set_tx_bufs_base(memory,command_bufs_count*IPAUGENBLICK_BUFSIZE + memory->rx_bufs_base_offset + rx_bufs_count*IPAUGENBLICK_BUFSIZE);
    
    if(length < (memory->tx_bufs_base_offset + tx_bufs_count*IPAUGENBLICK_BUFSIZE)) {
        printf("pool is too small for paramters: length %d rx_bufs_count %d tx_bufs_count %d\n",
                length,rx_bufs_count,tx_bufs_count);
        printf("rx_ring_size %d tx_ring_size %d rx_buffs_size %d tx_buffs_size %d\n",
               rx_rings_size,tx_rings_size,
               rx_bufs_count*IPAUGENBLICK_BUFSIZE,tx_bufs_count*IPAUGENBLICK_BUFSIZE);
        return NULL;
    }
    /* COMMAND RING */
    r = ipaugenblick_get_command_ring(memory);
    ipaugenblick_ring_init(r,command_bufs_count,RING_F_SP_ENQ|RING_F_SC_DEQ);
     /* FREE COMMAND RING */
    r = ipaugenblick_get_free_command_ring(memory);
    ipaugenblick_ring_init(r,command_bufs_count,RING_F_SP_ENQ|RING_F_SC_DEQ);
    /* RX RING */
    r = ipaugenblick_get_rx_ring(memory);
    ipaugenblick_ring_init(r,rx_bufs_count,RING_F_SP_ENQ|RING_F_SC_DEQ);
    /* TX RING */
    r = ipaugenblick_get_tx_ring(memory);
    ipaugenblick_ring_init(r,tx_bufs_count,RING_F_SP_ENQ|RING_F_SC_DEQ);
    /* RX FREE RING */
    r = ipaugenblick_get_rx_free_bufs_ring(memory);
    ipaugenblick_ring_init(r,rx_bufs_count,RING_F_SP_ENQ|RING_F_SC_DEQ);
    /* TX FREE RING */
    r = ipaugenblick_get_tx_free_bufs_ring(memory);
    ipaugenblick_ring_init(r,rx_bufs_count,RING_F_SP_ENQ|RING_F_SC_DEQ);
    /* RX BUFFERS */
    bufdesc = ipaugenblick_get_rx_bufs_base(memory);
    r = ipaugenblick_get_rx_free_bufs_ring(memory);
    for(i = 0;i < rx_bufs_count;i++) {
        bufdesc->connection_handle = 0;
        bufdesc->buffer_length = (IPAUGENBLICK_BUFSIZE - sizeof(struct ipaugenblick_buffer_desc));
        ipaugenblick_free_rx_buf(memory,bufdesc);
//        ipaugenblick_ring_enqueue(r,bufdesc);
        bufdesc = (struct ipaugenblick_buffer_desc *)((char *)bufdesc + IPAUGENBLICK_BUFSIZE);
    }
    /* TX BUFFERS */
    bufdesc = ipaugenblick_get_tx_bufs_base(memory);
    r = ipaugenblick_get_tx_free_bufs_ring(memory);
    for(i = 0;i < rx_bufs_count;i++) {
        bufdesc->connection_handle = 0;
        bufdesc->buffer_length = (IPAUGENBLICK_BUFSIZE - sizeof(struct ipaugenblick_buffer_desc));
        ipaugenblick_free_tx_buf(memory,bufdesc);
//        ipaugenblick_ring_enqueue(r,bufdesc);
        bufdesc = (struct ipaugenblick_buffer_desc *)((char *)bufdesc + IPAUGENBLICK_BUFSIZE);
    }
    printf("%s %d %d %d %d %d %d %d %p %p %p\n",__FILE__,__LINE__,
memory->rx_ring_offset,
memory->tx_ring_offset,
memory->rx_free_bufs_ring_offset,
memory->tx_free_bufs_ring_offset,
memory->rx_bufs_base_offset,
memory->tx_bufs_base_offset,
memory,
ipaugenblick_get_rx_ring(memory),
ipaugenblick_get_tx_ring(memory));
    return memory;
}
 
