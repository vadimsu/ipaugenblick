
#ifndef __IPAUGENBLICK_SERVER_SIDE_H__
#define __IPAUGENBLICK_SERVER_SIDE_H__
//#include <sys/types.h>
//#include <signal.h>
#define PKTMBUF_HEADROOM 128
#define IPAUGENBLICK_BUFSIZE (PKTMBUF_HEADROOM+1448)
#include <ipaugenblick_log.h>

struct ipaugenblick_clients
{
        TAILQ_ENTRY(ipaugenblick_clients) queue_entry;
        struct rte_ring *client_ring;
	int is_busy;
};

typedef struct
{
    struct socket *socket;
    int ringset_idx;
    int parent_idx;
    struct rte_ring *tx_ring;
    struct rte_ring *rx_ring;
    pid_t apppid;
} socket_satelite_data_t;

extern struct rte_ring *command_ring[MAXCPU];
extern struct rte_ring *selectors_ring[MAXCPU];
extern struct rte_ring *free_connections_ring[MAXCPU];
extern struct rte_mempool *free_connections_pool[MAXCPU];
extern struct rte_ring *free_clients_ring[MAXCPU];
extern struct ipaugenblick_clients ipaugenblick_clients[MAXCPU][IPAUGENBLICK_CLIENTS_POOL_SIZE];
extern struct rte_mempool *selectors_pool[MAXCPU];
extern struct rte_ring *rx_mbufs_ring[MAXCPU];
extern struct rte_mempool *free_command_pool[MAXCPU];
extern socket_satelite_data_t socket_satelite_data[MAXCPU][IPAUGENBLICK_CONNECTION_POOL_SIZE];
extern ipaugenblick_socket_t *g_ipaugenblick_sockets[MAXCPU];
extern ipaugenblick_selector_t *g_ipaugenblick_selectors[MAXCPU];
extern uint64_t user_kick_select_tx;
extern uint64_t user_kick_select_rx;
extern uint64_t user_pending_accept;

#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

static inline struct ipaugenblick_memory *ipaugenblick_service_api_init(int command_bufs_count,
                                                          int rx_bufs_count,
                                                          int tx_bufs_count)
{   
	char ringname[1024];
	int ringset_idx,i; 
	ipaugenblick_socket_t *ipaugenblick_socket;
	ipaugenblick_selector_t *ipaugenblick_selector;

	memset(ipaugenblick_clients[rte_lcore_id()],0,sizeof(ipaugenblick_clients[rte_lcore_id()]));

	sprintf(ringname,"#%d#%s",rte_lcore_id(),FREE_CLIENTS_RING);

	free_clients_ring[rte_lcore_id()] = rte_ring_create(ringname,IPAUGENBLICK_CLIENTS_POOL_SIZE,rte_socket_id(), 0);
	if(!free_clients_ring[rte_lcore_id()]) {
		ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot create ring %s %d\n",__FILE__,__LINE__);
		exit(0);
	}

	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"FREE CLIENTS RING CREATED\n");

	for(ringset_idx = 0;ringset_idx < IPAUGENBLICK_CLIENTS_POOL_SIZE;ringset_idx++) {
        	rte_ring_enqueue(free_clients_ring[rte_lcore_id()],(void*)ringset_idx);
		sprintf(ringname,"%s#%d#%d",FREE_CLIENTS_RING,rte_lcore_id(),ringset_idx);
		ipaugenblick_clients[rte_lcore_id()][ringset_idx].client_ring = rte_ring_create(ringname, 64,rte_socket_id(), RING_F_SP_ENQ | RING_F_SC_DEQ);
		if(!ipaugenblick_clients[rte_lcore_id()][ringset_idx].client_ring) {
			ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot create ring %s %d\n",__FILE__,__LINE__);
			exit(0);
		}
	}

	memset(socket_satelite_data[rte_lcore_id()],0,sizeof(void *)*IPAUGENBLICK_CONNECTION_POOL_SIZE);

	sprintf(ringname,"%s#%d",COMMAND_RING_NAME, rte_lcore_id());

	command_ring[rte_lcore_id()] = rte_ring_create(ringname, command_bufs_count,rte_socket_id(), RING_F_SC_DEQ);
	if(!command_ring[rte_lcore_id()]) {
        	ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot create ring %s %d\n",__FILE__,__LINE__);
	        exit(0);
	}
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"COMMAND RING CREATED\n");
	sprintf(ringname,"%s#%d","rx_mbufs_ring", rte_lcore_id());

	rx_mbufs_ring[rte_lcore_id()] = rte_ring_create(ringname, rx_bufs_count*IPAUGENBLICK_CONNECTION_POOL_SIZE,rte_socket_id(), 0);
	if(!rx_mbufs_ring[rte_lcore_id()]) {
		ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot create ring %s %d\n",__FILE__,__LINE__);
		exit(0);
	}
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"RX RING CREATED\n");
	sprintf(ringname,"%s#%d",FREE_COMMAND_POOL_NAME,rte_lcore_id());

	free_command_pool[rte_lcore_id()] = rte_mempool_create(ringname, command_bufs_count,
	    			           sizeof(ipaugenblick_cmd_t), 0,
				           0,
				           NULL, NULL,
				           NULL, NULL,
				           rte_socket_id(), 0);
	if(!free_command_pool[rte_lcore_id()]) {
		ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot create mempool %s %d\n",__FILE__,__LINE__);
		exit(0);
	}
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"COMMAND POOL CREATED\n");
	sprintf(ringname,"%s#%d",FREE_CONNECTIONS_POOL_NAME,rte_lcore_id());

	free_connections_pool[rte_lcore_id()] = rte_mempool_create(ringname, 1,
	    	  		               sizeof(ipaugenblick_socket_t)*IPAUGENBLICK_CONNECTION_POOL_SIZE, 0,
				               0,
				               NULL, NULL,
				               NULL, NULL,
				               rte_socket_id(), 0);
	if(!free_connections_pool[rte_lcore_id()]) {
		ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot create mempool %s %d\n",__FILE__,__LINE__);
		exit(0);
	}

	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"FREE CONNECTIONS POOL CREATED\n");

	if(rte_mempool_get(free_connections_pool[rte_lcore_id()],(void **)&g_ipaugenblick_sockets[rte_lcore_id()])) {
		ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot get from mempool %s %d \n",__FILE__,__LINE__);
		exit(0);
	}

	ipaugenblick_socket = g_ipaugenblick_sockets[rte_lcore_id()];

	sprintf(ringname,"%s#%d",FREE_CONNECTIONS_RING,rte_lcore_id());

	free_connections_ring[rte_lcore_id()] = rte_ring_create(ringname,IPAUGENBLICK_CONNECTION_POOL_SIZE,rte_socket_id(), 0);
	if(!free_connections_ring[rte_lcore_id()]) {
		ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot create ring %s %d\n",__FILE__,__LINE__);
		exit(0);
	}

	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"FREE CONNECTIONS RING CREATED\n"); 

	for(ringset_idx = 0;ringset_idx < IPAUGENBLICK_CONNECTION_POOL_SIZE;ringset_idx++,ipaugenblick_socket++) { 
		memset(ipaugenblick_socket,0,sizeof(ipaugenblick_socket_t));
		ipaugenblick_socket->connection_idx = ringset_idx;
		rte_atomic16_init(&ipaugenblick_socket->read_ready_to_app);
		rte_atomic16_init(&ipaugenblick_socket->write_ready_to_app);
		rte_atomic32_init(&ipaugenblick_socket->tx_space);
		rte_ring_enqueue(free_connections_ring[rte_lcore_id()],(void*)ipaugenblick_socket);
		sprintf(ringname,TX_RING_NAME_BASE"#%d#%d", rte_lcore_id(),ringset_idx);
		socket_satelite_data[rte_lcore_id()][ringset_idx].tx_ring = rte_ring_create(ringname, tx_bufs_count,rte_socket_id(), RING_F_SP_ENQ | RING_F_SC_DEQ);
		if(!socket_satelite_data[rte_lcore_id()][ringset_idx].tx_ring) {
			ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot create ring %s %d\n",__FILE__,__LINE__);
			exit(0);
		}
		rte_ring_set_water_mark(socket_satelite_data[rte_lcore_id()][ringset_idx].tx_ring,/*tx_bufs_count/10*/1);
		sprintf(ringname,RX_RING_NAME_BASE"%d#%d", rte_lcore_id(),ringset_idx);
		socket_satelite_data[rte_lcore_id()][ringset_idx].rx_ring = rte_ring_create(ringname, rx_bufs_count,rte_socket_id(), RING_F_SP_ENQ | RING_F_SC_DEQ);
		if(!socket_satelite_data[rte_lcore_id()][ringset_idx].rx_ring) {
			ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot create ring %s %d\n",__FILE__,__LINE__);
			exit(0);
		}
		rte_ring_set_water_mark(socket_satelite_data[rte_lcore_id()][ringset_idx].rx_ring,/*rx_bufs_count/10*/1);
		socket_satelite_data[rte_lcore_id()][ringset_idx].ringset_idx = -1;
		socket_satelite_data[rte_lcore_id()][ringset_idx].parent_idx = -1;
		socket_satelite_data[rte_lcore_id()][ringset_idx].socket = NULL;
		socket_satelite_data[rte_lcore_id()][ringset_idx].apppid = 0;
	}
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"CONNECTIONS Tx/Rx RINGS CREATED\n");
    
	sprintf(ringname,"%s#%d",SELECTOR_POOL_NAME,rte_lcore_id());

	selectors_pool[rte_lcore_id()] = rte_mempool_create(ringname, 1,
	     		                sizeof(ipaugenblick_selector_t)*IPAUGENBLICK_SELECTOR_POOL_SIZE, 0,
				               0,
				               NULL, NULL,
				               NULL, NULL,
				               rte_socket_id(), 0);
	if(!selectors_pool[rte_lcore_id()]) {
		ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot create mempool %s %d\n",__FILE__,__LINE__);
		exit(0);
	}
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"SELECTOR POOL CREATED\n");
	if(rte_mempool_get(selectors_pool[rte_lcore_id()],(void **)&g_ipaugenblick_selectors[rte_lcore_id()])) {
		ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot get from mempool %s %d \n",__FILE__,__LINE__);
		exit(0);
	}
	sprintf(ringname,"%s#%d",SELECTOR_RING_NAME, rte_lcore_id());
	selectors_ring[rte_lcore_id()] = rte_ring_create(ringname,IPAUGENBLICK_SELECTOR_POOL_SIZE,rte_socket_id(), 0);
	if(!selectors_ring[rte_lcore_id()]) {
		ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot create ring %s %d \n",__FILE__,__LINE__);
		exit(0);
	}
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"SELECTOR RING CREATED\n");
	ipaugenblick_selector = g_ipaugenblick_selectors[rte_lcore_id()];
	for(ringset_idx = 0;ringset_idx < IPAUGENBLICK_SELECTOR_POOL_SIZE;ringset_idx++) {
        	sprintf(ringname,"SELECTOR_RING_NAME%d#%d",ringset_idx, rte_lcore_id());
		ipaugenblick_selector[ringset_idx].ready_connections = rte_ring_create(ringname, tx_bufs_count,rte_socket_id(), RING_F_SP_ENQ | RING_F_SC_DEQ);
		if(!ipaugenblick_selector[ringset_idx].ready_connections) {
			ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot create ring %s %d\n",__FILE__,__LINE__);
			exit(0);
		}
		ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"SELECTOR READY RING#%d CREATED\n",ringset_idx);
		rte_ring_enqueue(selectors_ring[rte_lcore_id()],(void*)ringset_idx);
	} 
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"DONE\n");
	return 0;
}
static inline ipaugenblick_cmd_t *ipaugenblick_dequeue_command_buf()
{
    ipaugenblick_cmd_t *cmd;
    if(!rte_ring_count(command_ring[rte_lcore_id()]))
        return NULL;
    if(rte_ring_sc_dequeue_bulk(command_ring[rte_lcore_id()],(void **)&cmd,1)) {
        return NULL;
    }
    return cmd;
}

static inline ipaugenblick_cmd_t *ipaugenblick_get_free_command_buf()
{
    ipaugenblick_cmd_t *cmd;
    if(rte_mempool_get(free_command_pool[rte_lcore_id()],(void **)&cmd))
        return NULL;
    return cmd;
}

static inline void ipaugenblick_free_command_buf(ipaugenblick_cmd_t *cmd)
{
    rte_mempool_put(free_command_pool[rte_lcore_id()],(void *)cmd);
}

extern unsigned long app_pid;

static inline void ipaugenblick_mark_readable(void *descriptor)
{
    uint32_t ringidx_ready_mask; 
    socket_satelite_data_t *socket_satelite_data = (socket_satelite_data_t *)descriptor;
    if(socket_satelite_data->parent_idx == -1) {
        //ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"%s %d\n",__FILE__,__LINE__);
        return;
    }

//    ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"%s %d %d\n",__FILE__,__LINE__,socket_satelite_data->apppid);
    if(!rte_atomic16_test_and_set(&g_ipaugenblick_sockets[rte_lcore_id()][socket_satelite_data->ringset_idx].read_ready_to_app)) {
#if 0
        if(socket_satelite_data->apppid)
           kill(socket_satelite_data->apppid,/*SIGUSR1*/10);
#endif
        return;
    }

    ringidx_ready_mask = socket_satelite_data->ringset_idx|(SOCKET_READABLE_BIT << SOCKET_READY_SHIFT);
    rte_ring_enqueue(g_ipaugenblick_selectors[rte_lcore_id()][socket_satelite_data->parent_idx].ready_connections,(void *)ringidx_ready_mask);
    if(socket_satelite_data->apppid)
           kill(socket_satelite_data->apppid,/*SIGUSR1*/10);
    user_kick_select_rx++; 
}

static inline void ipaugenblick_post_accepted(ipaugenblick_cmd_t *cmd,void *parent_descriptor)
{
    socket_satelite_data_t *socket_satelite_data = (socket_satelite_data_t *)parent_descriptor;
//    ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"%s %d %d\n",__FILE__,__LINE__,socket_satelite_data->ringset_idx);
//    cmd->ringset_idx = socket_satelite_data->ringset_idx;
    if(rte_ring_enqueue(socket_satelite_data->rx_ring,(void *)cmd) == -ENOBUFS) { 
        ipaugenblick_free_command_buf(cmd);
    }
    else {
        ipaugenblick_mark_readable(parent_descriptor);
    }
    user_pending_accept++;
}

static inline struct rte_mbuf *ipaugenblick_dequeue_tx_buf(void *descriptor)
{
    struct rte_mbuf *mbuf;
    socket_satelite_data_t *socket_satelite_data = (socket_satelite_data_t *)descriptor;
    if(rte_ring_sc_dequeue_bulk(socket_satelite_data->tx_ring,(void **)&mbuf,1)) { 
        return NULL;
    }
    return mbuf;
}

static inline int ipaugenblick_dequeue_tx_buf_burst(void *descriptor,struct rte_mbuf **mbufs,int max_count)
{
    socket_satelite_data_t *socket_satelite_data = (socket_satelite_data_t *)descriptor; 
    return rte_ring_sc_dequeue_burst(socket_satelite_data->tx_ring,(void **)mbufs,max_count);
}

static inline int ipaugenblick_tx_buf_count(void *descriptor)
{
    socket_satelite_data_t *socket_satelite_data = (socket_satelite_data_t *)descriptor;
    rte_atomic16_set(&g_ipaugenblick_sockets[rte_lcore_id()][socket_satelite_data->ringset_idx].write_done_from_app,0);
    return rte_ring_count(socket_satelite_data->tx_ring);
}

static inline int ipaugenblick_rx_buf_free_count(void *descriptor)
{ 
    socket_satelite_data_t *socket_satelite_data = (socket_satelite_data_t *)descriptor;
    return rte_ring_free_count(socket_satelite_data->rx_ring);
}

static inline int ipaugenblick_submit_rx_buf(struct rte_mbuf *mbuf,void *descriptor)
{
    uint32_t ringidx_ready_mask; 
    int rc;
    socket_satelite_data_t *socket_satelite_data = (socket_satelite_data_t *)descriptor;
    rc = rte_ring_sp_enqueue_bulk(socket_satelite_data->rx_ring,(void *)&mbuf,1);
     
    if(rc != 0)
        ipaugenblick_mark_readable(descriptor);
    
    return (rc == -ENOBUFS);
}

static inline int ipaugenblick_mark_writable(void *descriptor)
{
    uint32_t ringidx_ready_mask;
    int rc;
    socket_satelite_data_t *socket_satelite_data = (socket_satelite_data_t *)descriptor;
    if(socket_satelite_data->parent_idx == -1) {
//	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"%s %d\n",__FILE__,__LINE__);
        return 1;
    }
    if(!rte_atomic16_test_and_set(&g_ipaugenblick_sockets[rte_lcore_id()][socket_satelite_data->ringset_idx].write_ready_to_app)) {
#if 0
	if(socket_satelite_data->apppid)
        	kill(socket_satelite_data->apppid,/*SIGUSR1*/10);
#endif
        return;
    }
    ringidx_ready_mask = socket_satelite_data->ringset_idx|(SOCKET_WRITABLE_BIT << SOCKET_READY_SHIFT);
    rc = rte_ring_enqueue(g_ipaugenblick_selectors[rte_lcore_id()][socket_satelite_data->parent_idx].ready_connections,(void *)ringidx_ready_mask);
    user_kick_select_tx++;
//    ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"%s %d %d\n",__FILE__,__LINE__,socket_satelite_data->apppid);
    if(socket_satelite_data->apppid)
        kill(socket_satelite_data->apppid,/*SIGUSR1*/10);
    return (rc == -ENOBUFS);
}

static inline int ipaugenblick_mark_closed(void *descriptor)
{
    uint32_t ringidx_ready_mask;
    int rc;
    socket_satelite_data_t *socket_satelite_data = (socket_satelite_data_t *)descriptor;
    if(socket_satelite_data->parent_idx == -1) {
//	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"%s %d\n",__FILE__,__LINE__);
        return 1;
    }
    ringidx_ready_mask = socket_satelite_data->ringset_idx|(SOCKET_CLOSED_BIT << SOCKET_READY_SHIFT);
    rc = rte_ring_enqueue(g_ipaugenblick_selectors[rte_lcore_id()][socket_satelite_data->parent_idx].ready_connections,(void *)ringidx_ready_mask);
//    ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"%s %d %d\n",__FILE__,__LINE__,socket_satelite_data->apppid);
    if(socket_satelite_data->apppid)
        kill(socket_satelite_data->apppid,/*SIGUSR1*/10);
    return (rc == -ENOBUFS);
}

static inline void ipaugenblick_free_socket(int connidx)
{
   rte_ring_enqueue(free_connections_ring[rte_lcore_id()],(void *)&g_ipaugenblick_sockets[rte_lcore_id()][connidx]);
}

#endif
