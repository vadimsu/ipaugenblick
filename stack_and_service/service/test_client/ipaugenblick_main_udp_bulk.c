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

#define USE_CONNECTED 0
#define DATAGRAM_SIZE 60

int main(int argc,char **argv)
{
    void *txbuff,*rxbuff,*pdesc;
    int sock,selector,len,ready_socket,i,tx_space,sent;
    char *p;
    int size = 0,ringset_idx,max_total_len = 0,seg_idx;
    unsigned int from_ip;
    unsigned short from_port,mask;
    unsigned long received_packets = 0;
    unsigned long  sent_packets = 0;

    srand (time(NULL));

    if(ipaugenblick_app_init(argc,argv,"udp_bulk") != 0) {
        printf("cannot initialize memory\n");
        return 0;
    } 
    selector = ipaugenblick_open_select();
    if(selector != -1) {
        printf("selector opened %d\n",selector);
    }
    for(i = 0;i < 1;i++) {
        if((sock = ipaugenblick_open_socket(AF_INET,SOCK_DGRAM,selector)) < 0) {
        	printf("cannot open udp socket\n");
        	return 0;
    	}
    	
#if USE_CONNECTED
        printf("connecting %d\n",sock);
        ipaugenblick_v4_connect_bind_socket(sock,inet_addr("192.168.150.62"),htons(7777),1);
#endif
    }
    printf("entering main loop\n");
    while(1) {	
        ready_socket = ipaugenblick_select(selector,&mask, NULL);
        if(ready_socket == -1) {
            continue;
        }
        if(mask & /*SOCKET_READABLE_BIT*/0x1) {
printf("%s %d\n",__FILE__,__LINE__);
            if(ipaugenblick_receivefrom(ready_socket,&rxbuff,&len,&from_ip,&from_port,&pdesc) == 0) {
                received_packets++;
                if(len > max_total_len)
                    max_total_len = len;
                void *porigdesc = pdesc;
                while(rxbuff) { 
                    if(len > 0) {
                        /* do something */
                        /* don't release buf, release rxbuff */
                    }
		    rxbuff = ipaugenblick_get_next_buffer_segment(&pdesc,&len);
                }
                if(!(received_packets%1000000)) {
                    printf("received %u max_total_len %u\n",received_packets,max_total_len);
                }
                ipaugenblick_release_rx_buffer(porigdesc,ready_socket);
           }
        }
        if(mask & /*SOCKET_WRITABLE_BIT*/0x2) {
            sent = 0;
            tx_space = ipaugenblick_get_socket_tx_space(ready_socket);
//	    if(tx_space > 10) tx_space = 10;
            if(tx_space > 0) {
                struct data_and_descriptor bufs_and_desc[tx_space];
                int offsets[tx_space];
                int lengths[tx_space];
                if(!ipaugenblick_get_buffers_bulk(DATAGRAM_SIZE,ready_socket,tx_space,bufs_and_desc)) { 
#if USE_CONNECTED
                    for(i = 0;i < tx_space;i++) {
                        offsets[i] = 0;
                        lengths[i] = DATAGRAM_SIZE;
                    }
                    if(ipaugenblick_send_bulk(ready_socket,bufs_and_desc,offsets,lengths,tx_space)) {
			for(i = 0;i < tx_space;i++)
                        	ipaugenblick_release_tx_buffer(bufs_and_desc[i].pdesc);
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
                    if(ipaugenblick_sendto_bulk(ready_socket,bufs_and_desc,offsets,lengths,ipaddresses,ports,tx_space)) {
			for(i = 0;i < tx_space;i++)
                        	ipaugenblick_release_tx_buffer(bufs_and_desc[i].pdesc);
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

