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

#define USE_CONNECTED 1
#define DATAGRAM_SIZE 60

int main(int argc,char **argv)
{
    void *buff,*buff_seg;
    int sock,selector,len,ready_socket,i,tx_space;
    char *p;
    int size = 0,ringset_idx,nb_segs,max_nb_segs = 0,max_total_len = 0,seg_idx;
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
        if((sock = ipaugenblick_open_udp(inet_addr("192.168.150.63"),rand())) < 0) {
            printf("cannot open UDP socket\n");
            return 0;
        }
        ipaugenblick_set_socket_select(sock,selector);
        from_ip = inet_addr("192.168.150.62");
        from_port = htons(7777);
        printf("waiting for creation feedback %d\n",sock);
        sleep(1);
#if USE_CONNECTED
        printf("connecting %d\n",sock);
        ipaugenblick_socket_connect(sock,from_ip,from_port);
#endif
    }
    while(1) {
        ready_socket = ipaugenblick_select(selector,&mask,10000);
        if(ready_socket == -1) {
            continue;
        }
        if(mask & /*SOCKET_READABLE_BIT*/0x1) {
            if(ipaugenblick_receivefrom(ready_socket,&buff,&len,&nb_segs,&from_ip,&from_port) == 0) {
                received_packets++;
                if(nb_segs > max_nb_segs)
                    max_nb_segs = nb_segs;
                if(len > max_total_len)
                    max_total_len = len;
                buff_seg = buff;
                for(seg_idx = 0;seg_idx < nb_segs;seg_idx++) {

                    buff_seg = ipaugenblick_get_next_buffer_segment(buff_seg,&len);
                    if((buff_seg)&&(len > 0)) {
                        /* do something */
                        /* don't release buf, release rxbuff */
                    }
                }
                if(!(received_packets%1000000)) {
                    printf("received %u max_nb_segs %u max_total_len %u\n",received_packets,max_nb_segs,max_total_len);
                }
                ipaugenblick_release_rx_buffer(buff);
           }
        }
        if(mask & /*SOCKET_WRITABLE_BIT*/0x2) {
            tx_space = ipaugenblick_get_socket_tx_space(ready_socket);
            for(i = 0;i < tx_space;i++) {
                buff = ipaugenblick_get_buffer(DATAGRAM_SIZE,ready_socket);
                if(buff) {
#if USE_CONNECTED
                    if(ipaugenblick_send(ready_socket,buff,0,DATAGRAM_SIZE)) { 
                        ipaugenblick_release_tx_buffer(buff);
                    }
#else
                    if(ipaugenblick_sendto(ready_socket,buff,0,DATAGRAM_SIZE,inet_addr("192.168.150.62"),7777)) { 
                        ipaugenblick_release_tx_buffer(buff);
                    } 
#endif
                } 
            }  
            ipaugenblick_socket_kick(sock);
        } 
   }
    return 0;
}

