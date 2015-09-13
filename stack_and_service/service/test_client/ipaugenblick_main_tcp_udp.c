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
#include <getopt.h>

#define USE_CONNECTED 0

int dgram_socks[1024];
int stream_socks[1024];
int dgram_socks_count = 0;
int stream_socks_count = 0;

static int is_dgram_sock(int sock)
{
	int idx;
	for(idx = 0; idx < dgram_socks_count; idx++)
		if (dgram_socks[idx] == sock)
			return 1;
	return 0;
}

static int is_stream_sock(int sock)
{
	int idx;
	for(idx = 0; idx < stream_socks_count; idx++)
		if (stream_socks[idx] == sock)
			return 1;
	return 0;
}

int main(int argc,char **argv)
{
    void *txbuff,*rxbuff,*pdesc;
    int sock,newsock,len,listener_sock;
    int sockets_connected = 0;
    int selector;
    int ready_socket;
    int i,tx_space;
    int max_total_length = 0;
    unsigned long received_packets = 0,transmitted_packets = 0;
    char my_ip_addr[20],ip_addr_2_connect[20];
    int port_to_bind, port_to_connect; 
    struct sockaddr addr;
    struct sockaddr_in *in_addr = (struct sockaddr_in *)&addr;
    int addrlen = sizeof(*in_addr);
    struct ipaugenblick_fdset readfdset,writefdset, excfdset;
    int rxtxmask = 0x3; /* both */
    static struct option long_options[] =
        {
          {"ip2bind",  required_argument, 0, 'a'},
          {"ip2connect",  required_argument, 0, 'b'},
          {"port2bind",    required_argument, 0, 'c'},
          {"port2connect",    required_argument, 0, 'd'},
          {"notransmit", no_argument, 0, 'e' },
	  {"noreceive", no_argument, 0, 'f' },
          {0, 0, 0, 0}
        };
    int opt_idx,c;

    memset(dgram_socks,-1,sizeof(dgram_socks));
    memset(stream_socks,-1,sizeof(stream_socks));
    strcpy(my_ip_addr,"192.168.150.63");
    strcpy(ip_addr_2_connect,"192.168.150.63");
    port_to_bind = 7777;
    port_to_connect = 8888;

   // ipaugenblick_set_log_level(0/*IPAUGENBLICK_LOG_DEBUG*/);

    ipaugenblick_fdzero(&readfdset);
    ipaugenblick_fdzero(&writefdset);
    ipaugenblick_fdzero(&excfdset);

    if(ipaugenblick_app_init(argc,argv,"tcp_udp") != 0) {
        printf("cannot initialize memory\n");
        return 0;
    } 
    printf("memory initialized\n");
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
	case 'e':
		rxtxmask &= ~0x2;
		break;
	case 'f':
		rxtxmask &= ~0x1;
		break;
        default:
                printf("undefined option %c\n",c);
        }
    }
    selector = ipaugenblick_open_select();
    if(selector != -1) {
        printf("selector opened\n");
    }
    if((listener_sock = ipaugenblick_open_socket(AF_INET, SOCK_STREAM, selector)) < 0) {
        printf("cannot open tcp client socket\n");
        return 0;
    }
    in_addr->sin_family = AF_INET;
    in_addr->sin_addr.s_addr = inet_addr(my_ip_addr);
    in_addr->sin_port = port_to_bind;
    ipaugenblick_bind(listener_sock, &addr, addrlen);
    int bufsize = 1024*1024*1000;
    ipaugenblick_setsockopt(listener_sock, SOL_SOCKET,SO_SNDBUFFORCE,(char *)&bufsize,sizeof(bufsize));
    ipaugenblick_setsockopt(listener_sock, SOL_SOCKET,SO_RCVBUFFORCE,(char *)&bufsize,sizeof(bufsize));
    ipaugenblick_fdset (listener_sock, &readfdset);
    ipaugenblick_listen_socket(listener_sock);
    printf("listener socket opened\n");
    for(i = 0;i < 1;i++) {
        if((sock = ipaugenblick_open_socket(AF_INET, SOCK_DGRAM, selector)) < 0) {
            printf("cannot open UDP socket\n");
            return 0;
        }
	in_addr->sin_family = AF_INET;
	in_addr->sin_addr.s_addr = inet_addr(my_ip_addr);
	in_addr->sin_port = htons(port_to_bind+i+1);
        ipaugenblick_bind(sock,&addr,addrlen);
//        ipaugenblick_setsockopt(sock, SOL_SOCKET,SO_SNDBUFFORCE,(char *)&bufsize,sizeof(bufsize));
  //      ipaugenblick_setsockopt(sock, SOL_SOCKET,SO_RCVBUFFORCE,(char *)&bufsize,sizeof(bufsize));
	dgram_socks[dgram_socks_count++] = sock;
	in_addr->sin_addr.s_addr = inet_addr(ip_addr_2_connect);
	in_addr->sin_port = htons(port_to_connect);
	if (rxtxmask & 0x1)
		ipaugenblick_fdset (sock, &readfdset);
	if (rxtxmask & 0x2)
		ipaugenblick_fdset (sock, &writefdset);
#if USE_CONNECTED
        printf("connecting %d\n",sock);	
        ipaugenblick_connect(sock,&addr,addrlen);	
#endif
    } 

    while(1) {  
        ready_socket = ipaugenblick_select(selector,&readfdset,&writefdset,&excfdset,NULL);
        if(ready_socket == -1) {
            continue;
        }
        
        for (ready_socket = 0; ready_socket < readfdset.returned_idx; ready_socket++) {
	    if (!ipaugenblick_fdisset(ipaugenblick_fd_idx2sock(&readfdset,sock),&readfdset))
                        continue;
            if(ipaugenblick_fd_idx2sock(&readfdset,ready_socket) == listener_sock) {
            	newsock = ipaugenblick_accept(listener_sock,&addr,&addrlen);
            	if(newsock != -1) {
                	printf("socket accepted %d %d\n",newsock,selector);
	                ipaugenblick_set_socket_select(newsock,selector);
        	        sockets_connected++;
			stream_socks[stream_socks_count++] = newsock;
            	}
            	continue;
       	    }
	    if (is_dgram_sock(ipaugenblick_fd_idx2sock(&readfdset,ready_socket))) {
		len = 1448;
		while(ipaugenblick_receivefrom(ipaugenblick_fd_idx2sock(&readfdset,ready_socket),&rxbuff,&len,&addr,&addrlen, &pdesc) == 0) {
			received_packets++;
			if(!(received_packets%1000)) {
	                    printf("received %u\n",received_packets);
        	        }
			ipaugenblick_release_rx_buffer(pdesc, ipaugenblick_fd_idx2sock(&readfdset,ready_socket));
			len = 1448;
		}
	    } else {
	    	int first_seg_len = 0;
            	while(ipaugenblick_receive(ipaugenblick_fd_idx2sock(&readfdset,ready_socket),&rxbuff,&len,&first_seg_len,&pdesc) == 0) {
                	received_packets++;
	                if(len > max_total_length)
        	            max_total_length = len;
                	void *porigdesc = pdesc;
	                do { 
        	            /* do something */
                	    /* don't release buf, release rxbuff */
			    rxbuff = ipaugenblick_get_next_buffer_segment(&pdesc,&len);
        	        }while(rxbuff);
                	if(!(received_packets%100000)) {
	                    printf("received %u max_total_len %u\n",received_packets,max_total_length);
        	        }
                	ipaugenblick_release_rx_buffer(porigdesc, ipaugenblick_fd_idx2sock(&readfdset,ready_socket));
            	}
	    }
        }
        for (ready_socket = 0; ready_socket < writefdset.returned_idx; ready_socket++) {
	    if (!ipaugenblick_fdisset(ipaugenblick_fd_idx2sock(&writefdset,ready_socket),&writefdset))
		continue;
            tx_space = ipaugenblick_get_socket_tx_space(ipaugenblick_fd_idx2sock(&writefdset,ready_socket));
            for(i = 0;i < tx_space;i++) {
                txbuff = ipaugenblick_get_buffer(1448,ipaugenblick_fd_idx2sock(&writefdset,ready_socket),&pdesc);
                if(!txbuff)
                    break;
		if (is_dgram_sock(ipaugenblick_fd_idx2sock(&writefdset,ready_socket))) {
			in_addr->sin_family = AF_INET;
			in_addr->sin_addr.s_addr = inet_addr(ip_addr_2_connect);
			in_addr->sin_port = /*htons(port_to_connect)*/port_to_connect;
			if(ipaugenblick_sendto(ipaugenblick_fd_idx2sock(&writefdset,ready_socket),pdesc,0,1448, &addr, addrlen)) {
				ipaugenblick_release_tx_buffer(pdesc);
	                    	break;
			} else
				transmitted_packets++;
		} else {
             	   if(ipaugenblick_send(ipaugenblick_fd_idx2sock(&writefdset,ready_socket),pdesc,0,1448)) { 
                	    ipaugenblick_release_tx_buffer(pdesc);
	                    break;
        	   } else
			transmitted_packets++;
		}
            }
	    if(tx_space == 0)
                continue;
	    if(!(transmitted_packets%1000)) {
                    printf("received %u transmitted_packets %u\n",received_packets,transmitted_packets);
                    print_stats();
            }
            int iter = 0;
            while(ipaugenblick_socket_kick(ipaugenblick_fd_idx2sock(&writefdset,ready_socket)) == -1) {
                iter++;
                if(!(iter%1000000)) {
                        printf("iter!\n");exit(0);
                }
            }
        }  
    }
    return 0;
}
