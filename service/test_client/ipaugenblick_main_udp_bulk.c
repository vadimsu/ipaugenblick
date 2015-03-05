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
    int sock,selector,len,ready_socket,i,tx_space,sent;
    char *p;
    int size = 0,ringset_idx,nb_segs,max_nb_segs = 0,max_total_len = 0,seg_idx;
    unsigned int from_ip;
    unsigned short from_port,mask;
    unsigned long received_packets = 0;
    unsigned long  sent_packets = 0;
    int select_timeout = atoi(argv[7]) - 1;
    printf("select_timeout = %d\n",select_timeout);

    srand (time(NULL));

    if(ipaugenblick_app_init(argc,argv) != 0) {
        printf("cannot initialize memory\n");
        return 0;
    } 
    selector = ipaugenblick_open_select();
    if(selector != -1) {
        printf("selector opened %d\n",selector);
    }
    for(i = 0;i < 1;i++) {
        if((sock = ipaugenblick_open_udp(inet_addr("192.168.150.63"),rand())) < 0) {
            printf("cannot open UDP socket %d\n",sock);
            return 0;
        }
        ipaugenblick_set_socket_select(sock,selector);
        from_ip = inet_addr("192.168.150.62");
        from_port = htons(7777);
        printf("waiting for creation feedback %d\n",sock);
        sleep(10);
#if USE_CONNECTED
        printf("connecting %d\n",sock);
        ipaugenblick_socket_connect(sock,from_ip,from_port);
#endif
    }
    printf("entering main loop\n");
    while(1) {
        ready_socket = ipaugenblick_select(selector,&mask,select_timeout);
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
            sent = 0;
            tx_space = ipaugenblick_get_socket_tx_space(ready_socket);
            if(tx_space > 0) {
                void *bufs[tx_space];
                int offsets[tx_space];
                int lengths[tx_space];
                if(!ipaugenblick_get_buffers_bulk(DATAGRAM_SIZE,ready_socket,tx_space,bufs)) { 
#if USE_CONNECTED
                    for(i = 0;i < tx_space;i++) {
                        offsets[i] = 0;
                        lengths[i] = DATAGRAM_SIZE;
                    }
                    if(ipaugenblick_send_bulk(ready_socket,bufs,offsets,lengths,tx_space)) { 
//                        ipaugenblick_release_tx_buffer(buff);
                        printf("%s %d\n",__FILE__,__LINE__);
                    }
                    else {
//                        printf("%s %d %d\n",__FILE__,__LINE__,tx_space);
                        sent = 1;
                    }
#else
                    unsigned short ports[tx_space];
                    unsigned int ipaddresses[tx_space];
                    for(i = 0;i < tx_space;i++) {
                        offsets[i] = 0;
                        lengths[i] = DATAGRAM_SIZE;
                        ipaddresses[i] = inet_addr("192.168.150.62");
                        ports[i] = 7777;
                    }
                    if(ipaugenblick_sendto_bulk(ready_socket,buff,offsets,lengths,ipaddresses,ports,tx_space)) { 
//                        ipaugenblick_release_tx_buffer(buff);
                    } 
#endif
                }
            }  
            i = ipaugenblick_socket_kick(sock);
            while(i != 0) {
                i = ipaugenblick_socket_kick(sock);
            }
        } 
   }
    return 0;
}

