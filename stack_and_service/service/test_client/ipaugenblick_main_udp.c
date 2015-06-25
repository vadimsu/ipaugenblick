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
#include <getopt.h>

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
    unsigned long received_packets = 0,transmitted_packets = 0;
    unsigned long  sent_packets = 0;
    char my_ip_addr[20],ip_addr_2_connect[20];
    int port_to_bind, port_to_connect;

    if(ipaugenblick_app_init(argc,argv,"udp") != 0) {
        printf("cannot initialize memory\n");
        return 0;
    }
    static struct option long_options[] =
        {
          {"ip2bind",  required_argument, 0, 'a'},
          {"ip2connect",  required_argument, 0, 'b'},
          {"port2bind",    required_argument, 0, 'c'},
	  {"port2connect",    required_argument, 0, 'd'},
          {0, 0, 0, 0}
        };
    int opt_idx,c;

    strcpy(my_ip_addr,"192.168.150.63");
    strcpy(ip_addr_2_connect,"192.168.150.62");
    port_to_bind = 7777;
    port_to_connect = 7777;
    
    while((c = getopt_long_only(argc,argv,"",long_options,&opt_idx)) != -1) {
	switch(c) {
	case 'a':
		strcpy(my_ip_addr,optarg);
		printf("IP address to bind: %s\n",my_ip_addr);
		break;
	case 'b':
		strcpy(ip_addr_2_connect,optarg);
		printf("IP address to connect: %s\n",ip_addr_2_connect);
		break;
	case 'c':
		port_to_bind = atoi(optarg);
		printf("Port to bind %d\n",port_to_bind);
		break;
	case 'd':
		port_to_connect = atoi(optarg);
		printf("Port to connect %d\n",port_to_connect);
		break;
	default:
		printf("undefined option %c\n",c);
	}
    }
    
    selector = ipaugenblick_open_select();
    if(selector != -1) {
        printf("selector opened\n");
    }
    for(i = 0;i < 1;i++) {
        if((sock = ipaugenblick_open_socket(AF_INET, SOCK_DGRAM, selector)) < 0) {
            printf("cannot open UDP socket\n");
            return 0;
        }
        ipaugenblick_v4_connect_bind_socket(sock,inet_addr(my_ip_addr),port_to_bind,0);
#if USE_CONNECTED
	from_ip = inet_addr(ip_addr_2_connect);
        from_port = htons(port_to_connect);
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
            while(ipaugenblick_receivefrom(ready_socket,&rxbuff,&len,&from_ip,&from_port,&pdesc) == 0) {
                received_packets++;
                if(len > max_total_len)
                    max_total_len = len;
                void *porigdesc = pdesc;
		int segs =0;
                do { 
                    /* do something */
                    /* don't release buf, release rxbuff */
		    segs++;
		    rxbuff = ipaugenblick_get_next_buffer_segment(&pdesc,&len);
                }while(rxbuff);
		received_packets += segs;
                if(!(received_packets%100000)) {
                    printf("received %u transmitted_packets %u\n",received_packets,transmitted_packets);
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
		    else
			transmitted_packets++;
#else
                    if(ipaugenblick_sendto(ready_socket,pdesc,0,DATAGRAM_SIZE,inet_addr(ip_addr_2_connect),port2connect)) { 
                        ipaugenblick_release_tx_buffer(pdesc);
                    }
		    else
			transmitted_packets++;
#endif
                } 
            }
	    if(tx_space == 0)
		continue;
	    if(!(transmitted_packets%100000)) {
                    printf("received %u transmitted_packets %u\n",received_packets,transmitted_packets);
                }
            ipaugenblick_socket_kick(sock);
        } 
   }
    return 0;
}

