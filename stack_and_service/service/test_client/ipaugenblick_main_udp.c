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

#define USE_CONNECTED 0
#define DATAGRAM_SIZE 1448

int main(int argc,char **argv)
{
    void *txbuff,*rxbuff,*pdesc;
    int sock,selector,len,ready_socket,i,tx_space;
    unsigned long received_packets = 0,transmitted_packets = 0;
    char my_ip_addr[20],ip_addr_2_connect[20];
    int port_to_bind, port_to_connect;
    struct timeval tm_out, *p_timeout = NULL;
    struct ipaugenblick_fdset readfdset,writefdset, excfdset;
    struct sockaddr addr;
    struct sockaddr_in *in_addr = (struct sockaddr_in *)&addr;
    int addrlen = sizeof(*in_addr);
    int rxtxmask = 0x3;
    int usebulk = 0;

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
	  {"notransmit", no_argument, 0, 'e' },
          {"noreceive", no_argument, 0, 'f' },
	  {"usebulk", no_argument, 0, 'g' },
          {0, 0, 0, 0}
        };
    int opt_idx,c;

    ipaugenblick_fdzero(&readfdset);
    ipaugenblick_fdzero(&writefdset);
    ipaugenblick_fdzero(&excfdset);

    strcpy(my_ip_addr,"192.168.150.63");
    strcpy(ip_addr_2_connect,"192.168.150.63");
    port_to_bind = 8888;
    port_to_connect = 7778;
    
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
	case 'g':
		usebulk = 1;
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
	in_addr->sin_family = AF_INET;
	in_addr->sin_addr.s_addr = inet_addr(my_ip_addr);
	in_addr->sin_port = htons(port_to_bind + i);
printf("bind to %s %d\n",my_ip_addr,port_to_bind + i);
        ipaugenblick_bind(sock,&addr,addrlen);
 
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

    memset(&tm_out,0,sizeof(tm_out));
    p_timeout = &tm_out;
    while(1) {
        ready_socket = ipaugenblick_select(selector,&readfdset,&writefdset,&excfdset,p_timeout);
        if(ready_socket == -1) {
            continue;
        }

        for (ready_socket = 0; ready_socket < readfdset.returned_idx; ready_socket++) {
	    if (!ipaugenblick_fdisset(ipaugenblick_fd_idx2sock(&readfdset,ready_socket),&readfdset))
		continue;
	    p_timeout = NULL;
	    len = 1448;
            while(ipaugenblick_receivefrom(ipaugenblick_fd_idx2sock(&readfdset,ready_socket),&rxbuff,&len,&addr,&addrlen,&pdesc) == 0) {
                void *porigdesc = pdesc;
		received_packets++;
                if(!(received_packets%1000)) {
                    printf("received %u transmitted_packets %u\n",received_packets,transmitted_packets);
		    print_stats();
                }
                ipaugenblick_release_rx_buffer(porigdesc,ipaugenblick_fd_idx2sock(&readfdset,ready_socket));
		len = 1448;
           }
        }
	
        for (ready_socket = 0; ready_socket < writefdset.returned_idx; ready_socket++) {
		if (!ipaugenblick_fdisset(ipaugenblick_fd_idx2sock(&writefdset,ready_socket),&writefdset))
			continue;
	    p_timeout = NULL;
            tx_space = ipaugenblick_get_socket_tx_space(ipaugenblick_fd_idx2sock(&writefdset,ready_socket));
	    if (!usebulk) {
	            for(i = 0;i < tx_space;i++) {
        	        txbuff = ipaugenblick_get_buffer(DATAGRAM_SIZE,ipaugenblick_fd_idx2sock(&writefdset,ready_socket),&pdesc);
                	if(txbuff) { 
#if USE_CONNECTED
	                    if(ipaugenblick_send(ipaugenblick_fd_idx2sock(&writefdset,ready_socket),pdesc,0,DATAGRAM_SIZE)) { 
        	                ipaugenblick_release_tx_buffer(pdesc);
                	    }
			    else
				transmitted_packets++;
#else
			    in_addr->sin_family = AF_INET;
			    in_addr->sin_addr.s_addr = inet_addr(ip_addr_2_connect);
			    in_addr->sin_port = htons(port_to_connect);
                	    if(ipaugenblick_sendto(ipaugenblick_fd_idx2sock(&writefdset,ready_socket),pdesc,0,DATAGRAM_SIZE,&addr,addrlen)) { 
                        	ipaugenblick_release_tx_buffer(pdesc);
	                    }
			    else
				transmitted_packets++;
#endif 
                	} 
            	}
	    } else {
		struct data_and_descriptor bulk_bufs[tx_space];
                int offsets[tx_space];
                int lengths[tx_space];
		struct sockaddr addresses[tx_space];
                if(!ipaugenblick_get_buffers_bulk(1448,ipaugenblick_fd_idx2sock(&writefdset,sock),tx_space,bulk_bufs)) {
                        for(i = 0;i < tx_space;i++) {
                                offsets[i] = 0;
	                        lengths[i] = 1448;
				in_addr = &addresses[i];
				in_addr->sin_family = AF_INET;
                        	in_addr->sin_addr.s_addr = inet_addr(ip_addr_2_connect);
                        	in_addr->sin_port = htons(port_to_connect);
                           }
                           if(ipaugenblick_sendto_bulk(ipaugenblick_fd_idx2sock(&writefdset,sock),
							bulk_bufs,offsets,lengths,addresses,
							sizeof(struct sockaddr),tx_space)) {
                                for(i = 0;i < tx_space;i++)
                                       	ipaugenblick_release_tx_buffer(bulk_bufs[i].pdesc);
	                        printf("%s %d\n",__FILE__,__LINE__);
                           }
                           else {
                                transmitted_packets += tx_space;
	                        if(!(transmitted_packets%1000)) {
        	                	printf("transmitted %u received_count %u\n", transmitted_packets, received_packets);
                	                print_stats();
                        	}
                            }
                    }
	    }
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

