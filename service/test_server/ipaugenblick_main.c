#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/errno.h>
#include "../ipaugenblick_memory_common/ipaugenblick_memory_layout.h"
#include "../ipaugenblick_memory_service/ipaugenblick_service.h"
#include "../ipaugenblick_memory_common/ipaugenblick_memory_common.h"
#include <string.h>

static void *memory = NULL;

int main(int argc,char **argv)
{
    int ringset_idx;
    struct ipaugenblick_buffer_desc *buff,*buff2;
    ipaugenblick_cmd_t *cmd;
    char *p;

    if((memory = ipaugenblick_service_init(0)) == NULL) {
        printf("cannot initialize memory\n");
    }
    while(1) {
        for(ringset_idx = 0;ringset_idx < IPAUGENBLICK_RINGSETS_COUNT;ringset_idx++) {
            buff = ipaugenblick_dequeue_command_buf(memory,ringset_idx);
            if(!buff)
                continue;
            cmd = (ipaugenblick_cmd_t *)buff;   
            switch(cmd->cmd) {
                case IPAUGENBLICK_OPEN_CLIENT_SOCKET_COMMAND:
                   printf("open_client_sock %x %x\n",cmd->u.open_client_sock.ipaddress,cmd->u.open_client_sock.port);
                   break;
                case IPAUGENBLICK_OPEN_LISTENING_SOCKET_COMMAND:
                   printf("open_listening_sock %x %x\n",
                           cmd->u.open_listening_sock.ipaddress,cmd->u.open_listening_sock.port);
                   break;
                case IPAUGENBLICK_OPEN_UDP_SOCKET_COMMAND:
                   printf("open_udp_sock %x %x\n",cmd->u.open_udp_sock.ipaddress,cmd->u.open_udp_sock.port);
                   break;
                case IPAUGENBLICK_OPEN_RAW_SOCKET_COMMAND:
                   printf("open_raw_sock %x %x\n",cmd->u.open_raw_sock.ipaddress,cmd->u.open_raw_sock.protocol);
                   break;
                default:
                   printf("unknown cmd %d\n",cmd->cmd);
                   break;
            }
            ipaugenblick_free_command_buf(memory,buff,ringset_idx);
        }
        for(ringset_idx = 0;ringset_idx < IPAUGENBLICK_RINGSETS_COUNT;ringset_idx++) {
            buff = ipaugenblick_dequeue_tx_buf(memory,ringset_idx);
            if(!buff)
                continue; 
            p = (char *)buff;
            printf("tx buff content %s\n",p);
            ipaugenblick_free_tx_buf(memory,buff,ringset_idx);
        }
        for(ringset_idx = 0;ringset_idx < IPAUGENBLICK_RINGSETS_COUNT;ringset_idx++) { 
            buff = ipaugenblick_get_rx_free_buf(memory,ringset_idx);
            if(!buff) {
                continue;
            }
            p = (char *)buff;
            sprintf(p,"SERVER%d\n",ringset_idx);
            ipaugenblick_submit_rx_buf(memory,buff,ringset_idx);
        }
    }
    return 0;
}
