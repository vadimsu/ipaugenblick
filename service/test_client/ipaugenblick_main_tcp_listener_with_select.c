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
    int sock,newsock,len;
    char *p;
    int size = 0,ringset_idx;
    int sockets_connected = 0;
    int selector;
    int ready_socket;
    int i;
    unsigned short mask;
    unsigned long received_count = 0;

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
        newsock = ipaugenblick_accept(sock);
        if(newsock != -1) {
            printf("socket accepted %d %d\n",newsock,selector);
            ipaugenblick_set_socket_select(newsock,selector);
            sockets_connected++;
        }
        if(sockets_connected == 0) {
            continue;
        }
        ready_socket = ipaugenblick_select(selector,&mask);
        if(ready_socket == -1) {
            continue;
        }
#if 1
        if(mask & /*SOCKET_READABLE_BIT*/0x1) {
            while(ipaugenblick_receive(ready_socket,&rxbuff,&len) == 0) {
                received_count++;
                if(!(received_count%1000)) {
                    printf("received %u\n",received_count);
                }
                ipaugenblick_release_rx_buffer(rxbuff);
            }
        }
#if USE_TX
        if(mask & /*SOCKET_WRITABLE_BIT*/0x2) {
            buff = ipaugenblick_get_buffer(1448);
            if(buff) {
                for(i = 0;i < 100;i++) {
                    if(ipaugenblick_send(ready_socket,buff,0,1448)) { 
                        ipaugenblick_release_tx_buffer(buff);
                        ipaugenblick_socket_kick(ready_socket);
                        break;
                    }
                }
            }
            else {
                ipaugenblick_socket_kick(ready_socket);
            } 
        } 
#endif
#endif
    }
    return 0;
}
