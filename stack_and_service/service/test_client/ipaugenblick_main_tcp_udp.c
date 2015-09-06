#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ipaugenblick_api.h>
#include <string.h>

#define USE_TX 1
#define USE_CONNECTED 1

int main(int argc,char **argv)
{
    void *txbuff,*rxbuff,*pdesc;
    int sock,newsock,len,listener_sock;
    char *p;
    int size = 0,ringset_idx;
    int sockets_connected = 0;
    int selector;
    int ready_socket;
    int i,tx_space;
    int max_total_length = 0;
    unsigned short mask,from_port;
    unsigned int from_ip;
    unsigned long received_count = 0;
    struct sockaddr addr;
    struct sockaddr_in *in_addr = (struct sockaddr_in *)&addr;
    int addrlen = sizeof(*in_addr);

    if(ipaugenblick_app_init(argc,argv,"tcp_udp") != 0) {
        printf("cannot initialize memory\n");
        return 0;
    } 
    printf("memory initialized\n");
    selector = ipaugenblick_open_select();
    if(selector != -1) {
        printf("selector opened\n");
    }
    if((listener_sock = ipaugenblick_open_socket(AF_INET, SOCK_STREAM, selector)) < 0) {
        printf("cannot open tcp client socket\n");
        return 0;
    }
    in_addr->sin_family = AF_INET;
    in_addr->sin_addr.s_addr = inet_addr("192.168.150.63");
    in_addr->sin_port = htons(7777);
    ipaugenblick_bind(listener_sock, &addr, addrlen);
    int bufsize = 1024*1024*1000;
    ipaugenblick_setsockopt(listener_sock, SOL_SOCKET,SO_SNDBUFFORCE,(char *)&bufsize,sizeof(bufsize));
    ipaugenblick_setsockopt(listener_sock, SOL_SOCKET,SO_RCVBUFFORCE,(char *)&bufsize,sizeof(bufsize));
    ipaugenblick_fdset (listener_sock, &readfdset);
    ipaugenblick_listen_socket(listener_sock);
    printf("listener socket opened\n");
    
    for(i = 0;i < 10;i++) {
        if((sock = ipaugenblick_open_socket(AF_INET, SOCK_DGRAM, selector)) < 0) {
            printf("cannot open UDP socket\n");
            return 0;
        }
	in_addr->sin_family = AF_INET;
	in_addr->sin_addr.s_addr = inet_addr("192.168.150.63");
	in_addr->sin_port = htons(7777+i);
        ipaugenblick_bind(sock,&addr,addrlen);
        ipaugenblick_setsockopt(sock, SOL_SOCKET,SO_SNDBUFFORCE,(char *)&bufsize,sizeof(bufsize));
        ipaugenblick_setsockopt(sock, SOL_SOCKET,SO_RCVBUFFORCE,(char *)&bufsize,sizeof(bufsize));    
#if USE_CONNECTED
        printf("connecting %d\n",sock);
	from_ip = inet_addr("192.168.150.62");
        from_port = htons(7777);
	in_addr->sin_addr.s_addr = from_ip;
	in_addr->sin_port = from_port
        ipaugenblick_connect(sock,&addr,addrlen);
	ipaugenblick_fdset (sock, &readfdset);
	ipaugenblick_fdset (sock, &writefdset);
#endif
    } 

    while(1) {  
        ready_socket = ipaugenblick_select(selector,&readfdset,&writefdset,&excfdset,NULL);
        if(ready_socket == -1) {
            continue;
        }
        
        for (ready_socket = 0; ready_socket < readfdset.returned_idx; ready_socket++) {
	    if (!ipaugenblick_fdisset(ready_socket,&readfdset))
		continue;
	    if(ready_socket == listener_sock) { 
            	newsock = ipaugenblick_accept(listener_sock,&addr,&addrlen);
            	if(newsock != -1) {
                	printf("socket accepted %d %d\n",newsock,selector);
	                ipaugenblick_set_socket_select(newsock,selector);
        	        sockets_connected++;
            	}
            	continue;
       	    }
	    int first_seg_len = 0;
            while(ipaugenblick_receive(ready_socket,&rxbuff,&len,&first_seg_len,&pdesc) == 0) {
                received_count++;
                if(len > max_total_length)
                    max_total_length = len;
                void *porigdesc = pdesc;
                do { 
                    /* do something */
                    /* don't release buf, release rxbuff */
		    rxbuff = ipaugenblick_get_next_buffer_segment(&pdesc,&len);
                }while(rxbuff);
                if(!(received_count%100000)) {
                    printf("received %u max_total_len %u\n",received_count,max_total_length);
                }
                ipaugenblick_release_rx_buffer(porigdesc,ready_socket);
            }
        }
#if USE_TX
        for (ready_socket = 0; ready_socket < writefdset.returned_idx; ready_socket++) {
	    if (!ipaugenblick_fdisset(ready_socket,&writefdset))
		continue;
            tx_space = ipaugenblick_get_socket_tx_space(ready_socket);
            for(i = 0;i < tx_space;i++) {
                txbuff = ipaugenblick_get_buffer(1448,ready_socket,&pdesc);
                if(!txbuff)
                    break;
                if(ipaugenblick_send(ready_socket,pdesc,0,1448)) { 
                    ipaugenblick_release_tx_buffer(pdesc);
                    break;
                }
            } 
            ipaugenblick_socket_kick(ready_socket);
        }  
#endif
    }
    return 0;
}
