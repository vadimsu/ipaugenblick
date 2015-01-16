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

int main(int argc,char **argv)
{
    void **buff;
    int sock,len;
    ipaugenblick_cmd_t *cmd;
    char *p;
    int size = 0,ringset_idx;
    int socket_connected = -1;
    unsigned int from_ip;
    unsigned short from_port;

    if(ipaugenblick_app_init(argc,argv) != 0) {
        printf("cannot initialize memory\n");
        return 0;
    } 
    if((sock = ipaugenblick_open_udp(inet_addr("192.168.150.63"),7777)) < 0) {
        printf("cannot open UDP socket\n");
        return 0;
    }
    while(1) {
/*        if(socket_connected == -1) {
            socket_connected = ipaugenblick_get_connected();
        }
        if(socket_connected == -1) {
            continue;
        }*/
#if 0
        buff = ipaugenblick_get_buffer(1448);
        if(buff) {
         printf("sending...\n");
            if(ipaugenblick_sendto(sock,buff,0,1448,inet_addr("192.168.150.62"),7777)) { 
                printf("failed\n");
                ipaugenblick_release_tx_buffer(buff);
                ipaugenblick_socket_kick(sock);
            } 
        }
        else {
//            ipaugenblick_socket_kick(sock);
        }
#endif
#if 1
        if(socket_connected == -1) {
            socket_connected = ipaugenblick_get_connected();
            if(socket_connected == -1)
                continue;
        }
        if(ipaugenblick_receivefrom(sock,&buff,&len,&from_ip,&from_port) == 0) {
     //       printf("received %p %x %d\n",buff,from_ip,from_port);
            ipaugenblick_release_rx_buffer(buff);
#if 1
            buff = ipaugenblick_get_buffer(1448);
            if(buff) {
             //printf("sending...\n");
                if(ipaugenblick_sendto(sock,buff,0,1448,from_ip,from_port)) { 
//                    printf("failed\n");
                    ipaugenblick_release_tx_buffer(buff);
                }
                else
                    printf("sent...\n");
                ipaugenblick_socket_kick(socket_connected);
            } 
#endif
        }
#endif
    }
    return 0;
}

