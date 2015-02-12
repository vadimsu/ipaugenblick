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

int main(int argc,char **argv)
{
    void **buff;
    int sock,selector,len,ready_socket,i,tx_space;
    char *p;
    int size = 0,ringset_idx;
    unsigned int from_ip;
    unsigned short from_port,mask;
    unsigned long received_packets = 0;
    unsigned long  sent_packets = 0;

    if(ipaugenblick_app_init(argc,argv) != 0) {
        printf("cannot initialize memory\n");
        return 0;
    } 
    selector = ipaugenblick_open_select();
    if(selector != -1) {
        printf("selector opened\n");
    }
    for(i = 0;i < 10;i++) {
        if((sock = ipaugenblick_open_udp(inet_addr("192.168.150.63"),7777+i)) < 0) {
            printf("cannot open UDP socket\n");
            return 0;
        }
        ipaugenblick_set_socket_select(sock,selector);
    }
    while(1) {
        ready_socket = ipaugenblick_select(selector,&mask);
        if(ready_socket == -1) {
            continue;
        }
        if(mask & /*SOCKET_READABLE_BIT*/0x1) {
            if(ipaugenblick_receivefrom(ready_socket,&buff,&len,&from_ip,&from_port) == 0) {
                received_packets++;
                if(!(received_packets%1000000)) {
                    printf("received %u\n",received_packets);
                }
                ipaugenblick_release_rx_buffer(buff);
           }
        }
        if(mask & /*SOCKET_WRITABLE_BIT*/0x2) {
            tx_space = ipaugenblick_get_socket_tx_space(ready_socket);
            for(i = 0;i < tx_space;i++) {
                buff = ipaugenblick_get_buffer(1448);
                if(buff) {
                    if(ipaugenblick_sendto(ready_socket,buff,0,1448,inet_addr("192.168.150.62"),7777)) { 
                        ipaugenblick_release_tx_buffer(buff);
                    } 
                } 
            } 
        }
        ipaugenblick_socket_kick(sock);
   }
    return 0;
}

