#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <ipaugenblick_api.h>
#include <string.h>

#define USE_CONNECTED 1
#define DATAGRAM_SIZE 60

int main(int argc,char **argv)
{
    void *txbuff,*rxbuff,*pdesc;
    int sock,selector,len,ready_socket,i,tx_space;
    char *p;
    int size = 0,ringset_idx,max_total_len = 0,seg_idx;
    unsigned int from_ip;
    unsigned short from_port,mask;
    unsigned long received_packets = 0;
    unsigned long  sent_packets = 0;

    if(ipaugenblick_app_init(argc,argv,"udp") != 0) {
        printf("cannot initialize memory\n");
        return 0;
    } 
    selector = ipaugenblick_open_select();
    if(selector != -1) {
        printf("selector opened\n");
    }
    for(i = 0;i < 10;i++) {
        if((sock = ipaugenblick_open_socket(AF_INET, SOCK_DGRAM, selector)) < 0) {
            printf("cannot open UDP socket\n");
            return 0;
        }
        ipaugenblick_v4_connect_bind_socket(sock,inet_addr("192.168.150.63"),rand(),0);
#if USE_CONNECTED
	from_ip = inet_addr("192.168.150.62");
        from_port = htons(7777);
        printf("connecting %d\n",sock);
        ipaugenblick_v4_connect_bind_socket(sock,from_ip,from_port,1);
#endif
    }
    while(1) {
        ready_socket = ipaugenblick_select(selector,&mask,NULL);
        if(ready_socket == -1) {
            continue;
        }
        if(mask & /*SOCKET_READABLE_BIT*/0x1) {
            if(ipaugenblick_receivefrom(ready_socket,&rxbuff,&len,&from_ip,&from_port,&pdesc) == 0) {
                received_packets++;
                if(len > max_total_len)
                    max_total_len = len;
                void *porigdesc = pdesc;
                do { 
                    /* do something */
                    /* don't release buf, release rxbuff */
		    rxbuff = ipaugenblick_get_next_buffer_segment(&pdesc,&len);
                }while(rxbuff);
                if(!(received_packets%1000000)) {
                    printf("received %u max_total_len %u\n",received_packets,max_total_len);
                }
                ipaugenblick_release_rx_buffer(porigdesc,ready_socket);
           }
        }
        if(mask & /*SOCKET_WRITABLE_BIT*/0x2) {
            tx_space = ipaugenblick_get_socket_tx_space(ready_socket);
            for(i = 0;i < tx_space;i++) {
                txbuff = ipaugenblick_get_buffer(DATAGRAM_SIZE,ready_socket,&pdesc);
                if(txbuff) {
#if USE_CONNECTED
                    if(ipaugenblick_send(ready_socket,pdesc,0,DATAGRAM_SIZE)) { 
                        ipaugenblick_release_tx_buffer(pdesc);
                    }
#else
                    if(ipaugenblick_sendto(ready_socket,pdesc,0,DATAGRAM_SIZE,inet_addr("192.168.150.62"),7777)) { 
                        ipaugenblick_release_tx_buffer(pdesc);
                    } 
#endif
                } 
            }  
            ipaugenblick_socket_kick(sock);
        } 
   }
    return 0;
}

