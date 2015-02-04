#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/errno.h>
#include "../ipaugenblick_app_api/ipaugenblick_api.h"
#include <string.h>

#define USE_TX 0

int main(int argc,char **argv)
{
    void *buff,**rxbuff;
    int sock,len;
    char *p;
    int size = 0,ringset_idx;
    int socket_connected = -1;
    int selector;
    int socket_to_read;
    int i;
    unsigned short mask;

    if(ipaugenblick_app_init(argc,argv) != 0) {
        printf("cannot initialize memory\n");
        return 0;
    } 
    printf("memory initialized\n");
    if((sock = ipaugenblick_open_tcp_server(inet_addr("192.168.150.63"),7777)) < 0) {
        printf("cannot open tcp client socket\n");
        return 0;
    }
    printf("listener socket opened\n");
    selector = ipaugenblick_open_select();
    if(selector != -1) {
        printf("selector opened\n");
    }
    while(1) {
        if(socket_connected == -1) {
            socket_connected = ipaugenblick_accept(sock);
            if(socket_connected != -1) {
                printf("socket accepted %d %d\n",socket_connected,selector);
                ipaugenblick_set_socket_select(socket_connected,selector);
            }
        }
        if(socket_connected == -1) {
            continue;
        }
        socket_to_read = ipaugenblick_select(selector,&mask);
        if(socket_to_read == -1) {
            continue;
        }
        //printf("%s %d %d\n",__FILE__,__LINE__,socket_to_read);
        if(ipaugenblick_receive(socket_to_read,&rxbuff,&len) == 0) {
          //  printf("%s %d\n",__FILE__,__LINE__);
            ipaugenblick_release_rx_buffer(rxbuff);
#if USE_TX
            buff = ipaugenblick_get_buffer(1448);
            if(buff) {
                if(ipaugenblick_send(socket_to_read,buff,0,1448)) { 
                    ipaugenblick_release_tx_buffer(buff);
                    ipaugenblick_socket_kick(socket_to_read);
                }
            }
            else {
                ipaugenblick_socket_kick(socket_to_read);
            } 
#endif
        } 
    }
    return 0;
}
