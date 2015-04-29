#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include "../ipaugenblick_app_api/ipaugenblick_api.h"
#include <string.h>

#define USE_TX 1

int main(int argc,char **argv)
{
    void *txbuff,*rxbuff,*pdesc;
    int sock,len;
    char *p;
    int size = 0,ringset_idx,tx_space = 0;
    int socket_connected = -1;
    int i;

    if(ipaugenblick_app_init(argc,argv,"tcp_listener") != 0) {
        printf("cannot initialize memory\n");
        return 0;
    } 
    printf("memory initialized\n");
    if((sock = ipaugenblick_open_socket(AF_INET,SOCK_STREAM,-1)) < 0) {
        printf("cannot open tcp client socket\n");
        return 0;
    }
    ipaugenblick_v4_connect_bind_socket(sock,inet_addr("192.168.150.63"),7777,0);

    ipaugenblick_listen_socket(sock);
    printf("listener socket opened\n");
    while(1) {
        if(socket_connected == -1) {
	    unsigned int ipaddr;
            unsigned short port;
            socket_connected = ipaugenblick_accept(sock,&ipaddr,&port);
            if(socket_connected != -1)
                printf("socket accepted\n");
        }
        if(socket_connected == -1) {
            continue;
        }
	int first_seg_len = 0;
        if(ipaugenblick_receive(socket_connected,&rxbuff,&len,&first_seg_len,&pdesc) == 0) {
            ipaugenblick_release_rx_buffer(pdesc,socket_connected);
        }
#if USE_TX
        tx_space = ipaugenblick_get_socket_tx_space(socket_connected);
#if 0
        for(i = 0;i < tx_space;i++) {
            txbuff = ipaugenblick_get_buffer(1448,socket_connected,&pdesc);
            if(!txbuff) {
                break;
            }
            if(ipaugenblick_send(socket_connected,pdesc,0,1448)) {
                ipaugenblick_release_tx_buffer(pdesc);
                break;
            }
        }
#else
        if(tx_space == 0) {
	    usleep(1);
            continue;
	}
        struct data_and_descriptor bulk_bufs[tx_space];
        int offsets[tx_space];
        int lengths[tx_space];
        if(!ipaugenblick_get_buffers_bulk(1448,socket_connected,tx_space,bulk_bufs)) {
                for(i = 0;i < tx_space;i++) {
                    offsets[i] = 0;
                    lengths[i] = 1448;
                }
                if(ipaugenblick_send_bulk(socket_connected,bulk_bufs,offsets,lengths,tx_space)) {
		    for(i = 0;i < tx_space;i++)
                    	ipaugenblick_release_tx_buffer(bulk_bufs[i].pdesc);
                    printf("%s %d\n",__FILE__,__LINE__);
                }
                else {
//                  printf("%s %d %d\n",__FILE__,__LINE__,tx_space);
//                  sent = 1;

//		    ipaugenblick_socket_kick(socket_connected);
                }
        }
#endif
            ipaugenblick_socket_kick(socket_connected);
#endif
    }
    return 0;
}
