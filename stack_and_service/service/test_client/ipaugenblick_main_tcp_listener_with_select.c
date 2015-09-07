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
#include <ipaugenblick_api.h>
#include <string.h>

#define USE_TX 1
#define USE_RX 1
#define LISTENERS_COUNT 1
#define LISTENERS_BASE 7777

static inline int is_listener(int sock, int *listeners)
{
    int i;

    for(i = 0;i < LISTENERS_COUNT;i++)
        if(listeners[i] == sock)
	    return 1;
    return 0;
}

int main(int argc,char **argv)
{
    void *txbuff,*rxbuff,*pdesc;
    int sock,newsock,len,listeners[LISTENERS_COUNT];
    int listeners_idx;
    char *p;
    int size = 0,ringset_idx;
    int selector;
    int ready_socket_count;
    int i,tx_space = 0;
    int max_total_length = 0;
    unsigned short mask;
    unsigned long received_count = 0;
    unsigned long transmitted_count = 0;
    struct timeval tm_out, *p_timeout = NULL;
    struct ipaugenblick_fdset readfdset,writefdset, excfdset;
    struct sockaddr addr;
    struct sockaddr_in *in_addr = (struct sockaddr_in *)&addr;

   // ipaugenblick_set_log_level(0/*IPAUGENBLICK_LOG_DEBUG*/);

    ipaugenblick_fdzero(&readfdset);
    ipaugenblick_fdzero(&writefdset);
    ipaugenblick_fdzero(&excfdset);

    if(ipaugenblick_app_init(argc,argv,"tcp_listener") != 0) {
        printf("cannot initialize memory\n");
        return 0;
    } 
    printf("memory initialized\n");
    selector = ipaugenblick_open_select();
    if(selector != -1) {
        printf("selector opened\n");
    }
    in_addr->sin_family = AF_INET;
    for(listeners_idx = 0;listeners_idx < LISTENERS_COUNT;listeners_idx++) {
	    if((sock = ipaugenblick_open_socket(AF_INET,SOCK_STREAM,selector)) < 0) {
        	printf("cannot open tcp client socket\n");
	        return 0;
    	}
	in_addr->sin_addr.s_addr = inet_addr("192.168.150.63");
	in_addr->sin_port = LISTENERS_BASE + listeners_idx;
    	ipaugenblick_bind(sock,&addr, sizeof(addr));

    	ipaugenblick_listen_socket(sock);
    	listeners[listeners_idx] = sock;
	ipaugenblick_fdset (sock, &readfdset);
    //    int bufsize = 1024*1024*1000;
//	ipaugenblick_setsockopt(sock, SOL_SOCKET,SO_SNDBUFFORCE,(char *)&bufsize,sizeof(bufsize));
  //      ipaugenblick_setsockopt(sock, SOL_SOCKET,SO_RCVBUFFORCE,(char *)&bufsize,sizeof(bufsize));
    }
    
    //    ipaugenblick_set_socket_select(sock,selector); 
    p_timeout = &tm_out;
    while(1) {
	memset(&tm_out,0,sizeof(tm_out));
	p_timeout = /*&tm_out*/NULL;
        ready_socket_count = ipaugenblick_select(selector,&readfdset,&writefdset,&excfdset, p_timeout);
//	printf("ready_socket_count %d read cnt %d write cnt %d\n",ready_socket_count, readfdset.returned_idx, writefdset.returned_idx);
        if(ready_socket_count == -1) {
            continue;
        }
	if (ready_socket_count == 0) {
		continue;
	}
	for (sock = 0; sock < readfdset.returned_idx; sock++) {
		if (!ipaugenblick_fdisset(ipaugenblick_fd_idx2sock(&readfdset,sock),&readfdset))
			continue;
	        if(is_listener(ipaugenblick_fd_idx2sock(&readfdset,sock),listeners)) {
		    unsigned int ipaddr;
		    unsigned short port;
		    int addrlen = sizeof(*in_addr);
		    p_timeout = NULL;
        	    while((newsock = ipaugenblick_accept(ipaugenblick_fd_idx2sock(&readfdset,sock),&addr,&addrlen)) != -1) {
                	printf("socket accepted %d %d %x %d\n",newsock,selector,ipaddr,port);
	                ipaugenblick_set_socket_select(newsock,selector);
			ipaugenblick_fdset (newsock, &readfdset);
			ipaugenblick_fdset (newsock, &writefdset);
			ipaugenblick_fdset (newsock, &excfdset);
            	    }
        	} else {
		    int first_seg_len = 0;
		    len = /*1024*/0;
		    p_timeout = NULL;
        	    while(ipaugenblick_receive(ipaugenblick_fd_idx2sock(&readfdset,sock),&rxbuff,&len,&first_seg_len,&pdesc) == 0) {

			int segs = 0;
	                void *porigdesc = pdesc;

#if 1
        	        while(rxbuff) { 
                	    if(len > 0) {
                        	/* do something */
	                        /* don't release buf, release rxbuff */
        	            }
                	    segs++;
	                    if(segs > 100000) {
        	                printf("segs!!!!\n");exit(0);
                	    }
                            rxbuff = ipaugenblick_get_next_buffer_segment(&pdesc,&len);
                	}

	                received_count+=segs;
#else
        	        received_count++;
#endif 
			if(!(received_count%1000)) {
	                    printf("received %u transmitted_count %u\n", received_count, transmitted_count);
			    print_stats();
                	}
	                ipaugenblick_release_rx_buffer(porigdesc,ipaugenblick_fd_idx2sock(&readfdset,sock));
			len = 0;
            	    }
		}
        }
#if USE_TX
        for (sock = 0; sock < writefdset.returned_idx; sock++) {
	    if (!ipaugenblick_fdisset(ipaugenblick_fd_idx2sock(&writefdset,sock),&writefdset))
			continue;
	    p_timeout = NULL;
            tx_space = ipaugenblick_get_socket_tx_space(ipaugenblick_fd_idx2sock(&writefdset,sock));
//	    printf("tx_space %d\n",tx_space);
#if 1
            for(i = 0;i < tx_space;i++) {
                txbuff = ipaugenblick_get_buffer(1448,ipaugenblick_fd_idx2sock(&writefdset,sock),&pdesc);
                if(!txbuff) {
                    break;
                }
		//strcpy(txbuff,"VADIM");
                if(ipaugenblick_send(ipaugenblick_fd_idx2sock(&writefdset,sock),pdesc,0,1448)) { 
                    ipaugenblick_release_tx_buffer(pdesc);
                    break;
                }
		transmitted_count++;
		if(!(transmitted_count%1000)) {
                    printf("received %u transmitted_count %u\n", received_count, transmitted_count);
		    print_stats();
                }
            }
//	    if(tx_space == 0)
//		continue;
#else
	    if(tx_space == 0)
		continue;
	    struct data_and_descriptor bulk_bufs[tx_space];
            int offsets[tx_space];
            int lengths[tx_space];
	    if(!ipaugenblick_get_buffers_bulk(1448,ready_socket,tx_space,bulk_bufs)) {
                    for(i = 0;i < tx_space;i++) {
                        offsets[i] = 0;
                        lengths[i] = 1448;
                    }
                    if(ipaugenblick_send_bulk(ready_socket,bulk_bufs,offsets,lengths,tx_space)) {
			for(i = 0;i < tx_space;i++)
                        	ipaugenblick_release_tx_buffer(bulk_bufs[i].pdesc);
                        printf("%s %d\n",__FILE__,__LINE__);
                    }
                    else {
			transmitted_count += tx_space;
			if(!(transmitted_count%1000)) {
                    		printf("transmitted %u received_count %u\n", transmitted_count, received_count);
				print_stats();
                	}
                    }
            }
#endif
	    int iter = 0;
            while(ipaugenblick_socket_kick(ipaugenblick_fd_idx2sock(&writefdset,sock)) == -1) {
		iter++;
		if(!(iter%1000000)) {
			printf("iter!\n");exit(0);
		}
	    }
        }  
//        ipaugenblick_socket_kick(ready_socket);
#endif	
    }
    return 0;
}
