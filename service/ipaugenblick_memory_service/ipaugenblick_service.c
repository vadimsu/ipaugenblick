#include <stdlib.h>
#include <sys/errno.h>
#include <stdio.h>
#include "../ipaugenblick_memory_common/ipaugenblick_memory_layout.h"
#include "../ipaugenblick_memory_common/ipaugenblick_memory_common.h"

#define MB_1 (1024*1024)
#define MB_8 (8*MB_1)

static struct ipaugenblick_memory *memory_base_address = NULL;

static int get_nearest_power_of_2(unsigned int count)
{
    int bitIdx,highest = 0;

    for(bitIdx = 31;bitIdx >= 0;bitIdx--) {
        if(count & (1 << bitIdx)) {
            if(!highest)
                highest = bitIdx;
            else if(bitIdx > 0) {
                return (1<<(highest));
            }
        }
    }
    return 1;
}

void *ipaugenblick_service_init(int memory_size)
{
    unsigned int command_count,rx_count,tx_count;
    memory_size = MB_8;
    memory_base_address = ipaugenblick_shared_memory_init(memory_size);    

    command_count = get_nearest_power_of_2(2);
    rx_count = get_nearest_power_of_2(((memory_size/IPAUGENBLICK_RINGSETS_COUNT)/(IPAUGENBLICK_BUFSIZE*3)-2));
    tx_count = get_nearest_power_of_2(((memory_size/IPAUGENBLICK_RINGSETS_COUNT)/(IPAUGENBLICK_BUFSIZE*3)-2));
    printf("command_count %u rx_count %u tx_count %u\n",command_count,rx_count,tx_count);
    if(ipaugenblick_memory_init(memory_base_address,
                                memory_size,command_count,
                                rx_count,
                                tx_count) != memory_base_address) {
        printf("cannot initialize %s %d\n",__FILE__,__LINE__);
        return NULL;
    }
    return memory_base_address;
}
