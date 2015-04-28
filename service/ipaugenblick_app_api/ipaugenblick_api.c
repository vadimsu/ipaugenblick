#include <sys/errno.h>
#include <stdlib.h>
#include <rte_atomic.h>
#include <rte_config.h>
#include <rte_common.h>
#include <rte_memcpy.h>
#include <rte_lcore.h>
#include <rte_mempool.h>
#include <rte_cycles.h>
#include <rte_ring.h>
#include <rte_mbuf.h>
#include <rte_byteorder.h>
#include "../ipaugenblick_common/ipaugenblick_common.h"
#include "ipaugenblick_ring_ops.h"
#include "ipaugenblick_api.h"
#include <netinet/in.h> 
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#if 0
#define offsetof(TYPE, MEMBER) ((size_t)&((TYPE*)0)->MEMBER)
#endif

#define container_of(ptr, type, member)({ \
const typeof(((type*) 0)->member)*__mptr=(ptr); \
(type*)((char*)__mptr-offsetof(type, member));})

#define PKT(d) container_of(d,struct rte_pktmbuf,data)

#define RTE_MBUF(d) container_of(PKT(d),struct rte_mbuf,pkt)

local_socket_descriptor_t local_socket_descriptors[IPAUGENBLICK_CONNECTION_POOL_SIZE];
struct rte_mempool *free_connections_pool = NULL;
struct rte_ring *free_connections_ring = NULL;
struct rte_ring *free_clients_ring = NULL;
struct rte_ring *client_ring = NULL;
struct rte_mempool *tx_bufs_pool = NULL;
struct rte_ring *rx_bufs_ring = NULL;
struct rte_mempool *free_command_pool = NULL;
struct rte_ring *command_ring = NULL;
struct rte_ring *selectors_ring = NULL;

typedef struct
{
    struct rte_ring *ready_connections; 
}selector_t;

static selector_t selectors[IPAUGENBLICK_CONNECTION_POOL_SIZE];

uint64_t ipaugenblick_stats_receive_called = 0;
uint64_t ipaugenblick_stats_send_called = 0;
uint64_t ipaugenblick_stats_rx_kicks_sent = 0;
uint64_t ipaugenblick_stats_tx_kicks_sent = 0;
uint64_t ipaugenblick_stats_rx_full = 0;
uint64_t ipaugenblick_stats_rx_dequeued = 0;
uint64_t ipaugenblick_stats_rx_dequeued_local = 0;
uint64_t ipaugenblick_stats_select_called = 0;
uint64_t ipaugenblick_stats_select_returned = 0;
uint64_t ipaugenblick_stats_tx_buf_allocation_failure = 0;
uint64_t ipaugenblick_stats_send_failure = 0;
uint64_t ipaugenblick_stats_recv_failure = 0;
uint64_t ipaugenblick_stats_buffers_sent = 0;
uint64_t ipaugenblick_stats_buffers_allocated = 0;
uint64_t ipaugenblick_stats_cannot_allocate_cmd = 0;
pthread_t stats_thread;
uint8_t g_print_stats_loop = 1;
uint32_t g_client_ringset_idx = IPAUGENBLICK_CONNECTION_POOL_SIZE;
ipaugenblick_update_cbk_t ipaugenblick_update_cbk = NULL;

void print_stats()
{
    while(g_print_stats_loop) {
#if 0
        printf("ipaugenblick_stats_receive_called %lu ipaugenblick_stats_send_called %lu \n\t\
                ipaugenblick_stats_rx_kicks_sent %lu ipaugenblick_stats_tx_kicks_sent %lu ipaugenblick_stats_cannot_allocate_cmd %lu  \n\t\
                ipaugenblick_stats_rx_full %lu ipaugenblick_stats_rx_dequeued %lu ipaugenblick_stats_rx_dequeued_local %lu \n\t\
                ipaugenblick_stats_select_called %lu ipaugenblick_stats_select_returned %lu ipaugenblick_stats_tx_buf_allocation_failure %lu \n\t\
                ipaugenblick_stats_send_failure %lu ipaugenblick_stats_recv_failure %lu ipaugenblick_stats_buffers_sent %lu ipaugenblick_stats_buffers_allocated %lu\n",
                ipaugenblick_stats_receive_called,ipaugenblick_stats_send_called,ipaugenblick_stats_rx_kicks_sent,
                ipaugenblick_stats_tx_kicks_sent,ipaugenblick_stats_cannot_allocate_cmd,ipaugenblick_stats_rx_full,ipaugenblick_stats_rx_dequeued,
                ipaugenblick_stats_rx_dequeued_local,ipaugenblick_stats_select_called,ipaugenblick_stats_select_returned,ipaugenblick_stats_tx_buf_allocation_failure,
                ipaugenblick_stats_send_failure,ipaugenblick_stats_recv_failure,
                ipaugenblick_stats_buffers_sent,
                ipaugenblick_stats_buffers_allocated);
#endif
        sleep(1);
    }
}

static inline void ipaugenblick_free_command_buf(ipaugenblick_cmd_t *cmd)
{
    rte_mempool_put(free_command_pool,(void *)cmd);
}

void sig_handler(int signum)
{
    uint32_t i;
    ipaugenblick_cmd_t *cmd;

    if(signum == SIGUSR1) {
        /* T.B.D. do something to wake up the thread */
        return;
    }

    printf("terminating on signal %d\n",signum);

    for(i = 0;i < IPAUGENBLICK_CONNECTION_POOL_SIZE;i++) {
        if(local_socket_descriptors[i].socket) {
            ipaugenblick_close(local_socket_descriptors[i].socket->connection_idx);
        }
    }
    
    cmd = ipaugenblick_get_free_command_buf();
    if(cmd) {
       cmd->cmd = IPAUGENBLICK_DISCONNECT_CLIENT;
       cmd->ringset_idx = g_client_ringset_idx;
        if(ipaugenblick_enqueue_command_buf(cmd)) {
           ipaugenblick_free_command_buf(cmd);
        }
    }

    signal(signum,SIG_DFL);
    g_print_stats_loop = 0;
    kill(getpid(),signum);
}

/* must be called per process */
int ipaugenblick_app_init(int argc,char **argv,char *app_unique_id)
{
    int i;
    char ringname[1024];

    if(rte_eal_init(argc, argv) < 0) {
        printf("%s %d\n",__FILE__,__LINE__);
	return -1;
    }
    printf("EAL initialized\n");

    free_clients_ring = rte_ring_lookup(FREE_CLIENTS_RING);
    if(!free_clients_ring) {
        printf("cannot find ring %s %d\n",__FILE__,__LINE__);
        exit(0);
    }

    free_connections_ring = rte_ring_lookup(FREE_CONNECTIONS_RING);

    if(!free_connections_ring) {
        printf("cannot find free connections ring\n");
        return -1;
    }

    free_connections_pool = rte_mempool_lookup(FREE_CONNECTIONS_POOL_NAME);

    if(!free_connections_pool) {
        printf("cannot find free connections pool\n");
        return -1;
    }

    memset(local_socket_descriptors,0,sizeof(local_socket_descriptors));
    for(i = 0;i < IPAUGENBLICK_CONNECTION_POOL_SIZE;i++) {
        sprintf(ringname,RX_RING_NAME_BASE"%d",i);
        local_socket_descriptors[i].rx_ring = rte_ring_lookup(ringname);
        if(!local_socket_descriptors[i].rx_ring) {
            printf("%s %d\n",__FILE__,__LINE__);
            exit(0);
        }
        sprintf(ringname,TX_RING_NAME_BASE"%d",i);
        local_socket_descriptors[i].tx_ring = rte_ring_lookup(ringname);
        if(!local_socket_descriptors[i].tx_ring) {
            printf("%s %d\n",__FILE__,__LINE__);
            exit(0);
        }
        local_socket_descriptors[i].select = -1;
        local_socket_descriptors[i].socket = NULL;
        sprintf(ringname,"lrxcache%s_%d",app_unique_id,i);
        printf("local cache name %s\n",ringname);
        local_socket_descriptors[i].local_cache = rte_ring_create(ringname, 16384,rte_socket_id(), RING_F_SC_DEQ|RING_F_SP_ENQ);
        if(!local_socket_descriptors[i].local_cache) {
           printf("cannot create local cache\n");
	   local_socket_descriptors[i].local_cache = rte_ring_lookup(ringname);
	   if(!local_socket_descriptors[i].local_cache) {
		printf("and cannot find\n");
		exit(0);
	   } 
        }
	local_socket_descriptors[i].any_event_received = 0;
    }
    tx_bufs_pool = rte_mempool_lookup("mbufs_mempool");
    if(!tx_bufs_pool) {
        printf("cannot find tx bufs pool\n");
        return -1;
    }
    
    free_command_pool = rte_mempool_lookup(FREE_COMMAND_POOL_NAME);
    if(!free_command_pool) {
        printf("cannot find free command pool\n");
        return -1;
    }
    
    command_ring = rte_ring_lookup(COMMAND_RING_NAME);
    if(!command_ring) {
        printf("cannot find command ring\n");
        return -1;
    }
    rx_bufs_ring = rte_ring_lookup("rx_mbufs_ring");
    if(!rx_bufs_ring) {
        printf("cannot find rx bufs ring\n");
        return -1;
    }
    selectors_ring = rte_ring_lookup(SELECTOR_RING_NAME);
    
    for(i = 0;i < IPAUGENBLICK_SELECTOR_POOL_SIZE;i++) {
        sprintf(ringname,"SELECTOR_RING_NAME%d",i);
        selectors[i].ready_connections = rte_ring_lookup(ringname);
        if(!selectors[i].ready_connections) {
            printf("cannot find ring %s %d\n",__FILE__,__LINE__);
            exit(0);
        } 
    }
    
    signal(SIGHUP, sig_handler);
    signal(SIGINT, sig_handler);
    signal(SIGILL, sig_handler);
    signal(SIGABRT, sig_handler);
    signal(SIGFPE, sig_handler);
    signal(SIGFPE, sig_handler);
    signal(SIGSEGV, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGUSR1, sig_handler);
    pthread_create(&stats_thread,NULL,print_stats,NULL);
    return ((tx_bufs_pool == NULL)||(command_ring == NULL)||(free_command_pool == NULL));
}

int ipaugenblick_create_client(ipaugenblick_update_cbk_t update_cbk)
{
    int ringset_idx;
    ipaugenblick_cmd_t *cmd;
    char ringname[1024];

    if(rte_ring_dequeue(free_clients_ring,(void **)&ringset_idx)) {
        printf("%s %d\n",__FILE__,__LINE__);
        return -1;
    }
    sprintf(ringname,"%s%d",FREE_CLIENTS_RING,ringset_idx);
    client_ring = rte_ring_lookup(ringname);
    if(!client_ring) {
	printf("%s %d\n",__FILE__,__LINE__);
        return -2;
    }
    cmd = ipaugenblick_get_free_command_buf();
    if(!cmd) {
       ipaugenblick_stats_cannot_allocate_cmd++;
       return -3;
    }

    cmd->cmd = IPAUGENBLICK_CONNECT_CLIENT;
    cmd->ringset_idx = ringset_idx;
    if(ipaugenblick_enqueue_command_buf(cmd)) {
       ipaugenblick_free_command_buf(cmd);
       return -3;
    }
    g_client_ringset_idx = (uint32_t)ringset_idx;
    ipaugenblick_update_cbk = update_cbk;
    return 0;
}

int ipaugenblick_read_updates(void)
{
	struct rte_mbuf *mbuf = NULL;
	unsigned char cmd = 0;
	
	if(rte_ring_dequeue(client_ring,(void **)&mbuf)) {
		printf("%s %d\n",__FILE__,__LINE__);
		return -1;
	}
	unsigned char *p = mbuf->pkt.data;
	switch(*p) {
		case IPAUGENBLICK_NEW_IFACES:
		if(ipaugenblick_update_cbk) {
			cmd = 1;
			p++;
			ipaugenblick_update_cbk(cmd,p,mbuf->pkt.data_len - 1);
		}
		break;
		case IPAUGENBLICK_NEW_ADDRESSES:
		if(ipaugenblick_update_cbk) {
			cmd = 3;
			p++;
			ipaugenblick_update_cbk(cmd,p,mbuf->pkt.data_len - 1);
		}
		break;
	}
	return 0;
}

int ipaugenblick_open_select(void)
{
    int ringset_idx;

    if(rte_ring_dequeue(selectors_ring,(void **)&ringset_idx)) {
        printf("%s %d\n",__FILE__,__LINE__);
        return -1;
    }
    return (int)ringset_idx;
}

int ipaugenblick_set_socket_select(int sock,int select)
{
   ipaugenblick_cmd_t *cmd;

    if(sock < 0)
        return -1;

   cmd = ipaugenblick_get_free_command_buf();
   if(!cmd) {
       ipaugenblick_stats_cannot_allocate_cmd++;
       return -2;
   }

   cmd->cmd = IPAUGENBLICK_SET_SOCKET_SELECT_COMMAND;
   cmd->ringset_idx = sock;
   cmd->u.set_socket_select.socket_select = select;
   cmd->u.set_socket_select.pid = getpid();
   if(ipaugenblick_enqueue_command_buf(cmd)) {
       ipaugenblick_free_command_buf(cmd);
       return -3;
   }
   local_socket_descriptors[sock].select = select;
}
int ipaugenblick_open_socket(int family,int type,int parent)
{
	ipaugenblick_socket_t *ipaugenblick_socket;
	ipaugenblick_cmd_t *cmd; 

	if(rte_ring_dequeue(free_connections_ring,(void **)&ipaugenblick_socket)) {
        	printf("%s %d\n",__FILE__,__LINE__);
        	return -1;
    	}

    	/* allocate a ringset (cmd/tx/rx) here */
    	cmd = ipaugenblick_get_free_command_buf();
    	if(!cmd) {
        	ipaugenblick_stats_cannot_allocate_cmd++;
        	return -2;
    	}

    	cmd->cmd = IPAUGENBLICK_OPEN_SOCKET_COMMAND;
	cmd->ringset_idx = ipaugenblick_socket->connection_idx;
    	cmd->parent_idx = parent;
	cmd->u.open_sock.family = family;
	cmd->u.open_sock.type = type;
	if(ipaugenblick_enqueue_command_buf(cmd)) {
        	ipaugenblick_free_command_buf(cmd);
	        return -3;
    	}

    	local_socket_descriptors[ipaugenblick_socket->connection_idx].socket = ipaugenblick_socket;
	if(parent != -1)
		local_socket_descriptors[ipaugenblick_socket->connection_idx].select = parent;

    	return ipaugenblick_socket->connection_idx;
}

int ipaugenblick_v4_connect_bind_socket(int sock,unsigned int ipaddr,unsigned short port,int is_connect)
{
	ipaugenblick_cmd_t *cmd;

	cmd = ipaugenblick_get_free_command_buf();
    	if(!cmd) {
        	ipaugenblick_stats_cannot_allocate_cmd++;
        	return -1;
    	}
 
	cmd->cmd = IPAUGENBLICK_SOCKET_CONNECT_BIND_COMMAND;
	cmd->ringset_idx = sock;
	cmd->parent_idx = local_socket_descriptors[sock].select;
	cmd->u.socket_connect_bind.ipaddr = ipaddr;
        cmd->u.socket_connect_bind.port = port;
	cmd->u.socket_connect_bind.is_connect = is_connect;
    	if(ipaugenblick_enqueue_command_buf(cmd)) {
       		ipaugenblick_free_command_buf(cmd);
		return -2;
    	}
	if(is_connect) {
		local_socket_descriptors[sock].remote_ipaddr = ipaddr;
		local_socket_descriptors[sock].remote_port = port;
	}
	else {
		local_socket_descriptors[sock].local_ipaddr = ipaddr;
		local_socket_descriptors[sock].local_port = port;
	}
	return 0;
}

int ipaugenblick_listen_socket(int sock)
{
	ipaugenblick_cmd_t *cmd;

	cmd = ipaugenblick_get_free_command_buf();
    	if(!cmd) {
        	ipaugenblick_stats_cannot_allocate_cmd++;
        	return -1;
    	}
 
	cmd->cmd = IPAUGENBLICK_LISTEN_SOCKET_COMMAND;
	cmd->ringset_idx = sock;
	cmd->parent_idx = local_socket_descriptors[sock].select;
    	if(ipaugenblick_enqueue_command_buf(cmd)) {
       		ipaugenblick_free_command_buf(cmd);
		return -2;
    	}
	return 0;
}

/* close any socket */
void ipaugenblick_close(int sock)
{
    ipaugenblick_cmd_t *cmd;
    cmd = ipaugenblick_get_free_command_buf();
    if(!cmd) {
        ipaugenblick_stats_cannot_allocate_cmd++;
        return;
    }
    cmd->cmd = IPAUGENBLICK_SOCKET_CLOSE_COMMAND;
    cmd->ringset_idx = sock;
    cmd->parent_idx = local_socket_descriptors[sock].select;
    if(ipaugenblick_enqueue_command_buf(cmd)) {
       ipaugenblick_free_command_buf(cmd); 
    }
}

static inline void ipaugenblick_notify_empty_tx_buffers(int sock)
{
    ipaugenblick_cmd_t *cmd;
    cmd = ipaugenblick_get_free_command_buf();
    if(!cmd) {
        ipaugenblick_stats_cannot_allocate_cmd++;
        return;
    }
    cmd->cmd = IPAUGENBLICK_SOCKET_TX_POOL_EMPTY_COMMAND;
    cmd->ringset_idx = sock;
    if(ipaugenblick_enqueue_command_buf(cmd)) {
        ipaugenblick_free_command_buf(cmd);
    }
}

int ipaugenblick_get_socket_tx_space(int sock)
{
    int ring_space = ipaugenblick_socket_tx_space(sock);
    int free_bufs_count = rte_mempool_count(tx_bufs_pool);
    int rc = (ring_space > free_bufs_count) ? (free_bufs_count > 0 ? free_bufs_count : 0)  : ring_space;
    int tx_space = rte_atomic32_read(&(local_socket_descriptors[sock & SOCKET_READY_MASK].socket->tx_space))/1448;
//    printf("ring space %d free bufs %d tx space %d\n",ring_space,free_bufs_count,tx_space);
    if(rc > tx_space)
	rc = tx_space;
    if(!rc) {
	rte_atomic16_set(&(local_socket_descriptors[sock & SOCKET_READY_MASK].socket->write_ready_to_app),0);
        ipaugenblick_notify_empty_tx_buffers(sock);
    }
    return rc;
}

/* TCP or connected UDP */
inline int ipaugenblick_send(int sock,void *buffer,int offset,int length)
{
    int rc;
    struct rte_mbuf *mbuf = RTE_MBUF(buffer);
    ipaugenblick_stats_send_called++;
    mbuf->pkt.data_len = length;
    rte_atomic16_set(&(local_socket_descriptors[sock & SOCKET_READY_MASK].socket->write_ready_to_app),0);
    rc = ipaugenblick_enqueue_tx_buf(sock,mbuf);
    if(rc == 0)
        rte_atomic32_sub(&(local_socket_descriptors[sock & SOCKET_READY_MASK].socket->tx_space),length);
    ipaugenblick_stats_send_failure += (rc != 0);
    return rc;
}

inline int ipaugenblick_send_bulk(int sock,void **buffers,int *offsets,int *lengths,int buffer_count)
{
    int rc,idx,total_length = 0;
    struct rte_mbuf *mbufs[buffer_count];

    for(idx = 0;idx < buffer_count;idx++) {
        /* TODO: set offsets */
        mbufs[idx] = RTE_MBUF(buffers[idx]);
        mbufs[idx]->pkt.data_len = lengths[idx];
	total_length += lengths[idx];
    }
    ipaugenblick_stats_send_called++;
    ipaugenblick_stats_buffers_sent += buffer_count;
    rte_atomic16_set(&(local_socket_descriptors[sock & SOCKET_READY_MASK].socket->write_ready_to_app),0);
    rc = ipaugenblick_enqueue_tx_bufs_bulk(sock,mbufs,buffer_count);
    if(rc == 0)
	rte_atomic32_sub(&(local_socket_descriptors[sock & SOCKET_READY_MASK].socket->tx_space),total_length);
    ipaugenblick_stats_send_failure += (rc != 0);
    return rc;
}

/* UDP or RAW */
inline int ipaugenblick_sendto(int sock,void *buffer,int offset,int length,unsigned int ipaddr,unsigned short port)
{
    int rc;
    struct rte_mbuf *mbuf = RTE_MBUF(buffer);
    char *p_addr = mbuf->pkt.data;
    struct sockaddr_in *p_addr_in;
    ipaugenblick_stats_send_called++;
    mbuf->pkt.data_len = length;
    p_addr -= sizeof(struct sockaddr_in);
    p_addr_in = (struct sockaddr_in *)p_addr;
    p_addr_in->sin_family = AF_INET;
    p_addr_in->sin_port = htons(port);
    p_addr_in->sin_addr.s_addr = ipaddr;
    rte_atomic16_set(&(local_socket_descriptors[sock & SOCKET_READY_MASK].socket->write_ready_to_app),0);
    rc = ipaugenblick_enqueue_tx_buf(sock,mbuf);
    if(rc == 0)
        rte_atomic32_sub(&(local_socket_descriptors[sock & SOCKET_READY_MASK].socket->tx_space),length);
    ipaugenblick_stats_send_failure += (rc != 0);
    return rc;
}

inline int ipaugenblick_sendto_bulk(int sock,void **buffers,int *offsets,int *lengths,unsigned int *ipaddrs,unsigned short *ports,int buffer_count)
{
    int rc,idx,total_length = 0;
    struct rte_mbuf *mbufs[buffer_count];

    for(idx = 0;idx < buffer_count;idx++) {
        char *p_addr;
        struct sockaddr_in *p_addr_in;
        /* TODO: set offsets */
        mbufs[idx] = RTE_MBUF(buffers[idx]);
        mbufs[idx]->pkt.data_len = lengths[idx];
	total_length += lengths[idx];
        p_addr = mbufs[idx]->pkt.data;
        
        mbufs[idx]->pkt.data_len = lengths[idx];
        p_addr -= sizeof(struct sockaddr_in);
        p_addr_in = (struct sockaddr_in *)p_addr;
        p_addr_in->sin_family = AF_INET;
        p_addr_in->sin_port = htons(ports[idx]);
        p_addr_in->sin_addr.s_addr = ipaddrs[idx];
    }
    ipaugenblick_stats_send_called++;
    ipaugenblick_stats_buffers_sent += buffer_count;
    rte_atomic16_set(&(local_socket_descriptors[sock & SOCKET_READY_MASK].socket->write_ready_to_app),0);
    rc = ipaugenblick_enqueue_tx_bufs_bulk(sock,mbufs,buffer_count);
    if(rc == 0)
	rte_atomic32_sub(&(local_socket_descriptors[sock & SOCKET_READY_MASK].socket->tx_space),total_length);
    ipaugenblick_stats_send_failure += (rc != 0);
    return rc;
}

static inline struct rte_mbuf *ipaugenblick_get_from_shadow(int sock)
{
	struct rte_mbuf *mbuf = NULL;

	if(local_socket_descriptors[sock].shadow) {
		mbuf = local_socket_descriptors[sock].shadow;
		local_socket_descriptors[sock].shadow = NULL;
		mbuf->pkt.data_len = local_socket_descriptors[sock].shadow_len_remainder;
		mbuf->pkt.data = (char *)mbuf->pkt.data + local_socket_descriptors[sock].shadow_len_delievered;
		if(mbuf->pkt.next) {
			mbuf->pkt.pkt_len = mbuf->pkt.data_len + mbuf->pkt.next->pkt.pkt_len;
		}
		else {
			mbuf->pkt.pkt_len = mbuf->pkt.data_len;
		}
		mbuf->pkt.next = local_socket_descriptors[sock].shadow_next;
		local_socket_descriptors[sock].shadow_next = NULL;
	}
	return mbuf;
}

static inline void ipaugenblick_try_read_exact_amount(struct rte_mbuf *mbuf,int sock,int *total_len,int *first_segment_len)
{
	struct rte_mbuf *tmp = mbuf,*prev = NULL;
	int curr_len = 0;
	while((tmp)&&((tmp->pkt.data_len + curr_len) < *total_len)) {
		curr_len += tmp->pkt.data_len;
//		printf("%s %d %d\n",__FILE__,__LINE__,tmp->pkt.data_len);
		prev = tmp;
		tmp = tmp->pkt.next;
	}
	if(tmp) {
//		printf("%s %d %d\n",__FILE__,__LINE__,tmp->pkt.data_len);
		if((curr_len + tmp->pkt.data_len) > *total_len) { /* more data remains */
			local_socket_descriptors[sock].shadow = tmp;
			local_socket_descriptors[sock].shadow_next = tmp->pkt.next;
			local_socket_descriptors[sock].shadow_len_remainder = (curr_len + tmp->pkt.data_len) - *total_len;
			local_socket_descriptors[sock].shadow_len_delievered = 
				tmp->pkt.data_len - local_socket_descriptors[sock].shadow_len_remainder;
			tmp->pkt.data_len = local_socket_descriptors[sock].shadow_len_delievered;
			*first_segment_len = local_socket_descriptors[sock].shadow_len_delievered;
		}
		/* if less data than required, tmp is NULL. we're here that means exact amount is read */	
		else {
			*first_segment_len = mbuf->pkt.data_len;
			/* store next mbuf, if there is */
			if(tmp->pkt.next) {
				local_socket_descriptors[sock].shadow = tmp->pkt.next;
				local_socket_descriptors[sock].shadow_next = tmp->pkt.next->pkt.next;
				if(local_socket_descriptors[sock].shadow_next) {
					local_socket_descriptors[sock].shadow_len_remainder = 
						local_socket_descriptors[sock].shadow_next->pkt.data_len;
					local_socket_descriptors[sock].shadow_len_delievered = 0;
				}
			}
		}
		tmp->pkt.next = NULL;
		if(curr_len == *total_len) {
			if(prev)
				prev->pkt.next = NULL;
		}	
	}
	else {
		*total_len = curr_len;
		*first_segment_len = mbuf->pkt.data_len;
	}		
}

/* TCP */
int ipaugenblick_receive(int sock,void **pbuffer,int *total_len,int *first_segment_len)
{ 
    struct rte_mbuf *mbuf;

    ipaugenblick_stats_receive_called++;
    /* first try to look shadow. shadow pointer saved when last mbuf delievered partially */
    mbuf = ipaugenblick_get_from_shadow(sock);
    if((mbuf)&&(*total_len > 0)) { /* total_len > 0 means user restricts total read count */
//	printf("%s %d %d\n",__FILE__,__LINE__,*total_len);
	int total_len2 = *total_len;
	/* now find mbuf (if any) to be delievered partially and save it to shadown */
	ipaugenblick_try_read_exact_amount(mbuf,sock,&total_len2,first_segment_len);
	*pbuffer = &(mbuf->pkt.data);
	if((total_len2 > 0)&&(total_len2 < *total_len)) { /* read less than user requested, try ring */
//		printf("%s %d %d\n",__FILE__,__LINE__,total_len2);
		struct rte_mbuf *mbuf2 = ipaugenblick_dequeue_rx_buf(sock);
		if(!mbuf2) { /* ring is empty */
			*total_len = total_len2;
//			printf("%s %d\n",__FILE__,__LINE__);
		}
		else { /* now try to find an mbuf to be delievered partially in the chain */
			int total_len3 = *total_len - total_len2;
			int first_segment_len_dummy;
			ipaugenblick_try_read_exact_amount(mbuf2,sock,&total_len3,&first_segment_len_dummy);
			struct rte_mbuf *last_mbuf = rte_pktmbuf_lastseg(mbuf);
			last_mbuf->pkt.next = mbuf2;
			*total_len = total_len2 + total_len3;
//			printf("%s %d %d\n",__FILE__,__LINE__,total_len3);
		}
	}
	else {
//		printf("%s %d %d\n",__FILE__,__LINE__,*total_len);
		//goto read_from_ring;
	}
	if(local_socket_descriptors[sock].shadow) {
		uint32_t ringidx_ready_mask = sock|(SOCKET_READABLE_BIT << SOCKET_READY_SHIFT);
		rte_ring_enqueue(selectors[local_socket_descriptors[sock].select].ready_connections,(void *)ringidx_ready_mask);	
	}
	return 0;
    }
    if(mbuf) { /* means user does not care how much to read. Try to read all */
	    struct rte_mbuf *mbuf2 = ipaugenblick_dequeue_rx_buf(sock);
	    struct rte_mbuf *last_mbuf = rte_pktmbuf_lastseg(mbuf);
	    last_mbuf->pkt.next = mbuf2;
	    if(mbuf2)
		    mbuf->pkt.pkt_len += mbuf2->pkt.pkt_len;
	    *total_len = mbuf->pkt.pkt_len;
	    *first_segment_len = mbuf->pkt.data_len;
	    *pbuffer = &(mbuf->pkt.data); 
//	    printf("%s %d\n",__FILE__,__LINE__);
	    return 0;
    }
read_from_ring:
    /* either user cares how much to read or/and shadow is empty */
    mbuf = ipaugenblick_dequeue_rx_buf(sock);
    if(mbuf) {
	if(*total_len > 0) { /* read exact */
//		printf("%s %d %d\n",__FILE__,__LINE__,*total_len);
		int total_len2 = *total_len;
		ipaugenblick_try_read_exact_amount(mbuf,sock,&total_len2,first_segment_len);
		*total_len = total_len2;
		if(local_socket_descriptors[sock].shadow) {
			uint32_t ringidx_ready_mask = sock|(SOCKET_READABLE_BIT << SOCKET_READY_SHIFT);
			rte_ring_enqueue(selectors[local_socket_descriptors[sock].select].ready_connections,(void *)ringidx_ready_mask);	
		}
	}
	else {
//		printf("%s %d %d\n",__FILE__,__LINE__,mbuf->pkt.pkt_len);
		*total_len = mbuf->pkt.pkt_len;
		*first_segment_len = mbuf->pkt.data_len;	
	}
	*pbuffer = &(mbuf->pkt.data);
	return 0;
    }
    ipaugenblick_stats_recv_failure++;
    return -1;
}

/* UDP or RAW */
inline int ipaugenblick_receivefrom(int sock,void **buffer,int *len,unsigned int *ipaddr,unsigned short *port)
{
    struct rte_mbuf *mbuf = ipaugenblick_dequeue_rx_buf(sock);
    ipaugenblick_stats_receive_called++;

    if(!mbuf) {
        ipaugenblick_stats_recv_failure++;
        return -1;
    }
    *buffer = &(mbuf->pkt.data);
    *len = mbuf->pkt.pkt_len;
    char *p_addr = mbuf->pkt.data;
    p_addr -= sizeof(struct sockaddr_in);
    struct sockaddr_in *p_addr_in = (struct sockaddr_in *)p_addr;
    *port = p_addr_in->sin_port;
    *ipaddr = p_addr_in->sin_addr.s_addr;
    return 0;
}

/* Allocate buffer to use later in *send* APIs */
inline void *ipaugenblick_get_buffer(int length,int owner_sock)
{
    struct rte_mbuf *mbuf;
    mbuf = rte_pktmbuf_alloc(tx_bufs_pool);
    if(!mbuf) {
        ipaugenblick_notify_empty_tx_buffers(owner_sock);
        ipaugenblick_stats_tx_buf_allocation_failure++; 
        return NULL;
    }
    ipaugenblick_stats_buffers_allocated++;
    return &(mbuf->pkt.data);
}

int ipaugenblick_get_buffers_bulk(int length,int owner_sock,int count,void **bufs)
{
    struct rte_mbuf *mbufs[count];
    int idx;
    if(rte_mempool_get_bulk(tx_bufs_pool,mbufs,count)) {
        ipaugenblick_notify_empty_tx_buffers(owner_sock); 
        ipaugenblick_stats_tx_buf_allocation_failure++; 
        return 1;
    }
    for(idx = 0;idx < count;idx++) {
        rte_pktmbuf_reset(mbufs[idx]);
        rte_pktmbuf_refcnt_update(mbufs[idx],1);
        bufs[idx] = &(mbufs[idx]->pkt.data);
    } 
    ipaugenblick_stats_buffers_allocated += count;
    return 0;
}

/* release buffer when either send is complete or receive has done with the buffer */
void ipaugenblick_release_tx_buffer(void *buffer)
{
    struct rte_mbuf *mbuf = RTE_MBUF(buffer);

    rte_pktmbuf_free_seg(mbuf);
}

inline void ipaugenblick_release_rx_buffer(void *buffer,int sock)
{
    struct rte_mbuf *mbuf = RTE_MBUF(buffer); 
#if 1 /* this will work if only all mbufs are guaranteed from the same mempool */
    struct rte_mbuf *next;
    void *mbufs[MAX_PKT_BURST];
    int count;
    while(mbuf) {	
        for(count = 0;(count < MAX_PKT_BURST)&&(mbuf);) {
	    if(mbuf == local_socket_descriptors[sock].shadow) {
	       mbuf = NULL; /* to stop the outer loop after freeing */
	       break;
	    }
            next = mbuf->pkt.next;
            if(likely(__rte_pktmbuf_prefree_seg(mbuf))) {
                mbufs[count++] = mbuf; 
                mbuf->pkt.next = NULL; 
            }
            mbuf = next;
        }
        if(count > 0)
            rte_mempool_put_bulk(((struct rte_mbuf *)mbufs[0])->pool,mbufs,count);
    }
#else
    rte_pktmbuf_free(mbuf); 
#endif
}

int ipaugenblick_socket_kick(int sock)
{
    ipaugenblick_cmd_t *cmd;
    if(!rte_atomic16_test_and_set(&(local_socket_descriptors[sock].socket->write_done_from_app)) > 0) {
        return 0;
    }
    cmd = ipaugenblick_get_free_command_buf();
    if(!cmd) {
        ipaugenblick_stats_cannot_allocate_cmd++;
        return -1;
    }
    cmd->cmd = IPAUGENBLICK_SOCKET_TX_KICK_COMMAND;
    cmd->ringset_idx = sock;
    if(ipaugenblick_enqueue_command_buf(cmd)) {
        ipaugenblick_free_command_buf(cmd);
    }
    else
        ipaugenblick_stats_tx_kicks_sent++;
    return 0;
}

int ipaugenblick_accept(int sock,unsigned int *ipaddr,unsigned short *port)
{
    ipaugenblick_cmd_t *cmd;
    ipaugenblick_socket_t *ipaugenblick_socket;
    unsigned long accepted_socket;

    rte_atomic16_set(&(local_socket_descriptors[sock].socket->read_ready_to_app),0);
    if(rte_ring_dequeue(local_socket_descriptors[sock].rx_ring,(void **)&cmd)) {
        return -1;
    }
    
    if(rte_ring_dequeue(free_connections_ring,(void **)&ipaugenblick_socket)) {
        ipaugenblick_free_command_buf(cmd);
	printf("NO FREE CONNECTIONS\n");
        return -1;
    } 
    accepted_socket = cmd->u.accepted_socket.socket_descr;
    *ipaddr = cmd->u.accepted_socket.ipaddr;
    *port = cmd->u.accepted_socket.port;
    local_socket_descriptors[ipaugenblick_socket->connection_idx].remote_ipaddr = *ipaddr;
    local_socket_descriptors[ipaugenblick_socket->connection_idx].remote_port = *port;
    local_socket_descriptors[ipaugenblick_socket->connection_idx].local_ipaddr = 
	local_socket_descriptors[sock].local_ipaddr;
    local_socket_descriptors[ipaugenblick_socket->connection_idx].local_port = 
	local_socket_descriptors[sock].local_port;
printf("%s %d %p %d %d %x %d\n",__FILE__,__LINE__,accepted_socket,sock,ipaugenblick_socket->connection_idx,*ipaddr,*port);
    local_socket_descriptors[ipaugenblick_socket->connection_idx].socket = ipaugenblick_socket;
    cmd->cmd = IPAUGENBLICK_SET_SOCKET_RING_COMMAND;
    cmd->ringset_idx = ipaugenblick_socket->connection_idx;
    cmd->parent_idx = 0;
    cmd->u.set_socket_ring.socket_descr = accepted_socket;
    if(ipaugenblick_enqueue_command_buf(cmd)) {
        ipaugenblick_free_command_buf(cmd);
        printf("CANNOT ENQUEUE SET_RING_COMMAND\n");
        return -2;
    }
    return ipaugenblick_socket->connection_idx;
}

void ipaugenblick_getsockname(int sock,int is_local,unsigned int *ipaddr,unsigned short *port)
{
	if(is_local) {
		*ipaddr = local_socket_descriptors[sock].socket->local_ipaddr;
		*port = local_socket_descriptors[sock].socket->local_port;
		//*ipaddr = local_socket_descriptors[sock].local_ipaddr;
		//*port = local_socket_descriptors[sock].local_port;
	}
	else {
		*ipaddr = local_socket_descriptors[sock].remote_ipaddr;
		*port = local_socket_descriptors[sock].remote_port;
	}
}

static inline void ipaugenblick_free_common_notification_buf(ipaugenblick_cmd_t *cmd)
{
    rte_mempool_put(free_command_pool,(void *)cmd);
}

int ipaugenblick_is_connected(int sock)
{
    return local_socket_descriptors[sock].any_event_received;
}

int ipaugenblick_select(int selector,unsigned short *mask,int timeout)
{
    uint32_t ringset_idx_and_ready_mask;
    uint32_t i;
    ipaugenblick_cmd_t *cmd;
    uint32_t iterations_to_wait;
    ipaugenblick_stats_select_called++;
restart_waiting:
    
    if(rte_ring_dequeue(selectors[selector].ready_connections,(void **)&ringset_idx_and_ready_mask)) { 
        if(timeout > 0) {
            for(i = 0;i < timeout;i++) rte_pause();
        }
        else if(timeout == 0) {
            return -1;
        }
        else 
            usleep(1);
        goto restart_waiting;
    }
    ipaugenblick_stats_select_returned++;
    *mask = ringset_idx_and_ready_mask >> SOCKET_READY_SHIFT;
    if((ringset_idx_and_ready_mask & SOCKET_READY_MASK) >= IPAUGENBLICK_CONNECTION_POOL_SIZE) {
        printf("FATAL ERROR %s %d %d\n",__FILE__,__LINE__,ringset_idx_and_ready_mask & SOCKET_READY_MASK);
        exit(0);
    }
    local_socket_descriptors[ringset_idx_and_ready_mask & SOCKET_READY_MASK].any_event_received = 1;
       
    return ringset_idx_and_ready_mask & SOCKET_READY_MASK;
}

/* receive functions return a chained buffer. this function
   retrieves a next chunk and its length */
void *ipaugenblick_get_next_buffer_segment(void *buffer,int *len)
{
   struct rte_mbuf *mbuf = RTE_MBUF(buffer);
   
   mbuf = mbuf->pkt.next;
   if(!mbuf) {
       return NULL;
   }
   *len = mbuf->pkt.data_len;
   return &(mbuf->pkt.data);
}

static inline int ipaugenblick_route_command(int opcode,unsigned int ipaddr,unsigned int mask,unsigned int nexthop,short metric)
{
    ipaugenblick_cmd_t *cmd;
    cmd = ipaugenblick_get_free_command_buf();
    if(!cmd) {
        ipaugenblick_stats_cannot_allocate_cmd++;
        return -1;
    } 
    cmd->cmd = opcode;
    cmd->u.route.dest_ipaddr = ipaddr;
    cmd->u.route.metric = metric;
    cmd->u.route.dest_mask = mask;
    cmd->u.route.next_hop = nexthop;
    if(ipaugenblick_enqueue_command_buf(cmd)) {
        ipaugenblick_free_command_buf(cmd);
        return -2;
    }
    return 0;
}

int ipaugenblick_add_v4_route(unsigned int ipaddr,unsigned int mask,unsigned int nexthop,short metric)
{
    return ipaugenblick_route_command(IPAUGENBLICK_ROUTE_ADD_COMMAND,ipaddr,mask,nexthop,metric);
}

int ipaugenblick_del_v4_route(unsigned int ipaddr,unsigned int mask,unsigned int nexthop)
{
    return ipaugenblick_route_command(IPAUGENBLICK_ROUTE_DEL_COMMAND,ipaddr,mask,nexthop,0);	
}

int ipaugenblick_setsockopt(int sock, int level, int optname,char *optval, unsigned int optlen)
{
    ipaugenblick_cmd_t *cmd;

    cmd = ipaugenblick_get_free_command_buf();
    if(!cmd) {
        ipaugenblick_stats_cannot_allocate_cmd++;
        return -1;
    } 
    cmd->cmd = IPAUGENBLICK_SETSOCKOPT_COMMAND;
    cmd->u.setsockopt.level = level;
    cmd->u.setsockopt.optname = optname;
    cmd->u.setsockopt.optlen = optlen;
    rte_memcpy(&cmd->u.setsockopt.optval,optval,optlen);
    if(ipaugenblick_enqueue_command_buf(cmd)) {
        ipaugenblick_free_command_buf(cmd);
        return -2;
    }
    return 0;
}
