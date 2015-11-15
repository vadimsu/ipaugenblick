#include <specific_includes/dummies.h>
#include <specific_includes/linux/types.h>
#include <specific_includes/linux/bitops.h>
#include <specific_includes/linux/slab.h>
#include <specific_includes/linux/hash.h>
#include <specific_includes/linux/socket.h>
#include <specific_includes/linux/sockios.h>
#include <specific_includes/linux/if_ether.h>
#include <specific_includes/linux/netdevice.h>
#include <specific_includes/linux/etherdevice.h>
#include <specific_includes/linux/ethtool.h>
#include <specific_includes/linux/skbuff.h>
#include <specific_includes/net/net_namespace.h>
#include <specific_includes/net/sock.h>
#include <specific_includes/linux/rtnetlink.h>
#include <specific_includes/net/dst.h>
#include <specific_includes/net/checksum.h>
#include <specific_includes/linux/err.h>
#include <specific_includes/linux/if_arp.h>
#include <specific_includes/linux/if_vlan.h>
#include <specific_includes/linux/ip.h>
#include <specific_includes/net/ip.h>
#include <specific_includes/linux/ipv6.h>
#include <specific_includes/linux/in.h>
#include <specific_includes/net/netlink.h>
#include <string.h>
#include <rte_config.h>
#include <rte_common.h>
#include <rte_cycles.h>
#include <rte_timer.h>
#include <rte_ring.h>
#include <api.h>
#include <porting/libinit.h>
#include <rte_atomic.h>
#include "ipaugenblick_common/ipaugenblick_common.h"
#include "ipaugenblick_service/ipaugenblick_server_side.h"
#include "user_callbacks.h"
#include <ipaugenblick_log.h>

uint64_t user_on_tx_opportunity_cycles = 0;
uint64_t user_on_tx_opportunity_called = 0;
uint64_t user_on_tx_opportunity_getbuff_called = 0;
uint64_t user_on_tx_opportunity_api_nothing_to_tx = 0;
uint64_t user_on_tx_opportunity_api_failed = 0;
uint64_t user_on_tx_opportunity_api_mbufs_sent = 0;
uint64_t user_on_tx_opportunity_socket_full = 0;
uint64_t user_on_tx_opportunity_cannot_get_buff = 0;
uint64_t user_on_rx_opportunity_called = 0;
uint64_t user_on_rx_opportunity_called_exhausted = 0;
uint64_t user_rx_mbufs = 0;
uint64_t user_kick_tx = 0;
uint64_t user_kick_rx = 0;
uint64_t user_kick_select_rx = 0;
uint64_t user_kick_select_tx = 0;
uint64_t user_on_tx_opportunity_cannot_send = 0;
uint64_t user_rx_ring_full = 0;
uint64_t user_on_tx_opportunity_socket_send_error = 0;
uint64_t user_client_app_accepted = 0;
uint64_t user_pending_accept = 0;
uint64_t user_sockets_closed = 0;
uint64_t user_sockets_shutdown = 0;
uint64_t g_last_time_transmitted = 0;

struct rte_ring *command_ring = NULL;
struct rte_ring *selectors_ring = NULL;
struct rte_mempool *free_connections_pool = NULL;
struct rte_ring *free_connections_ring = NULL;
struct rte_ring *free_clients_ring = NULL;
struct rte_mempool *selectors_pool = NULL;
struct rte_ring *rx_mbufs_ring = NULL;
struct rte_mempool *free_command_pool = NULL;
socket_satelite_data_t socket_satelite_data[IPAUGENBLICK_CONNECTION_POOL_SIZE];
ipaugenblick_socket_t *g_ipaugenblick_sockets = NULL;
ipaugenblick_selector_t *g_ipaugenblick_selectors = NULL;
//unsigned long app_pid = 0;

TAILQ_HEAD(buffers_available_notification_socket_list_head, socket) buffers_available_notification_socket_list_head;

struct ipaugenblick_clients ipaugenblick_clients[IPAUGENBLICK_CLIENTS_POOL_SIZE];
TAILQ_HEAD(ipaugenblick_clients_list_head, ipaugenblick_clients) ipaugenblick_clients_list_head;

int get_all_devices(unsigned char *buf);

int get_all_addresses(unsigned char *buf);

void on_iface_up(char *name)
{
}

void on_new_addr(char *iface_name,unsigned int ipaddr,unsigned int mask)
{
}

void user_on_closure(struct socket *sock)
{
	socket_satelite_data_t *socket_satelite_data = get_user_data(sock);
	if (!socket_satelite_data) {
		return;
	}
	ipaugenblick_mark_closed(socket_satelite_data);
}

static void on_client_connect(int client_idx)
{
	struct rte_mbuf *buffer = get_buffer();
	if(!buffer) {
		return;
	}
	unsigned char *data = rte_pktmbuf_mtod(buffer, unsigned char *);
	*data = IPAUGENBLICK_NEW_IFACES;
	data++;
	rte_pktmbuf_data_len(buffer) = get_all_devices(data) + 1;
	rte_ring_enqueue(ipaugenblick_clients[client_idx].client_ring,(void *)buffer);
	buffer = get_buffer();
	if(!buffer) {
		return;
	}
	data = rte_pktmbuf_mtod(buffer, unsigned char *);
	*data = IPAUGENBLICK_NEW_ADDRESSES;
	data++;
	rte_pktmbuf_data_len(buffer) = get_all_addresses(data) + 1;
	rte_ring_enqueue(ipaugenblick_clients[client_idx].client_ring,(void *)buffer);
	buffer = get_buffer();
	if(!buffer) {
		return;
	}
	data = rte_pktmbuf_mtod(buffer, unsigned char *);
	*data = IPAUGENBLICK_END_OF_RECORD;
	rte_ring_enqueue(ipaugenblick_clients[client_idx].client_ring,(void *)buffer);
}

void user_transmitted_callback(struct rte_mbuf *mbuf,struct socket *sock)
{
	int last = /*(rte_mbuf_refcnt_read(mbuf) == 1)*/1;
        if((sock)&&(last)) {
               socket_satelite_data_t *socket_satelite_data = get_user_data(sock);
               if(socket_satelite_data) {
//printf("%s %d %p %d %d %d\n",__FILE__,__LINE__,&g_ipaugenblick_sockets[socket_satelite_data->ringset_idx],socket_satelite_data->ringset_idx, rte_pktmbuf_data_len(mbuf),rte_mbuf_refcnt_read(mbuf));
                       user_increment_socket_tx_space(&g_ipaugenblick_sockets[socket_satelite_data->ringset_idx].tx_space,rte_pktmbuf_data_len(mbuf));
               }
        }
        rte_pktmbuf_free_seg(mbuf);
}

static inline void process_commands()
{
    int ringset_idx;
    ipaugenblick_cmd_t *cmd;
    struct rte_mbuf *mbuf;
    struct socket *sock;
    char *p;
    struct sockaddr_in addr;
    struct sockaddr_in *p_sockaddr;
    struct rtentry rtentry;
    int len;

    cmd = ipaugenblick_dequeue_command_buf();
    if(!cmd)
        return;
    switch(cmd->cmd) {
        case IPAUGENBLICK_OPEN_SOCKET_COMMAND:
           ipaugenblick_log(IPAUGENBLICK_LOG_DEBUG,"open_sock %x %x %x %x\n",cmd->u.open_sock.family,cmd->u.open_sock.type);
           sock = app_glue_create_socket(cmd->u.open_sock.family,cmd->u.open_sock.type);
           if(sock) {
               ipaugenblick_log(IPAUGENBLICK_LOG_DEBUG,"setting user data %p\n",sock);
               socket_satelite_data[cmd->ringset_idx].ringset_idx = cmd->ringset_idx;
               socket_satelite_data[cmd->ringset_idx].parent_idx = cmd->parent_idx;
	       socket_satelite_data[cmd->ringset_idx].apppid = cmd->u.open_sock.pid;
               app_glue_set_user_data(sock,(void *)&socket_satelite_data[cmd->ringset_idx]);
               socket_satelite_data[cmd->ringset_idx].socket = sock;
	       ipaugenblick_log(IPAUGENBLICK_LOG_DEBUG,"%d setting tx_space %d\n",__LINE__,sk_stream_wspace(sock->sk));
	       user_set_socket_tx_space(&g_ipaugenblick_sockets[socket_satelite_data[cmd->ringset_idx].ringset_idx].tx_space,sk_stream_wspace(sock->sk));
           }
           ipaugenblick_log(IPAUGENBLICK_LOG_DEBUG,"Done\n");
           break;
	case IPAUGENBLICK_SOCKET_CONNECT_BIND_COMMAND:
           if(socket_satelite_data[cmd->ringset_idx].socket) { 
               if(cmd->u.socket_connect_bind.is_connect) {
			ipaugenblick_log(IPAUGENBLICK_LOG_DEBUG,"connect %x\n",cmd->ringset_idx);
		       if(app_glue_v4_connect(socket_satelite_data[cmd->ringset_idx].socket,
		   		     cmd->u.socket_connect_bind.ipaddr,
				     cmd->u.socket_connect_bind.port)) {
	                   ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"failed to connect socket\n");
        	       }
               	       else {
                   	   ipaugenblick_log(IPAUGENBLICK_LOG_DEBUG,"socket connected\n");
			   len = sizeof(addr);
			   inet_getname(socket_satelite_data[cmd->ringset_idx].socket,&addr,&len,0);
			   g_ipaugenblick_sockets[cmd->ringset_idx].local_ipaddr = addr.sin_addr.s_addr;
			   g_ipaugenblick_sockets[cmd->ringset_idx].local_port = addr.sin_port;
               	       } 
	       }
	       else {
			ipaugenblick_log(IPAUGENBLICK_LOG_DEBUG,"bind %x\n",cmd->ringset_idx);
		       if(app_glue_v4_bind(socket_satelite_data[cmd->ringset_idx].socket,
		   		     cmd->u.socket_connect_bind.ipaddr,
				     cmd->u.socket_connect_bind.port)) {
				ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"cannot bind %x %x\n",cmd->u.socket_connect_bind.ipaddr,cmd->u.socket_connect_bind.port);
			}
	       } 
           }
           else {
              ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"no socket to invoke command!!!\n");
           }
           break;
	case IPAUGENBLICK_LISTEN_SOCKET_COMMAND:
           ipaugenblick_log(IPAUGENBLICK_LOG_DEBUG,"listen %x\n",cmd->ringset_idx);
           if(app_glue_v4_listen(socket_satelite_data[cmd->ringset_idx].socket)) {
               ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"failed listening\n"); 
           }
           break;

        case IPAUGENBLICK_SOCKET_CLOSE_COMMAND:
           if(socket_satelite_data[cmd->ringset_idx].socket) {
//               ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"closing socket %d %p\n",cmd->ringset_idx,socket_satelite_data[cmd->ringset_idx].socket);
               app_glue_close_socket((struct socket *)socket_satelite_data[cmd->ringset_idx].socket);
               socket_satelite_data[cmd->ringset_idx].socket = NULL;
               socket_satelite_data[cmd->ringset_idx].ringset_idx = -1;
               socket_satelite_data[cmd->ringset_idx].parent_idx = -1;
               ipaugenblick_free_socket(cmd->ringset_idx);
	       user_sockets_closed++;
           }
           break;
        case IPAUGENBLICK_SOCKET_TX_KICK_COMMAND:
           if(socket_satelite_data[cmd->ringset_idx].socket) {
               user_kick_tx++;
    //           user_data_available_cbk(socket_satelite_data[cmd->ringset_idx].socket);
               user_on_transmission_opportunity(socket_satelite_data[cmd->ringset_idx].socket);
           }
           break;
        case IPAUGENBLICK_SOCKET_RX_KICK_COMMAND:
           if(socket_satelite_data[cmd->ringset_idx].socket) {
               user_kick_rx++;
               user_data_available_cbk(socket_satelite_data[cmd->ringset_idx].socket);
      //         user_on_transmission_opportunity(socket_satelite_data[cmd->ringset_idx].socket);
           }
           break;
        case IPAUGENBLICK_SET_SOCKET_RING_COMMAND:
           //ipaugenblick_log(IPAUGENBLICK_LOG_DEBUG,"%s %d %d %d %p\n",__FILE__,__LINE__,cmd->ringset_idx,cmd->parent_idx,cmd->u.set_socket_ring.socket_descr);
           socket_satelite_data[cmd->ringset_idx].ringset_idx = cmd->ringset_idx;
	   if(cmd->parent_idx != -1)
	           socket_satelite_data[cmd->ringset_idx].parent_idx = cmd->parent_idx;
	   socket_satelite_data[cmd->ringset_idx].apppid = cmd->u.set_socket_ring.pid;
           app_glue_set_user_data(cmd->u.set_socket_ring.socket_descr,&socket_satelite_data[cmd->ringset_idx]);
           socket_satelite_data[cmd->ringset_idx].socket = cmd->u.set_socket_ring.socket_descr; 
	   //ipaugenblick_log(IPAUGENBLICK_LOG_DEBUG,"setting tx space: %d connidx %d\n",sk_stream_wspace(socket_satelite_data[cmd->ringset_idx].socket->sk),g_ipaugenblick_sockets[socket_satelite_data[cmd->ringset_idx].ringset_idx].connection_idx);
	   user_set_socket_tx_space(&g_ipaugenblick_sockets[socket_satelite_data[cmd->ringset_idx].ringset_idx].tx_space,sk_stream_wspace(socket_satelite_data[cmd->ringset_idx].socket->sk));
//           user_on_transmission_opportunity(socket_satelite_data[cmd->ringset_idx].socket);
           user_data_available_cbk(socket_satelite_data[cmd->ringset_idx].socket);
	   ipaugenblick_mark_writable(&socket_satelite_data[cmd->ringset_idx]);
	   ipaugenblick_mark_readable(&socket_satelite_data[cmd->ringset_idx]);
	   user_client_app_accepted++;
           break;
        case IPAUGENBLICK_SET_SOCKET_SELECT_COMMAND:
//           ipaugenblick_log(IPAUGENBLICK_LOG_DEBUG,"setting selector %d for socket %d\n",cmd->u.set_socket_select.socket_select,cmd->ringset_idx);
           socket_satelite_data[cmd->ringset_idx].parent_idx = cmd->u.set_socket_select.socket_select;
	   socket_satelite_data[cmd->ringset_idx].apppid = cmd->u.set_socket_select.pid;
	   user_data_available_cbk(socket_satelite_data[cmd->ringset_idx].socket);
           ipaugenblick_mark_writable(&socket_satelite_data[cmd->ringset_idx]);
	   ipaugenblick_mark_readable(&socket_satelite_data[cmd->ringset_idx]);
           break;
        case IPAUGENBLICK_SOCKET_TX_POOL_EMPTY_COMMAND:
           if(socket_satelite_data[cmd->ringset_idx].socket) {
               if(!socket_satelite_data[cmd->ringset_idx].socket->buffers_available_notification_queue_present) {
                   TAILQ_INSERT_TAIL(&buffers_available_notification_socket_list_head,socket_satelite_data[cmd->ringset_idx].socket,buffers_available_notification_queue_entry);
                   socket_satelite_data[cmd->ringset_idx].socket->buffers_available_notification_queue_present = 1;
		   if(socket_satelite_data[cmd->ringset_idx].socket->type == SOCK_DGRAM)
		   	user_set_socket_tx_space(&g_ipaugenblick_sockets[socket_satelite_data[cmd->ringset_idx].ringset_idx].tx_space,sk_stream_wspace(socket_satelite_data[cmd->ringset_idx].socket->sk));
               }
           }
           break;
	case IPAUGENBLICK_ROUTE_ADD_COMMAND:
   	   memset((void *)&rtentry,0,sizeof(rtentry));
	   rtentry.rt_metric = cmd->u.route.metric;
	   rtentry.rt_flags = RTF_UP|RTF_GATEWAY;
	   p_sockaddr = (struct sockaddr_in *)&rtentry.rt_dst;
	   p_sockaddr->sin_family = AF_INET;
           p_sockaddr->sin_addr.s_addr = cmd->u.route.dest_ipaddr;
	   p_sockaddr = (struct sockaddr_in *)&rtentry.rt_gateway;
	   p_sockaddr->sin_family = AF_INET;
           p_sockaddr->sin_addr.s_addr = cmd->u.route.next_hop;
	   p_sockaddr = (struct sockaddr_in *)&rtentry.rt_genmask;
	   p_sockaddr->sin_family = AF_INET;
           p_sockaddr->sin_addr.s_addr = cmd->u.route.dest_mask;
	   if(ip_rt_ioctl(&init_net,SIOCADDRT,&rtentry)) {
		ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"CANNOT ADD ROUTE ENTRY %x %x %x\n",
			((struct sockaddr_in *)&rtentry.rt_dst)->sin_addr.s_addr,
			((struct sockaddr_in *)&rtentry.rt_gateway)->sin_addr.s_addr,
			((struct sockaddr_in *)&rtentry.rt_genmask)->sin_addr.s_addr);
	   }
	   else {
		ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"ROUTE ENTRY %x %x %x is added\n",
			((struct sockaddr_in *)&rtentry.rt_dst)->sin_addr.s_addr,
			((struct sockaddr_in *)&rtentry.rt_gateway)->sin_addr.s_addr,
			((struct sockaddr_in *)&rtentry.rt_genmask)->sin_addr.s_addr);
	   }
	   break;
    	case IPAUGENBLICK_ROUTE_DEL_COMMAND:
	   memset((void *)&rtentry,0,sizeof(rtentry));
	   p_sockaddr = (struct sockaddr_in *)&rtentry.rt_dst;
	   p_sockaddr->sin_family = AF_INET;
           p_sockaddr->sin_addr.s_addr = cmd->u.route.dest_ipaddr;
	   p_sockaddr = (struct sockaddr_in *)&rtentry.rt_gateway;
	   p_sockaddr->sin_family = AF_INET;
           p_sockaddr->sin_addr.s_addr = cmd->u.route.next_hop;
	   p_sockaddr = (struct sockaddr_in *)&rtentry.rt_genmask;
	   p_sockaddr->sin_family = AF_INET;
           p_sockaddr->sin_addr.s_addr = cmd->u.route.dest_mask;
	   if(ip_rt_ioctl(&init_net,SIOCDELRT,&rtentry)) {
		ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"CANNOT DELETE ROUTE ENTRY %x %x %x\n",
			((struct sockaddr_in *)&rtentry.rt_dst)->sin_addr.s_addr,
			((struct sockaddr_in *)&rtentry.rt_gateway)->sin_addr.s_addr,
			((struct sockaddr_in *)&rtentry.rt_genmask)->sin_addr.s_addr);
	   }
	   else {
		ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"ROUTE ENTRY %x %x %x is deleted\n",
			((struct sockaddr_in *)&rtentry.rt_dst)->sin_addr.s_addr,
			((struct sockaddr_in *)&rtentry.rt_gateway)->sin_addr.s_addr,
			((struct sockaddr_in *)&rtentry.rt_genmask)->sin_addr.s_addr);
	   }
	   break;
	case IPAUGENBLICK_CONNECT_CLIENT:
	   if(cmd->ringset_idx >= IPAUGENBLICK_CLIENTS_POOL_SIZE) {
		break;
	   }
	   if(!ipaugenblick_clients[cmd->ringset_idx].is_busy) {
	   	TAILQ_INSERT_TAIL(&ipaugenblick_clients_list_head,&ipaugenblick_clients[cmd->ringset_idx],queue_entry);
		ipaugenblick_clients[cmd->ringset_idx].is_busy = 1;
		on_client_connect(cmd->ringset_idx);
	   }
	   break;
	case IPAUGENBLICK_DISCONNECT_CLIENT:
	   if(cmd->ringset_idx >= IPAUGENBLICK_CLIENTS_POOL_SIZE) {
		break;
	   }
	   if(ipaugenblick_clients[cmd->ringset_idx].is_busy) {
	   	TAILQ_REMOVE(&ipaugenblick_clients_list_head,&ipaugenblick_clients[cmd->ringset_idx],queue_entry);
		ipaugenblick_clients[cmd->ringset_idx].is_busy = 0;
	   }
	   break;
	case IPAUGENBLICK_SETSOCKOPT_COMMAND:
	   if(socket_satelite_data[cmd->ringset_idx].socket) { 
	   	sock_setsockopt(socket_satelite_data[cmd->ringset_idx].socket, cmd->u.setsockopt.level, cmd->u.setsockopt.optname, cmd->u.setsockopt.optval, cmd->u.setsockopt.optlen);
	   }
	   break;
	case IPAUGENBLICK_SOCKET_SHUTDOWN_COMMAND:
	   if(socket_satelite_data[cmd->ringset_idx].socket) {
		inet_shutdown(socket_satelite_data[cmd->ringset_idx].socket, cmd->u.socket_shutdown.how);
		user_sockets_shutdown++;
	   }
	   break;
	case IPAUGENBLICK_SOCKET_DECLINE_COMMAND:
	   app_glue_close_socket((struct socket *)cmd->u.socket_decline.socket_descr);
	   user_sockets_closed++;
	   break;
        default:
           ipaugenblick_log(IPAUGENBLICK_LOG_ERR,"unknown cmd %d\n",cmd->cmd);
           break;
    }
    ipaugenblick_free_command_buf(cmd);
}

void ipaugenblick_main_loop()
{
    struct rte_mbuf *mbuf;
    uint8_t ports_to_poll[1] = { 0 };
    int drv_poll_interval = get_max_drv_poll_interval_in_micros(0);
    app_glue_init_poll_intervals(drv_poll_interval/(2*MAX_PKT_BURST),
                                 100 /*timer_poll_interval*/,
                                 drv_poll_interval/(10*MAX_PKT_BURST),
                                drv_poll_interval/(60*MAX_PKT_BURST));
    
    ipaugenblick_service_api_init(COMMAND_POOL_SIZE,DATA_RINGS_SIZE,DATA_RINGS_SIZE);
    TAILQ_INIT(&buffers_available_notification_socket_list_head);
    TAILQ_INIT(&ipaugenblick_clients_list_head);
    init_systick(rte_lcore_id());
    ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"IPAugenblick service initialized\n");
    while(1) {
        process_commands();
	app_glue_periodic(1,ports_to_poll,1);
        while(!TAILQ_EMPTY(&buffers_available_notification_socket_list_head)) {
            if(get_buffer_count() > 0) {
                struct socket *sock = TAILQ_FIRST(&buffers_available_notification_socket_list_head);
                socket_satelite_data_t *socket_data = get_user_data(sock);
		if(socket_data->socket->type == SOCK_DGRAM)
		   	user_set_socket_tx_space(&g_ipaugenblick_sockets[socket_data->ringset_idx].tx_space,sk_stream_wspace(socket_data->socket->sk));
		//printf("%s %d %d %d %d\n",__FILE__,__LINE__,socket_data->ringset_idx,g_ipaugenblick_sockets[socket_data->ringset_idx].tx_space,sk_stream_wspace(socket_data->socket->sk));
                if(!ipaugenblick_mark_writable(socket_data)) { 
                    sock->buffers_available_notification_queue_present = 0;
                    TAILQ_REMOVE(&buffers_available_notification_socket_list_head,sock,buffers_available_notification_queue_entry); 
                }
                else {
                    break;
                }
            }
            else {
                break;
            }
        }
    }
}
/*this is called in non-data-path thread */
void print_user_stats()
{
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"user_on_tx_opportunity_called %"PRIu64"\n",user_on_tx_opportunity_called);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"user_on_tx_opportunity_api_nothing_to_tx %"PRIu64"user_on_tx_opportunity_socket_full %"PRIu64" \n",
                user_on_tx_opportunity_api_nothing_to_tx,user_on_tx_opportunity_socket_full);
        ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"user_kick_tx %"PRIu64" user_kick_rx %"PRIu64" user_kick_select_tx %"PRIu64" user_kick_select_rx %"PRIu64"\n",
                user_kick_tx,user_kick_rx,user_kick_select_tx,user_kick_select_rx);
        ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"user_on_tx_opportunity_cannot_send %"PRIu64"\n",user_on_tx_opportunity_cannot_send);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"user_on_tx_opportunity_socket_send_error %"PRIu64"\n",user_on_tx_opportunity_socket_send_error);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"user_on_tx_opportunity_cannot_get_buff %"PRIu64"\n",user_on_tx_opportunity_cannot_get_buff);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"user_on_tx_opportunity_getbuff_called %"PRIu64"\n",user_on_tx_opportunity_getbuff_called);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"user_on_tx_opportunity_api_failed %"PRIu64"\n",	user_on_tx_opportunity_api_failed);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"user_on_rx_opportunity_called %"PRIu64"\n",user_on_rx_opportunity_called);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"user_on_rx_opportunity_called_exhausted %"PRIu64"\n",user_on_rx_opportunity_called_exhausted);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"user_rx_mbufs %"PRIu64" user_rx_ring_full %"PRIu64"\n",user_rx_mbufs,user_rx_ring_full);
        ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"user_on_tx_opportunity_api_mbufs_sent %"PRIu64"\n",user_on_tx_opportunity_api_mbufs_sent);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"user_client_app_accepted %"PRIu64" user_pending_accept %"PRIu64"\n",user_client_app_accepted, user_pending_accept);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO,"user_sockets_closed %"PRIu64" user_sockets_shutdown %"PRIu64"\n",
	user_sockets_closed, user_sockets_shutdown);
}
