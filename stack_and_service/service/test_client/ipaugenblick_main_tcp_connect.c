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

#define USE_TX 0
#define USE_RX 1

int main(int argc,char **argv)
{
    void *txbuff,*rxbuff,*pdesc;
    int sock,newsock,len;
    char *p;
    int size = 0,ringset_idx;
    int sockets_connected = 0;
    int selector;
    int ready_socket;
    int i,tx_space;
    int max_total_length = 0;
    unsigned short mask;
    unsigned long received_count = 0;
    unsigned long transmitted_count = 0;
    struct timeval tm_out, *p_timeout = NULL;

    if(ipaugenblick_app_init(argc,argv,"tcp_connect") != 0) {
        printf("cannot initialize memory\n");
        return 0;
    } 
    printf("memory initialized\n");
    selector = ipaugenblick_open_select();
    if(selector != -1) {
        printf("selector opened %d\n",selector);
    }

    if((sock = ipaugenblick_open_socket(AF_INET,SOCK_STREAM,selector)) < 0) {
        printf("cannot open tcp client socket\n");
        return 0;
    }
    printf("opened socket %d\n",sock);
//    int bufsize = 1024*1024*1000;
  //  ipaugenblick_setsockopt(sock, SOL_SOCKET,SO_SNDBUFFORCE,(char *)&bufsize,sizeof(bufsize));
    //ipaugenblick_setsockopt(sock, SOL_SOCKET,SO_RCVBUFFORCE,(char *)&bufsize,sizeof(bufsize));
    ipaugenblick_v4_connect_bind_socket(sock,inet_addr("192.168.150.63"),htons(7777),1);
 
    int first_time = 1;
    memset(&tm_out,0,sizeof(tm_out));
    p_timeout = &tm_out;
    while(1) {
        ready_socket = ipaugenblick_select(selector,&mask, p_timeout);
        if(ready_socket == -1) {
            continue;
        }
	
        if(first_time) {
		ipaugenblick_socket_kick(ready_socket);
		first_time = 0;
	}
        
        if(mask & /*SOCKET_READABLE_BIT*/0x1) {
#if USE_RX
	    int first_seg_len = 0;
	    int len = 0;
	    p_timeout = NULL;
            while(ipaugenblick_receive(ready_socket,&rxbuff,&len,&first_seg_len,&pdesc) == 0) {
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
		if(!(received_count%1000000)) {
                    printf("received %u transmitted_count %u\n", received_count, transmitted_count);
		    print_stats();
                }
                ipaugenblick_release_rx_buffer(porigdesc,ready_socket);
		len = 0;
            }
#endif
        }
#if USE_TX
        if(mask & /*SOCKET_WRITABLE_BIT*/0x2) {
	    p_timeout = NULL;
            tx_space = ipaugenblick_get_socket_tx_space(ready_socket)/2;
#if 1
            for(i = 0;i < tx_space;i++) {
                txbuff = ipaugenblick_get_buffer(1448,ready_socket,&pdesc);
                if(!txbuff) {
                    break;
                }
                //strcpy(txbuff,"VADIM");
                if(ipaugenblick_send(ready_socket,pdesc,0,1448)) {
                    ipaugenblick_release_tx_buffer(pdesc);
                    break;
                }
                transmitted_count++;
                if(!(transmitted_count%1000)) {
                    printf("received %u transmitted_count %u\n", received_count, transmitted_count);
                    print_stats();
                }
            }
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
			/*if(!(transmitted_count%1000))*/ {
                    		printf("transmitted %u received_count %u\n",transmitted_count, received_count);
				print_stats();
                	}
                    }
            }
#endif
	    int iter = 0;
            while(ipaugenblick_socket_kick(ready_socket) == -1) {
		iter++;
		if(!(iter%1000000)) {
			printf("iter!\n");exit(0);
		}
	    }
        }  
//        ipaugenblick_socket_kick(ready_socket);
#endif
	if (mask == 0) {
		memset(&tm_out,0,sizeof(tm_out));
		p_timeout = &tm_out;
	}
    }
    return 0;
}
