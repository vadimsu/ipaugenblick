#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/errno.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include "../ipaugenblick_app_api/ipaugenblick_api.h"
#include <string.h>

#define USE_TX 1

int main(int argc,char **argv)
{
    void *buff,*rxbuff;
    int sock,newsock,len;
    char *p;
    int size = 0,ringset_idx;
    int sockets_connected = 0;
    int selector;
    int ready_socket;
    int i,tx_space,nb_segs,seg_idx;
    int max_nb_segs = 0;
    int max_total_length = 0;
    unsigned short mask;
    unsigned long received_count = 0;

    if(ipaugenblick_app_init(argc,argv) != 0) {
        printf("cannot initialize memory\n");
        return 0;
    } 
    printf("memory initialized\n");
    selector = ipaugenblick_open_select();
    if(selector != -1) {
        printf("selector opened\n");
    }

    if((sock = ipaugenblick_open_socket(AF_INET,SOCK_STREAM,selector)) < 0) {
        printf("cannot open tcp client socket\n");
        return 0;
    }
    ipaugenblick_v4_connect_bind_socket(sock,inet_addr("192.168.150.62"),htons(7777),1);

    //    ipaugenblick_set_socket_select(sock,selector);
    while(1) {  
        ready_socket = ipaugenblick_select(selector,&mask,10000);
        if(ready_socket == -1) {
            continue;
        }
        
        if(mask & /*SOCKET_READABLE_BIT*/0x1) {
	    int first_seg_len = 0;
            while(ipaugenblick_receive(ready_socket,&rxbuff,&len,&first_seg_len) == 0) {
                received_count++;
                
                buff = rxbuff;

#if 0
                while(buff) {
                    
                    buff = ipaugenblick_get_next_buffer_segment(buff,&len);
                    if((buff)&&(len > 0)) {
                        /* do something */
                        /* don't release buf, release rxbuff */
                    }
                }
#endif
                if(!(received_count%10000000)) {
                    printf("received %u max_nb_segs %u max_total_len %u\n",received_count,max_nb_segs,max_total_length);
                }
                ipaugenblick_release_rx_buffer(rxbuff,ready_socket);
            }
        }
#if USE_TX
        if(mask & /*SOCKET_WRITABLE_BIT*/0x2) {
            tx_space = ipaugenblick_get_socket_tx_space(ready_socket);
            for(i = 0;i < tx_space;i++) {
                buff = ipaugenblick_get_buffer(1448,ready_socket);
                if(!buff) {
                    break;
                }
                if(ipaugenblick_send(ready_socket,buff,0,1448)) { 
                    ipaugenblick_release_tx_buffer(buff);
                    break;
                }
            } 
            ipaugenblick_socket_kick(ready_socket);
        }  
//        ipaugenblick_socket_kick(ready_socket);
#endif
    }
    return 0;
}
