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

#if 0
int main(int argc,char **argv)
{
    void *buff;
    int sock,len;
    ipaugenblick_cmd_t *cmd;
    char *p;
    int size = 0,ringset_idx;
    int socket_connected = -1;

    if(ipaugenblick_app_init(argc,argv) != 0) {
        printf("cannot initialize memory\n");
        return 0;
    } 
    if((sock = ipaugenblick_open_tcp_client(inet_addr("192.168.150.62"),7777,inet_addr("192.168.150.63"),7777)) < 0) {
        printf("cannot open tcp client socket\n");
        return 0;
    }
    while(1) {
        if(socket_connected == -1) {
            socket_connected = ipaugenblick_get_connected();
        }
        if(socket_connected == -1) {
            continue;
        }
#if 0
        buff = ipaugenblick_get_buffer(1024);
        if(buff) {
         printf("sending...\n");
            if(ipaugenblick_send(sock,buff,0,1024)) { 
                printf("failed\n");
                ipaugenblick_release_tx_buffer(buff);
                ipaugenblick_socket_kick(socket_descr);
            } 
        }
        else {
//            ipaugenblick_socket_kick(socket_descr);
        }
#endif
#if 1
        if(ipaugenblick_receive(sock,&buff,&len) == 0) {
//            printf("received %p\n",buff);
            ipaugenblick_release_rx_buffer(buff);
#if 1
            buff = ipaugenblick_get_buffer(1024);
            if(buff) {
             printf("sending...\n");
                if(ipaugenblick_send(sock,buff,0,1024)) { 
                    printf("failed\n");
                    ipaugenblick_release_tx_buffer(buff);
                }
            }
            ipaugenblick_socket_kick(sock);
#endif
        }
#endif
    }
    return 0;
}
#else
int main(int argc,char **argv)
{
    void *buff,**rxbuff;
    int sock,len;
    ipaugenblick_cmd_t *cmd;
    char *p;
    int size = 0,ringset_idx;
    int socket_connected = -1;
    unsigned int iterations = 0,iterations2 = 0;
    int i;

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
    while(1) {
        if(socket_connected == -1) {
            socket_connected = ipaugenblick_accept(sock);
            if(socket_connected != -1)
                printf("socket accepted\n");
        }
        if(socket_connected == -1) {
            continue;
        }
#if 1
        buff = ipaugenblick_get_buffer(1024);
        if(buff) {
            if(ipaugenblick_send(socket_connected,buff,0,1024)) { 
                ipaugenblick_release_tx_buffer(buff);
            } 
        }
        iterations++;
        if(iterations == 1000000) {
            ipaugenblick_socket_kick(socket_connected);
            iterations = 0;
        }
#endif
#if 1
        if(ipaugenblick_receive(socket_connected,&rxbuff,&len) == 0) {
            if(++iterations2 == 1) {
                iterations2 = 0;
            //    printf("received %p %d\n",*rxbuff,len);
#if 0
                for(i = 0;i < len;i++) {
                    if(!(i%8))
                        printf("\n");
                    printf(" %x",((char *)*rxbuff)[i]);
                }
#endif
            }
            ipaugenblick_release_rx_buffer(rxbuff);
#if 0
            buff = ipaugenblick_get_buffer(1024);
            if(buff) {
             printf("sending...\n");
                if(ipaugenblick_send(sock,buff,0,1024)) { 
                    printf("failed\n");
                    ipaugenblick_release_tx_buffer(buff);
                }
            }
            ipaugenblick_socket_kick(sock);
#endif
        }
#endif
    }
    return 0;
}
#endif
