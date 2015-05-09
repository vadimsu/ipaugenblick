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
#include <sys/queue.h>
#include "../ipaugenblick_app_api/ipaugenblick_api.h"
#include <string.h>

#define USE_TX 1
#define LISTENERS_COUNT 1
#define LISTENERS_BASE 7777
#define QUEUE_LENGTH 16384

struct echo_queue_entry
{
        void *pdesc;
	int len;
        TAILQ_ENTRY(echo_queue_entry) q_entry;
};
struct echo_queue_entry echo_queue_entries[QUEUE_LENGTH];
TAILQ_HEAD(, free_q) free_queue;
TAILQ_HEAD(, echo_q) echo_queue;

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
    int sockets_connected = 0;
    int selector;
    int ready_socket;
    int i,tx_space = 0;
    int max_total_length = 0;
    unsigned short mask;
    unsigned long received_count = 0;
    struct echo_queue_entry *p_echo_queue_entry;
    struct timeval tv,*ptv;
    int read_ready_flag = 0;
    int write_ready_flag = 0;
    int rc;

    if(ipaugenblick_app_init(argc,argv,"tcp_listener") != 0) {
        printf("cannot initialize memory\n");
        return 0;
    } 
    printf("memory initialized\n");
    TAILQ_INIT(&free_queue);
    TAILQ_INIT(&echo_queue);
    for(i = 0; i < QUEUE_LENGTH;i++) {
	TAILQ_INSERT_TAIL(&free_queue, &echo_queue_entries[i], q_entry);
    }
    selector = ipaugenblick_open_select();
    if(selector != -1) {
        printf("selector opened\n");
    }
    for(listeners_idx = 0;listeners_idx < LISTENERS_COUNT;listeners_idx++) {
	    if((sock = ipaugenblick_open_socket(AF_INET,SOCK_STREAM,selector)) < 0) {
        	printf("cannot open tcp client socket\n");
	        return 0;
    	}
    	ipaugenblick_v4_connect_bind_socket(sock,inet_addr("192.168.150.63"),LISTENERS_BASE + listeners_idx,0);

    	ipaugenblick_listen_socket(sock);
    	listeners[listeners_idx] = sock;
        int bufsize = 1024*1024*1000;
	ipaugenblick_setsockopt(sock, SOL_SOCKET,SO_SNDBUFFORCE,(char *)&bufsize,sizeof(bufsize));
        ipaugenblick_setsockopt(sock, SOL_SOCKET,SO_RCVBUFFORCE,(char *)&bufsize,sizeof(bufsize));
    }
    
    //    ipaugenblick_set_socket_select(sock,selector);
    while(1) {
	mask = 0;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	if(read_ready_flag || write_ready_flag)
		ptv = &tv;
	else
		ptv = NULL;
        ready_socket = ipaugenblick_select(selector,&mask, ptv);
        if(ready_socket == -1) {
            continue;
        }
        if(is_listener(ready_socket,listeners)) {
	    unsigned int ipaddr;
	    unsigned short port;
            while((newsock = ipaugenblick_accept(ready_socket,&ipaddr,&port)) != -1) {
                printf("socket accepted %d %d %x %d\n",newsock,selector,ipaddr,port);
                ipaugenblick_set_socket_select(newsock,selector);
                sockets_connected++;
            }
            continue;
        }
        if(sockets_connected == 0) {
            continue;
        }
        if(((mask & /*SOCKET_READABLE_BIT*/0x1)||(read_ready_flag))&&(!TAILQ_EMPTY(&free_queue))) {
	    int first_seg_len = 0;
	    len = /*1024*/0;
            while((rc = ipaugenblick_receive(ready_socket,&rxbuff,&len,&first_seg_len,&pdesc)) == 0) {
                received_count++;
                if(len > max_total_length)
                    max_total_length = len;
                void *porigdesc = pdesc;
#if 0
                while(rxbuff) {
                    
                    rxbuff = ipaugenblick_get_next_buffer_segment(&pdesc,&len);
                    if((rxbuff)&&(len > 0)) {
                        /* do something */
                        /* don't release buf, release rxbuff */
                    }
                }
#endif
                if(!(received_count%10000000)) {
                    printf("received %u max_total_len %u\n",received_count,max_total_length);
                }
		struct echo_queue_entry *p_queue_entry = TAILQ_FIRST(&free_queue);
		TAILQ_REMOVE(&free_queue,p_queue_entry, q_entry);
		p_queue_entry->pdesc = porigdesc;
		p_queue_entry->len = first_seg_len;
		TAILQ_INSERT_TAIL(&echo_queue,p_queue_entry,q_entry);
		if(TAILQ_EMPTY(&free_queue))
			break;
            }
	    if(rc == 0)
		read_ready_flag = 1;
	    else
		read_ready_flag = 0;
        }
	else if(mask & /*SOCKET_REDABLE_BIT*/0x1)
		read_ready_flag = 1;
#if USE_TX
        if(((mask & /*SOCKET_WRITABLE_BIT*/0x2)||(write_ready_flag))&&(!TAILQ_EMPTY(&echo_queue))) {
	    tx_space = ipaugenblick_get_socket_tx_space_own_buffer(ready_socket);
            for(i = 0;i < tx_space;i++) {
                p_echo_queue_entry = TAILQ_FIRST(&echo_queue);
		TAILQ_REMOVE(&echo_queue, p_echo_queue_entry, q_entry);
		pdesc = p_echo_queue_entry->pdesc;
		len = p_echo_queue_entry->len;
		do {	
                	if(ipaugenblick_send(ready_socket,pdesc,0,len)) { 
                    		ipaugenblick_release_tx_buffer(p_echo_queue_entry->pdesc);
                    		break;
                	}
			txbuff = ipaugenblick_get_next_buffer_segment(&pdesc,&len);
		} while(txbuff && pdesc);
		TAILQ_INSERT_TAIL(&free_queue,p_echo_queue_entry,q_entry);
		if(TAILQ_EMPTY(&echo_queue))
		    break;
            }
	    if(tx_space == 0)
		continue;
	    if(i < tx_space)
		write_ready_flag = 1;
	    else
		write_ready_flag = 0;
            ipaugenblick_socket_kick(ready_socket);
        }
	else if(mask & /*SOCKET_WRITABLE_BIT*/0x2)
		write_ready_flag = 1;
//        ipaugenblick_socket_kick(ready_socket);
#endif
    }
    return 0;
}
