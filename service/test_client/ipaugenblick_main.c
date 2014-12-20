#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/errno.h>
#include "../ipaugenblick_common/ipaugenblick_common.h"
#include "../ipaugenblick_app_api/ipaugenblick_api.h"
#include <string.h>

void *memory = NULL;

int main(int argc,char **argv)
{
    void *buff;
    int sock,len;
    ipaugenblick_cmd_t *cmd;
    char *p;
    int size = 0,ringset_idx;
    if(ipaugenblick_app_init(argc,argv) != 0) {
        printf("cannot initialize memory\n");
        return 0;
    } 
    if((sock = ipaugenblick_open_tcp_client(inet_addr("192.168.150.62"),7777,inet_addr("192.168.150.63"),7777)) < 0) {
        printf("cannot open tcp client socket\n");
        return 0;
    }
    while(1) {
    //    printf("receiving\n");
        if(ipaugenblick_receive(sock,&buff,&len) == 0) {
            printf("received %p\n",buff);
            ipaugenblick_release_rx_buffer(buff);
#if 0
            buff = ipaugenblick_get_buffer(1024);
            if(buff) {
//             printf("sending...\n");
                if(ipaugenblick_send(sock,buff,0,1024,NULL,NULL)) { 
  //              printf("failed\n");
                    ipaugenblick_release_tx_buffer(buff);
                }
            }
#endif
        }
    }
    return 0;
}
