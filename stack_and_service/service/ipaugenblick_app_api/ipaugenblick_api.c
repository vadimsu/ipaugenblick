#define _GNU_SOURCE
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
#include "ipaugenblick_log.h"
#include <sched.h>

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
    TAILQ_HEAD(local_read_ready_cache, _local_socket_descriptor_) local_ready_cache;
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
uint64_t ipaugenblick_stats_rx_returned = 0;
pthread_t stats_thread;
uint8_t g_print_stats_loop = 1;
uint32_t g_client_ringset_idx = IPAUGENBLICK_CONNECTION_POOL_SIZE;
ipaugenblick_update_cbk_t ipaugenblick_update_cbk = NULL;

void print_stats()
{
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"ipaugenblick_stats_receive_called %lu ipaugenblick_stats_send_called %lu \n\t\
                ipaugenblick_stats_rx_kicks_sent %lu ipaugenblick_stats_tx_kicks_sent %lu ipaugenblick_stats_cannot_allocate_cmd %lu  \n\t\
                ipaugenblick_stats_rx_full %lu ipaugenblick_stats_rx_dequeued %lu ipaugenblick_stats_rx_dequeued_local %lu \n\t\
                ipaugenblick_stats_select_called %lu ipaugenblick_stats_select_returned %lu ipaugenblick_stats_tx_buf_allocation_failure %lu \n\t\
                ipaugenblick_stats_send_failure %lu ipaugenblick_stats_recv_failure %lu ipaugenblick_stats_buffers_sent %lu ipaugenblick_stats_buffers_allocated %lu ipaugenblick_stats_rx_dequeued %lu\n",
                ipaugenblick_stats_receive_called,ipaugenblick_stats_send_called,ipaugenblick_stats_rx_kicks_sent,
                ipaugenblick_stats_tx_kicks_sent,ipaugenblick_stats_cannot_allocate_cmd,ipaugenblick_stats_rx_full,ipaugenblick_stats_rx_dequeued,
                ipaugenblick_stats_rx_dequeued_local,ipaugenblick_stats_select_called,ipaugenblick_stats_select_returned,ipaugenblick_stats_tx_buf_allocation_failure,
                ipaugenblick_stats_send_failure,ipaugenblick_stats_recv_failure,
                ipaugenblick_stats_buffers_sent,
                ipaugenblick_stats_buffers_allocated,
		ipaugenblick_stats_rx_dequeued);
}

void print_rings_stats()
{
	int i;

	for(i = 0;i < IPAUGENBLICK_CONNECTION_POOL_SIZE;i++) {	
		if(local_socket_descriptors[i].socket) {
			ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"Connection #%d\n",i);
        		ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"rx ring count %d tx ring count %d",
				rte_ring_count(local_socket_descriptors[i].rx_ring),
				rte_ring_count(local_socket_descriptors[i].tx_ring));
			ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"local cache count %d",
				rte_ring_count(local_socket_descriptors[i].local_cache));
			ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"read_ready %d write_ready %d",
				local_socket_descriptors[i].socket->read_ready_to_app.cnt,
				local_socket_descriptors[i].socket->write_ready_to_app.cnt);
		}
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
//printf("%s %d %d\n",__FILE__,__LINE__,signum);
    if(signum == SIGUSR1) {
        /* T.B.D. do something to wake up the thread */
        return;
    }

    ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"terminating on signal %d\n",signum);
    print_rings_stats();

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
    print_stats();
    kill(getpid(),signum);
}

/* must be called per process */
int ipaugenblick_app_init(int argc,char **argv,char *app_unique_id)
{
    int i;
    char ringname[1024];
    rte_cpuset_t cpuset;
    unsigned cpu;

    ipaugenblick_log_init(0);

    if(rte_eal_init(argc, argv) < 0) {
        ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot initialize rte_eal");
	return -1;
    }
    ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"EAL initialized\n");

    free_clients_ring = rte_ring_lookup(FREE_CLIENTS_RING);
    if(!free_clients_ring) {
        ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot find ring %s %d\n",__FILE__,__LINE__);
        exit(0);
    }

    free_connections_ring = rte_ring_lookup(FREE_CONNECTIONS_RING);

    if(!free_connections_ring) {
        ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot find free connections ring\n");
        return -1;
    }

    free_connections_pool = rte_mempool_lookup(FREE_CONNECTIONS_POOL_NAME);

    if(!free_connections_pool) {
        ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot find free connections pool\n");
        return -1;
    }

    memset(local_socket_descriptors,0,sizeof(local_socket_descriptors));
    for(i = 0;i < IPAUGENBLICK_CONNECTION_POOL_SIZE;i++) {
        sprintf(ringname,RX_RING_NAME_BASE"%d",i);
        local_socket_descriptors[i].rx_ring = rte_ring_lookup(ringname);
        if(!local_socket_descriptors[i].rx_ring) {
            ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"%s %d\n",__FILE__,__LINE__);
            exit(0);
        }
        sprintf(ringname,TX_RING_NAME_BASE"%d",i);
        local_socket_descriptors[i].tx_ring = rte_ring_lookup(ringname);
        if(!local_socket_descriptors[i].tx_ring) {
            ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"%s %d\n",__FILE__,__LINE__);
            exit(0);
        }
        local_socket_descriptors[i].select = -1;
        local_socket_descriptors[i].socket = NULL;
        sprintf(ringname,"lrxcache%s_%d",app_unique_id,i);
        ipaugenblick_log(IPAUGENBLICK_LOG_DEBUG,"local cache name %s\n",ringname);
        local_socket_descriptors[i].local_cache = rte_ring_create(ringname, 16384,rte_socket_id(), RING_F_SC_DEQ|RING_F_SP_ENQ);
        if(!local_socket_descriptors[i].local_cache) {
           ipaugenblick_log(IPAUGENBLICK_LOG_WARNING,"cannot create local cache\n");
	   local_socket_descriptors[i].local_cache = rte_ring_lookup(ringname);
	   if(!local_socket_descriptors[i].local_cache) {
		ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"and cannot find\n");
		exit(0);
	   } 
        }
	local_socket_descriptors[i].any_event_received = 0;
    }
    tx_bufs_pool = rte_mempool_lookup("mbufs_mempool");
    if(!tx_bufs_pool) {
        ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot find tx bufs pool\n");
        return -1;
    }
    
    free_command_pool = rte_mempool_lookup(FREE_COMMAND_POOL_NAME);
    if(!free_command_pool) {
        ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot find free command pool\n");
        return -1;
    }
    
    command_ring = rte_ring_lookup(COMMAND_RING_NAME);
    if(!command_ring) {
        ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot find command ring\n");
        return -1;
    }
    rx_bufs_ring = rte_ring_lookup("rx_mbufs_ring");
    if(!rx_bufs_ring) {
        ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot find rx bufs ring\n");
        return -1;
    }
    selectors_ring = rte_ring_lookup(SELECTOR_RING_NAME);
    
    for(i = 0;i < IPAUGENBLICK_SELECTOR_POOL_SIZE;i++) {
        sprintf(ringname,"SELECTOR_RING_NAME%d",i);
        selectors[i].ready_connections = rte_ring_lookup(ringname);
        if(!selectors[i].ready_connections) {
            ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot find ring %s %d\n",__FILE__,__LINE__);
            exit(0);
        }
	TAILQ_INIT(&selectors[i].local_ready_cache);
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
    
    RTE_LCORE_FOREACH(cpu) {
	if(rte_lcore_is_enabled(cpu)) {
		CPU_ZERO(&cpuset);
//		memset(&cpuset,0,sizeof(cpuset));
//		unsigned char *p = (unsigned char *)&cpuset;
		CPU_SET(cpu,&cpuset);
		//*p = cpu;
		if(!rte_thread_set_affinity(&cpuset))
			break;
		ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot set thread affinity for %d %s %d\n",cpu,__FILE__,__LINE__);
	}
    }
    return ((tx_bufs_pool == NULL)||(command_ring == NULL)||(free_command_pool == NULL));
}

int ipaugenblick_create_client(ipaugenblick_update_cbk_t update_cbk)
{
    int ringset_idx;
    ipaugenblick_cmd_t *cmd;
    char ringname[1024];

    if(rte_ring_dequeue(free_clients_ring,(void **)&ringset_idx)) {
        ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"%s %d\n",__FILE__,__LINE__);
        return -1;
    }
    sprintf(ringname,"%s%d",FREE_CLIENTS_RING,ringset_idx);
    client_ring = rte_ring_lookup(ringname);
    if(!client_ring) {
	ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"%s %d\n",__FILE__,__LINE__);
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
		//ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"%s %d\n",__FILE__,__LINE__);
		return -1;
	}
	unsigned char *p = rte_pktmbuf_mtod(mbuf, unsigned char *);
	switch(*p) {
		case IPAUGENBLICK_NEW_IFACES:
		if(ipaugenblick_update_cbk) {
			cmd = 1;
			p++;
			ipaugenblick_update_cbk(cmd,p,rte_pktmbuf_data_len(mbuf) - 1);
		}
		break;
		case IPAUGENBLICK_NEW_ADDRESSES:
		if(ipaugenblick_update_cbk) {
			cmd = 3;
			p++;
			ipaugenblick_update_cbk(cmd,p,rte_pktmbuf_data_len(mbuf) - 1);
		}
		break;
		case IPAUGENBLICK_END_OF_RECORD:
		return 0;
	}
	return -1;
}

int ipaugenblick_open_select(void)
{
    int ringset_idx;

    if(rte_ring_dequeue(selectors_ring,(void **)&ringset_idx)) {
        ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"%s %d\n",__FILE__,__LINE__);
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
        	ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"%s %d\n",__FILE__,__LINE__);
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
	cmd->u.open_sock.pid = getpid();
	if(ipaugenblick_enqueue_command_buf(cmd)) {
        	ipaugenblick_free_command_buf(cmd);
	        return -3;
    	}

    	local_socket_descriptors[ipaugenblick_socket->connection_idx].socket = ipaugenblick_socket;
	if(parent != -1)
		local_socket_descriptors[ipaugenblick_socket->connection_idx].select = parent;

    	return ipaugenblick_socket->connection_idx;
}

int ipaugenblick_bind(int sock, struct sockaddr *addr, __rte_unused int addrlen)
{
	ipaugenblick_cmd_t *cmd;
	struct sockaddr_in *sockaddr_in = (struct sockaddr_in *)addr;

	if (sockaddr_in->sin_family != AF_INET) /*only IPv4 for now*/
		return -1;

	cmd = ipaugenblick_get_free_command_buf();
    	if(!cmd) {
        	ipaugenblick_stats_cannot_allocate_cmd++;
        	return -1;
    	}
 
	cmd->cmd = IPAUGENBLICK_SOCKET_CONNECT_BIND_COMMAND;
	cmd->ringset_idx = sock;
	cmd->parent_idx = local_socket_descriptors[sock].select;
	cmd->u.socket_connect_bind.ipaddr = sockaddr_in->sin_addr.s_addr;
        cmd->u.socket_connect_bind.port = sockaddr_in->sin_port;
	cmd->u.socket_connect_bind.is_connect = 0;
    	if(ipaugenblick_enqueue_command_buf(cmd)) {
       		ipaugenblick_free_command_buf(cmd);
		return -2;
    	}
	local_socket_descriptors[sock].local_ipaddr = sockaddr_in->sin_addr.s_addr;
	local_socket_descriptors[sock].local_port = sockaddr_in->sin_port;

	return 0;
}

int ipaugenblick_connect(int sock,struct sockaddr *addr, __rte_unused int addrlen)
{
	ipaugenblick_cmd_t *cmd;
	struct sockaddr_in *sockaddr_in = (struct sockaddr_in *)addr;

	if (sockaddr_in->sin_family != AF_INET) /*only IPv4 for now*/
		return -1;
	cmd = ipaugenblick_get_free_command_buf();
    	if(!cmd) {
        	ipaugenblick_stats_cannot_allocate_cmd++;
        	return -2;
    	}
	cmd->cmd = IPAUGENBLICK_SOCKET_CONNECT_BIND_COMMAND;
	cmd->ringset_idx = sock;
	cmd->parent_idx = local_socket_descriptors[sock].select;
	cmd->u.socket_connect_bind.ipaddr = sockaddr_in->sin_addr.s_addr;
        cmd->u.socket_connect_bind.port = sockaddr_in->sin_port;
	cmd->u.socket_connect_bind.is_connect = 1;
    	if(ipaugenblick_enqueue_command_buf(cmd)) {
       		ipaugenblick_free_command_buf(cmd);
		return -2;
    	}
	local_socket_descriptors[sock].remote_ipaddr = sockaddr_in->sin_addr.s_addr;
	local_socket_descriptors[sock].remote_port = sockaddr_in->sin_port;
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
    if ((local_socket_descriptors[sock].select != -1)&&
	(local_socket_descriptors[sock].present_in_ready_cache)) {
		TAILQ_REMOVE(&selectors[local_socket_descriptors[sock].select].local_ready_cache,
			     &local_socket_descriptors[sock],
			     local_ready_cache_entry);
		local_socket_descriptors[sock].present_in_ready_cache = 0;
	}
}

static inline void ipaugenblick_notify_empty_tx_buffers(int sock)
{
    ipaugenblick_cmd_t *cmd;
    if (sock == -1) //anonimous allocation
	return;
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
//    printf("sock %d ring space %d free bufs %d tx space %d\n",sock,ring_space,free_bufs_count,tx_space);
    if(rc > tx_space)
	rc = tx_space;
    if(rc < 10) {
	rte_atomic16_set(&(local_socket_descriptors[sock & SOCKET_READY_MASK].socket->write_ready_to_app),0);
        ipaugenblick_notify_empty_tx_buffers(sock);
    }
    return rc;
}

int ipaugenblick_get_socket_tx_space_own_buffer(int sock)
{
    int ring_space = ipaugenblick_socket_tx_space(sock); 
    int tx_space = rte_atomic32_read(&(local_socket_descriptors[sock & SOCKET_READY_MASK].socket->tx_space))/1448;
    int rc = (ring_space > tx_space) ? tx_space : ring_space;
    if(!rc) {
	rte_atomic16_set(&(local_socket_descriptors[sock & SOCKET_READY_MASK].socket->write_ready_to_app),0);
        ipaugenblick_notify_empty_tx_buffers(sock);
    }
    return rc;
}
//#define DEBUG_SOCKET_SELECTOR
#ifdef DEBUG_SOCKET_SELECTOR
#define IPAUGENBLICK_UPDATE_LOCAL_READY_CACHE(a,b,c) ipaugenblick_update_local_ready_cache(a,b,c,__func__,__LINE__)
static inline void ipaugenblick_update_local_ready_cache(int selector_idx,local_socket_descriptor_t *local_socket_descr,int bit,char *func,int line)
#else
#define IPAUGENBLICK_UPDATE_LOCAL_READY_CACHE(a,b,c) ipaugenblick_update_local_ready_cache(a,b,c)
static inline void ipaugenblick_update_local_ready_cache(int selector_idx,local_socket_descriptor_t *local_socket_descr,int bit)
#endif
{
	if (selector_idx == -1) {
		ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"%s %d\n",__func__,__LINE__);
		return;
	}
	//ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"%s %d %x %x %d %p %s %d\n",__func__,__LINE__,local_socket_descr->local_mask,bit,local_socket_descr->present_in_ready_cache,local_socket_descr,func, line);
	local_socket_descr->local_mask &= ~(bit);
	if (local_socket_descr->local_mask)
		return;
	if (local_socket_descr->present_in_ready_cache) {
		TAILQ_REMOVE(&selectors[selector_idx].local_ready_cache,local_socket_descr,local_ready_cache_entry);
		local_socket_descr->present_in_ready_cache = 0;
	}
}
/* TCP or connected UDP */
inline int ipaugenblick_send(int sock,void *pdesc,int offset,int length)
{
    int rc;
    struct rte_mbuf *mbuf = (struct rte_mbuf *)pdesc;
    ipaugenblick_stats_send_called++;
    rte_pktmbuf_data_len(mbuf) = length;
    mbuf->next = NULL;
    rte_atomic16_set(&(local_socket_descriptors[sock & SOCKET_READY_MASK].socket->write_ready_to_app),0);
    rc = ipaugenblick_enqueue_tx_buf(sock,mbuf);
    if(rc == 0)
        rte_atomic32_sub(&(local_socket_descriptors[sock & SOCKET_READY_MASK].socket->tx_space),length);
    else
	IPAUGENBLICK_UPDATE_LOCAL_READY_CACHE(local_socket_descriptors[sock].select,&local_socket_descriptors[sock],SOCKET_WRITABLE_BIT);
    ipaugenblick_stats_send_failure += (rc != 0);
    return rc;
}

inline int ipaugenblick_send_bulk(int sock,struct data_and_descriptor *bufs_and_desc,int *offsets,int *lengths,int buffer_count)
{
    int rc,idx,total_length = 0;
    struct rte_mbuf *mbufs[buffer_count];

    for(idx = 0;idx < buffer_count;idx++) {
        /* TODO: set offsets */
        mbufs[idx] = (struct rte_mbuf *)bufs_and_desc[idx].pdesc;
        rte_pktmbuf_data_len(mbufs[idx]) = lengths[idx];
	total_length += lengths[idx];
    }
    ipaugenblick_stats_send_called++;
    ipaugenblick_stats_buffers_sent += buffer_count;
    rte_atomic16_set(&(local_socket_descriptors[sock].socket->write_ready_to_app),0);
    rc = ipaugenblick_enqueue_tx_bufs_bulk(sock,mbufs,buffer_count);
//printf("%s %d %p %d %d %d\n",__FILE__,__LINE__,local_socket_descriptors[sock & SOCKET_READY_MASK].socket,sock & SOCKET_READY_MASK,local_socket_descriptors[sock & SOCKET_READY_MASK].socket->tx_space,total_length);
    if(rc == 0)
	rte_atomic32_sub(&(local_socket_descriptors[sock].socket->tx_space),total_length);
    else
	IPAUGENBLICK_UPDATE_LOCAL_READY_CACHE(local_socket_descriptors[sock].select,&local_socket_descriptors[sock],SOCKET_WRITABLE_BIT);
    ipaugenblick_stats_send_failure += (rc != 0);
    return rc;
}

/* UDP or RAW */
int ipaugenblick_sendto(int sock,void *pdesc,int offset,int length, struct sockaddr *addr, __rte_unused int addrlen)
{
    int rc;
    struct rte_mbuf *mbuf = (struct rte_mbuf *)pdesc;
    char *p_addr = rte_pktmbuf_mtod(mbuf,char *);
    struct sockaddr_in *p_addr_in, *in_addr = (struct sockaddr_in *)addr;
    ipaugenblick_stats_send_called++;
    rte_pktmbuf_data_len(mbuf) = length;
    p_addr -= sizeof(struct sockaddr_in);
    p_addr_in = (struct sockaddr_in *)p_addr;
    *p_addr_in = *in_addr;
    rte_atomic16_set(&(local_socket_descriptors[sock & SOCKET_READY_MASK].socket->write_ready_to_app),0);
    rc = ipaugenblick_enqueue_tx_buf(sock,mbuf);
    if(rc == 0)
        rte_atomic32_sub(&(local_socket_descriptors[sock & SOCKET_READY_MASK].socket->tx_space),length);
    else
	IPAUGENBLICK_UPDATE_LOCAL_READY_CACHE(local_socket_descriptors[sock].select,&local_socket_descriptors[sock],SOCKET_WRITABLE_BIT);
    ipaugenblick_stats_send_failure += (rc != 0);
    return rc;
}

inline int ipaugenblick_sendto_bulk(int sock,struct data_and_descriptor *bufs_and_desc,int *offsets,int *lengths, struct sockaddr *addr,__rte_unused int addrlen,int buffer_count)
{
    int rc,idx,total_length = 0;
    struct rte_mbuf *mbufs[buffer_count];
    struct sockaddr_in *in_addr;

    for(idx = 0;idx < buffer_count;idx++) {
        char *p_addr;
        struct sockaddr_in *p_addr_in;
        /* TODO: set offsets */
        mbufs[idx] = (struct rte_mbuf *)bufs_and_desc[idx].pdesc;
        rte_pktmbuf_data_len(mbufs[idx]) = lengths[idx];
	total_length += lengths[idx];
        p_addr = rte_pktmbuf_mtod(mbufs[idx],char *);
        
        rte_pktmbuf_data_len(mbufs[idx]) = lengths[idx];
        p_addr -= sizeof(struct sockaddr_in);
        p_addr_in = (struct sockaddr_in *)p_addr;
	in_addr = (struct sockaddr_in *)addr;
	*p_addr_in = *in_addr;
	addr++;
    }
    ipaugenblick_stats_send_called++;
    ipaugenblick_stats_buffers_sent += buffer_count;
    rte_atomic16_set(&(local_socket_descriptors[sock & SOCKET_READY_MASK].socket->write_ready_to_app),0);
    rc = ipaugenblick_enqueue_tx_bufs_bulk(sock,mbufs,buffer_count);
    if(rc == 0)
	rte_atomic32_sub(&(local_socket_descriptors[sock & SOCKET_READY_MASK].socket->tx_space),total_length);
    else
	IPAUGENBLICK_UPDATE_LOCAL_READY_CACHE(local_socket_descriptors[sock].select,&local_socket_descriptors[sock],SOCKET_WRITABLE_BIT);
    ipaugenblick_stats_send_failure += (rc != 0);
    return rc;
}

static inline struct rte_mbuf *ipaugenblick_get_from_shadow(int sock)
{
	struct rte_mbuf *mbuf = NULL;

	if(local_socket_descriptors[sock].shadow) {
		mbuf = local_socket_descriptors[sock].shadow;
		local_socket_descriptors[sock].shadow = NULL;
		rte_pktmbuf_data_len(mbuf) = local_socket_descriptors[sock].shadow_len_remainder;
		mbuf->data_off += local_socket_descriptors[sock].shadow_len_delievered;
		if(mbuf->next) {
			rte_pktmbuf_pkt_len(mbuf) = rte_pktmbuf_data_len(mbuf) + rte_pktmbuf_pkt_len(mbuf->next);
		}
		else {
			rte_pktmbuf_pkt_len(mbuf) = rte_pktmbuf_data_len(mbuf);
		}
		mbuf->next = local_socket_descriptors[sock].shadow_next;
		local_socket_descriptors[sock].shadow_next = NULL;
	}
	return mbuf;
}

static inline void ipaugenblick_try_read_exact_amount(struct rte_mbuf *mbuf,int sock,int *total_len,int *first_segment_len)
{
	struct rte_mbuf *tmp = mbuf,*prev = NULL;
	int curr_len = 0;
	while((tmp)&&((rte_pktmbuf_data_len(tmp) + curr_len) < *total_len)) {
		curr_len += rte_pktmbuf_data_len(tmp);
//		printf("%s %d %d\n",__FILE__,__LINE__,rte_pktmbuf_data_len(tmp));
		prev = tmp;
		tmp = tmp->next;
	}
	if(tmp) {
//		printf("%s %d %d\n",__FILE__,__LINE__,rte_pktmbuf_data_len(tmp));
		if((curr_len + rte_pktmbuf_data_len(tmp)) > *total_len) { /* more data remains */
			local_socket_descriptors[sock].shadow = tmp;
			local_socket_descriptors[sock].shadow_next = tmp->next;
			local_socket_descriptors[sock].shadow_len_remainder = (curr_len + rte_pktmbuf_data_len(tmp)) - *total_len;
			local_socket_descriptors[sock].shadow_len_delievered = 
				rte_pktmbuf_data_len(tmp) - local_socket_descriptors[sock].shadow_len_remainder;
			rte_pktmbuf_data_len(tmp) = local_socket_descriptors[sock].shadow_len_delievered;
			*first_segment_len = local_socket_descriptors[sock].shadow_len_delievered;
		}
		/* if less data than required, tmp is NULL. we're here that means exact amount is read */	
		else {
			*first_segment_len = rte_pktmbuf_data_len(mbuf);
			/* store next mbuf, if there is */
			if(tmp->next) {
				local_socket_descriptors[sock].shadow = tmp->next;
				local_socket_descriptors[sock].shadow_next = tmp->next->next;
				if(local_socket_descriptors[sock].shadow_next) {
					local_socket_descriptors[sock].shadow_len_remainder = 
						rte_pktmbuf_data_len(local_socket_descriptors[sock].shadow_next);
					local_socket_descriptors[sock].shadow_len_delievered = 0;
				}
			}
		}
		tmp->next = NULL;
		if(curr_len == *total_len) {
			if(prev)
				prev->next = NULL;
		}	
	}
	else {
		*total_len = curr_len;
		*first_segment_len = rte_pktmbuf_data_len(mbuf);
	}		
}

/* TCP */
int ipaugenblick_receive(int sock,void **pbuffer,int *total_len,int *first_segment_len,void **pdesc)
{ 
    struct rte_mbuf *mbuf;

    ipaugenblick_stats_receive_called++;
    /* first try to look shadow. shadow pointer saved when last mbuf delievered partially */
    mbuf = ipaugenblick_get_from_shadow(sock);
    if((mbuf)&&(*total_len > 0)) { /* total_len > 0 means user restricts total read count */
	int total_len2 = *total_len;
	/* now find mbuf (if any) to be delievered partially and save it to shadown */
	ipaugenblick_try_read_exact_amount(mbuf,sock,&total_len2,first_segment_len);
	*pbuffer = rte_pktmbuf_mtod(mbuf,void *);
    	*pdesc = mbuf;
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
			last_mbuf->next = mbuf2;
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
		if(local_socket_descriptors[sock].select != -1)
			rte_ring_enqueue(selectors[local_socket_descriptors[sock].select].ready_connections,(void *)ringidx_ready_mask);	
	}
	return 0;
    }
    if(mbuf) { /* means user does not care how much to read. Try to read all */
	    struct rte_mbuf *mbuf2 = ipaugenblick_dequeue_rx_buf(sock);
	    struct rte_mbuf *last_mbuf = rte_pktmbuf_lastseg(mbuf);
	    last_mbuf->next = mbuf2;
	    if(mbuf2)
		    rte_pktmbuf_pkt_len(mbuf) += rte_pktmbuf_pkt_len(mbuf2);
	    *total_len = rte_pktmbuf_pkt_len(mbuf);
	    *first_segment_len = rte_pktmbuf_data_len(mbuf);
	    *pbuffer = rte_pktmbuf_mtod(mbuf,void *);
    	    *pdesc = mbuf;
	    return 0;
    }
read_from_ring:
    /* either user cares how much to read or/and shadow is empty */
    mbuf = ipaugenblick_dequeue_rx_buf(sock);
    if(mbuf) {
	if(*total_len > 0) { /* read exact */
		int total_len2 = *total_len;
		ipaugenblick_try_read_exact_amount(mbuf,sock,&total_len2,first_segment_len);
		*total_len = total_len2;
		if(local_socket_descriptors[sock].shadow) {
			uint32_t ringidx_ready_mask = sock|(SOCKET_READABLE_BIT << SOCKET_READY_SHIFT);
			if(local_socket_descriptors[sock].select != -1)
				rte_ring_enqueue(selectors[local_socket_descriptors[sock].select].ready_connections,(void *)ringidx_ready_mask);	
		}
	}
	else {
//		printf("%s %d %d\n",__FILE__,__LINE__,mbuf->pkt.pkt_len);
		*total_len = rte_pktmbuf_pkt_len(mbuf);
		*first_segment_len = rte_pktmbuf_data_len(mbuf);
		ipaugenblick_stats_rx_returned += mbuf->nb_segs;
	}
	*pbuffer = rte_pktmbuf_mtod(mbuf,void *);
    	*pdesc = mbuf;
	return 0;
    }
    IPAUGENBLICK_UPDATE_LOCAL_READY_CACHE(local_socket_descriptors[sock & SOCKET_READY_MASK].select,&local_socket_descriptors[sock & SOCKET_READY_MASK],SOCKET_READABLE_BIT);
    ipaugenblick_stats_recv_failure++;
    return -1;
}

/* UDP or RAW */
int ipaugenblick_receivefrom(int sock,void **buffer,int *len,struct sockaddr *addr,__rte_unused int *addrlen, void **pdesc)
{
    struct rte_mbuf *mbuf = ipaugenblick_dequeue_rx_buf(sock);
    struct sockaddr_in *in_addr = (struct sockaddr_in *)addr;
    ipaugenblick_stats_receive_called++;

    if(!mbuf) {
	IPAUGENBLICK_UPDATE_LOCAL_READY_CACHE(local_socket_descriptors[sock].select,&local_socket_descriptors[sock],SOCKET_READABLE_BIT);
        ipaugenblick_stats_recv_failure++;
        return -1;
    }
    *buffer = rte_pktmbuf_mtod(mbuf,void *);
    *pdesc = mbuf;
    *len = rte_pktmbuf_pkt_len(mbuf);
    char *p_addr = rte_pktmbuf_mtod(mbuf,char *);
    p_addr -= sizeof(struct sockaddr_in);
    struct sockaddr_in *p_addr_in = (struct sockaddr_in *)p_addr;
    *in_addr = *p_addr_in;
    return 0;
}

/* Allocate buffer to use later in *send* APIs */
void *ipaugenblick_get_buffer(int length,int owner_sock,void **pdesc)
{
    struct rte_mbuf *mbuf;
    mbuf = rte_pktmbuf_alloc(tx_bufs_pool);
    if(!mbuf) {
        ipaugenblick_notify_empty_tx_buffers(owner_sock);
        ipaugenblick_stats_tx_buf_allocation_failure++; 
        return NULL;
    }
    ipaugenblick_stats_buffers_allocated++;
    *pdesc = mbuf;
    return rte_pktmbuf_mtod(mbuf,void *);
}

int ipaugenblick_get_buffers_bulk(int length,int owner_sock,int count,struct data_and_descriptor *bufs_and_desc)
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
        bufs_and_desc[idx].pdata = rte_pktmbuf_mtod(mbufs[idx],void *);
	bufs_and_desc[idx].pdesc = mbufs[idx];
    } 
    ipaugenblick_stats_buffers_allocated += count;
    return 0;
}

/* release buffer when either send is complete or receive has done with the buffer */
void ipaugenblick_release_tx_buffer(void *pdesc)
{
    struct rte_mbuf *mbuf = (struct rte_mbuf *)pdesc;

    rte_pktmbuf_free_seg(mbuf);
}

void ipaugenblick_release_rx_buffer(void *pdesc,int sock)
{
    struct rte_mbuf *mbuf = (struct rte_mbuf *)pdesc; 
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
            next = mbuf->next;
            if(likely(__rte_pktmbuf_prefree_seg(mbuf) != NULL)) {
                mbufs[count++] = mbuf; 
                mbuf->next = NULL; 
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
	usleep(1);
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

int ipaugenblick_accept(int sock, struct sockaddr *addr, __rte_unused int *addrlen)
{
    ipaugenblick_cmd_t *cmd;
    ipaugenblick_socket_t *ipaugenblick_socket;
    unsigned long accepted_socket;
    struct sockaddr_in *in_addr = (struct sockaddr_in *)addr;
ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"%s %d %d\n",__func__,__LINE__,sock);
    rte_atomic16_set(&(local_socket_descriptors[sock].socket->read_ready_to_app),0);
    if(rte_ring_dequeue(local_socket_descriptors[sock].rx_ring,(void **)&cmd)) {
	IPAUGENBLICK_UPDATE_LOCAL_READY_CACHE(local_socket_descriptors[sock].select,&local_socket_descriptors[sock],SOCKET_READABLE_BIT);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"%s %d %d\n",__func__,__LINE__,sock);
        return -1;
    }
    
    if(rte_ring_dequeue(free_connections_ring,(void **)&ipaugenblick_socket)) {
        ipaugenblick_free_command_buf(cmd);
	ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"NO FREE CONNECTIONS\n");
        return -1;
    } 
    accepted_socket = cmd->u.accepted_socket.socket_descr;
    in_addr->sin_family = AF_INET;
    in_addr->sin_addr.s_addr = cmd->u.accepted_socket.ipaddr;
    in_addr->sin_port  = cmd->u.accepted_socket.port;
    local_socket_descriptors[ipaugenblick_socket->connection_idx].remote_ipaddr = in_addr->sin_addr.s_addr;
    local_socket_descriptors[ipaugenblick_socket->connection_idx].remote_port = in_addr->sin_port;
    local_socket_descriptors[ipaugenblick_socket->connection_idx].local_ipaddr = 
	local_socket_descriptors[sock].local_ipaddr;
    local_socket_descriptors[ipaugenblick_socket->connection_idx].local_port = 
	local_socket_descriptors[sock].local_port;
ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"%s %d acpt sock %p listen sock %d accpt idx %d rmt ip %x port %d local ip %x port %d\n",__func__,__LINE__,(void *)accepted_socket,sock,(int)ipaugenblick_socket->connection_idx,in_addr->sin_addr.s_addr,in_addr->sin_port,
local_socket_descriptors[ipaugenblick_socket->connection_idx].local_ipaddr,
local_socket_descriptors[ipaugenblick_socket->connection_idx].local_port);
    local_socket_descriptors[ipaugenblick_socket->connection_idx].socket = ipaugenblick_socket;
    cmd->cmd = IPAUGENBLICK_SET_SOCKET_RING_COMMAND;
    cmd->ringset_idx = ipaugenblick_socket->connection_idx;
    cmd->parent_idx = -1;
    cmd->u.set_socket_ring.socket_descr = accepted_socket;
    cmd->u.set_socket_ring.pid = getpid();
    if(ipaugenblick_enqueue_command_buf(cmd)) {
        ipaugenblick_free_command_buf(cmd);
        ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"CANNOT ENQUEUE SET_RING_COMMAND\n");
        return -2;
    }
    return ipaugenblick_socket->connection_idx;
}

void ipaugenblick_getsockname(int sock,int is_local, struct sockaddr *addr, __rte_unused int *addrlen)
{
	struct sockaddr_in *in_addr = (struct sockaddr_in *)addr;
	in_addr->sin_family = AF_INET;
	if(is_local) {
		if (local_socket_descriptors[sock].socket->local_ipaddr) {
			in_addr->sin_addr.s_addr = local_socket_descriptors[sock].socket->local_ipaddr;
			in_addr->sin_port = local_socket_descriptors[sock].socket->local_port;
		}
		else {
//printf("%s %d %d\n",__func__,__LINE__,sock);
			in_addr->sin_addr.s_addr = local_socket_descriptors[sock].local_ipaddr;
			in_addr->sin_port = local_socket_descriptors[sock].local_port;
		}
	}
	else {
		in_addr->sin_addr.s_addr = local_socket_descriptors[sock].remote_ipaddr;
		in_addr->sin_port = local_socket_descriptors[sock].remote_port;
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

void ipaugenblick_put_to_local_cache(int sock)
{
	if (!local_socket_descriptors[sock].local_mask)
		return;
	if (!local_socket_descriptors[sock].present_in_ready_cache) { 
		TAILQ_INSERT_TAIL(&selectors[local_socket_descriptors[sock].select].local_ready_cache,
		  &local_socket_descriptors[sock],
		  local_ready_cache_entry);
		local_socket_descriptors[sock].present_in_ready_cache = 1;
	}
}

static inline int ipaugenblick_update_fdset(int idx, struct ipaugenblick_fdset *fdset)
{
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"%s %d %d \n",
		__func__,__LINE__,idx, fdset->interested_flags[idx], fdset->returned_flags[idx]);
	if (fdset->interested_flags[idx]) {
		if (!(fdset->returned_flags[idx])) {
			fdset->returned_sockets[fdset->returned_idx] = idx;
			fdset->returned_flags[idx] = 1;
			fdset->returned_idx++;
			return 1;
		}
	}
	return 0;
}

int ipaugenblick_select(int selector,
			struct ipaugenblick_fdset *readfdset, 
			struct ipaugenblick_fdset *writefdset,
			struct ipaugenblick_fdset *excfdset,
			struct timeval* timeout)
{
    uint32_t ringset_idx_and_ready_mask;
    ipaugenblick_cmd_t *cmd;
    uint32_t iterations_to_wait;
    int exit_after_check = 0;
    int ret = 0,fd_updated,i;
    local_socket_descriptor_t *local_socket_descr = TAILQ_FIRST(&selectors[selector].local_ready_cache), *next;

    if (readfdset) {
	for(i = 0; i < readfdset->returned_idx; i++)
		readfdset->returned_flags[readfdset->returned_sockets[i]] = 0;
	readfdset->returned_idx = 0;
    }	

    if (writefdset) {
	for(i = 0; i < writefdset->returned_idx; i++)
		writefdset->returned_flags[writefdset->returned_sockets[i]] = 0;
	writefdset->returned_idx = 0;
    }

    if (excfdset) {
	for(i = 0; i < excfdset->returned_idx; i++)
		excfdset->returned_flags[excfdset->returned_sockets[i]] = 0;
	excfdset->returned_idx = 0;
    }
 
    ipaugenblick_stats_select_called++;

    fd_updated = 0;
  
    while (local_socket_descr != NULL) {
	int idx = ((unsigned char *)local_socket_descr - (unsigned char *)&local_socket_descriptors[0])/sizeof(local_socket_descriptor_t);
	ipaugenblick_log(IPAUGENBLICK_LOG_DEBUG,"%s %d sock %d mask %x readfdset mask %x writefdset mask %x\n",
			__func__,__LINE__,idx,local_socket_descr->local_mask,readfdset ? readfdset->interested_flags[idx] : 0,writefdset ? writefdset->interested_flags[idx] : 0);
	if ((local_socket_descr->local_mask & SOCKET_READABLE_BIT)&&(readfdset)&&(readfdset->interested_flags[idx])) {
		fd_updated |= ipaugenblick_update_fdset(idx, readfdset);
	}
	if ((local_socket_descr->local_mask & SOCKET_WRITABLE_BIT)&&(writefdset)&&(writefdset->interested_flags[idx])) {
		fd_updated |= ipaugenblick_update_fdset(idx, writefdset);
	}
	if ((local_socket_descr->local_mask & SOCKET_CLOSED_BIT)&&(excfdset)&&(excfdset->interested_flags[idx])) {	
		ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"%s %d %d %p\n",__func__,__LINE__,idx,local_socket_descr);
		fd_updated |= ipaugenblick_update_fdset(idx, excfdset);
	}
	next = TAILQ_NEXT(local_socket_descr, local_ready_cache_entry);
	if (fd_updated) {
		ipaugenblick_log(IPAUGENBLICK_LOG_DEBUG,"%s %d present_in_ready_cache %d\n",
		__func__,__LINE__,local_socket_descr->present_in_ready_cache);
		if (local_socket_descr->present_in_ready_cache) {
			TAILQ_REMOVE(&selectors[selector].local_ready_cache,
			     local_socket_descr, 
			     local_ready_cache_entry);
		    local_socket_descr->present_in_ready_cache = 0;
		}
	}
	local_socket_descr = next;
    }
restart_waiting:
    ret = (readfdset ? readfdset->returned_idx : 0) + (writefdset ? writefdset->returned_idx : 0) + (excfdset ? excfdset->returned_idx : 0);
    if (ret) {
	ipaugenblick_log(IPAUGENBLICK_LOG_DEBUG,"%s %d ret %d\n",__func__,__LINE__,ret);
	return ret;
    }
    if(rte_ring_dequeue(selectors[selector].ready_connections,(void **)&ringset_idx_and_ready_mask)) {
	if (exit_after_check)
		return -1;
        if((timeout)&&(timeout->tv_sec == 0)&&(timeout->tv_usec == 0)) {
            return -1;
        }
        else { /* if timeout is NULL or non-zero */
		pselect(0, NULL, NULL, NULL, (const struct timespec *)timeout, /*&sigmask*/NULL);
		if (timeout != NULL)
			exit_after_check = 1;
	}
        goto restart_waiting;
    }
    ipaugenblick_stats_select_returned++; 
    if((ringset_idx_and_ready_mask & SOCKET_READY_MASK) >= IPAUGENBLICK_CONNECTION_POOL_SIZE) {
        ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"FATAL ERROR %s %d %d\n",__FILE__,__LINE__,ringset_idx_and_ready_mask & SOCKET_READY_MASK);
        exit(0);
    }
    ipaugenblick_log(IPAUGENBLICK_LOG_DEBUG,"%s %d sock %d mask %x local_mask %x\n",
	__func__,__LINE__,ringset_idx_and_ready_mask & SOCKET_READY_MASK, ringset_idx_and_ready_mask >>SOCKET_READY_SHIFT,
	local_socket_descriptors[ringset_idx_and_ready_mask & SOCKET_READY_MASK]);
    local_socket_descriptors[ringset_idx_and_ready_mask & SOCKET_READY_MASK].local_mask |= 
	ringset_idx_and_ready_mask >> SOCKET_READY_SHIFT;
    local_socket_descriptors[ringset_idx_and_ready_mask & SOCKET_READY_MASK].any_event_received = 1;
    ipaugenblick_put_to_local_cache(ringset_idx_and_ready_mask & SOCKET_READY_MASK);
    fd_updated = 0;
    if (((ringset_idx_and_ready_mask >> SOCKET_READY_SHIFT) & SOCKET_READABLE_BIT)&&(readfdset))
    	fd_updated |= ipaugenblick_update_fdset(ringset_idx_and_ready_mask & SOCKET_READY_MASK, readfdset);
    if (((ringset_idx_and_ready_mask >> SOCKET_READY_SHIFT) & SOCKET_WRITABLE_BIT)&&(writefdset))
    	fd_updated |= ipaugenblick_update_fdset(ringset_idx_and_ready_mask & SOCKET_READY_MASK, writefdset);
    if (((ringset_idx_and_ready_mask >> SOCKET_READY_SHIFT) & SOCKET_CLOSED_BIT)&&(excfdset))
    	fd_updated |= ipaugenblick_update_fdset(ringset_idx_and_ready_mask & SOCKET_READY_MASK, excfdset);
    if (fd_updated) {
	    ipaugenblick_log(IPAUGENBLICK_LOG_DEBUG,"%s %d present_in_ready_cache %d\n",
		__func__,__LINE__,local_socket_descriptors[ringset_idx_and_ready_mask & SOCKET_READY_MASK].present_in_ready_cache);
	    if (local_socket_descriptors[ringset_idx_and_ready_mask & SOCKET_READY_MASK].present_in_ready_cache) {
			TAILQ_REMOVE(&selectors[selector].local_ready_cache,
			     &local_socket_descriptors[ringset_idx_and_ready_mask & SOCKET_READY_MASK
], 
			     local_ready_cache_entry);
		    local_socket_descriptors[ringset_idx_and_ready_mask & SOCKET_READY_MASK].present_in_ready_cache = 0;
  	    }
    }
    goto restart_waiting;
}

/* receive functions return a chained buffer. this function
   retrieves a next chunk and its length */
void *ipaugenblick_get_next_buffer_segment(void **pdesc,int *len)
{
   struct rte_mbuf *mbuf = (struct rte_mbuf *)*pdesc;
   
   mbuf = mbuf->next;
   if(!mbuf) {
       return NULL;
   } 
   *len = rte_pktmbuf_data_len(mbuf);
   *pdesc = mbuf;
   return rte_pktmbuf_mtod(mbuf,void *);
}

void *ipaugenblick_get_next_buffer_segment_and_detach_first(void **pdesc,int *len)
{
   struct rte_mbuf *mbuf = (struct rte_mbuf *)*pdesc;
   
   mbuf = mbuf->next;
   if(!mbuf) {
       return NULL;
   } 
   *len = rte_pktmbuf_data_len(mbuf);
   *pdesc = mbuf;
   struct rte_mbuf *orig = (struct rte_mbuf *)*pdesc;
   orig->next = NULL;
   return rte_pktmbuf_mtod(mbuf,void *);
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

void ipaugenblick_set_buffer_data_len(void *buffer, int len)
{
    struct rte_mbuf *mbuf = (struct rte_mbuf *)buffer;
    rte_pktmbuf_data_len(mbuf) = len;
}

int ipaugenblick_get_buffer_data_len(void *buffer)
{
    struct rte_mbuf *mbuf = (struct rte_mbuf *)buffer;
    return rte_pktmbuf_data_len(mbuf);
}

void *ipaugenblick_get_data_ptr(void *desc)
{
	struct rte_mbuf *mbuf = (struct rte_mbuf *)desc;
	return rte_pktmbuf_mtod(mbuf, void *);
}

void ipaugenblick_update_rfc(void *desc, signed delta)
{
	struct rte_mbuf *mbuf = (struct rte_mbuf *)desc;
	rte_mbuf_refcnt_update(mbuf, delta);
}
}
