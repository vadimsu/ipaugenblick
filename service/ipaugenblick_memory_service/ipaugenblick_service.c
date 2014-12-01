#include <stdlib.h>
#include <sys/errno.h>
#include <stdio.h>
#include "../ipaugenblick_memory_common/ipaugenblick_memory_layout.h"
#include "../ipaugenblick_memory_common/ipaugenblick_memory_common.h"

#define MB_1 (1024*1024)
#define MB_8 (8*MB_1)

static struct ipaugenblick_memory *memory_base_address = NULL;

int ipaugenblick_service_init(int memory_size)
{
    memory_size = MB_8;
    memory_base_address = ipaugenblick_shared_memory_init(memory_size);    

    if(ipaugenblick_memory_init(memory_base_address,memory_size,2,(memory_size/IPAUGENBLICK_BUFSIZE*2)-2,(memory_size/IPAUGENBLICK_BUFSIZE*2)-2) != memory_base_address) {
        printf("cannot initialize %s %d\n",__FILE__,__LINE__);
        return -1;
    }
    return 0;
}
