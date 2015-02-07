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

uint64_t user_on_tx_opportunity_cycles = 0;
uint64_t user_on_tx_opportunity_called = 0;
uint64_t user_on_tx_opportunity_getbuff_called = 0;
uint64_t user_on_tx_opportunity_api_nothing_to_tx = 0;
uint64_t user_on_tx_opportunity_api_failed = 0;
uint64_t user_on_tx_opportunity_api_mbufs_sent = 0;
uint64_t user_on_tx_opportunity_cannot_get_buff = 0;
uint64_t user_on_rx_opportunity_called = 0;
uint64_t user_on_rx_opportunity_called_exhausted = 0;
uint64_t user_rx_mbufs = 0;
uint64_t user_kick_tx = 0;
uint64_t user_on_tx_opportunity_cannot_send = 0;

uint64_t g_last_time_transmitted = 0;

struct rte_ring *command_ring = NULL;
struct rte_ring *selectors_ring = NULL;
struct rte_mempool *free_connections_pool = NULL;
struct rte_ring *free_connections_ring = NULL;
struct rte_mempool *selectors_pool = NULL;
struct rte_ring *rx_mbufs_ring = NULL;
struct rte_mempool *free_command_pool = NULL;
struct ipaugenblick_ring_set ringsets[IPAUGENBLICK_CONNECTION_POOL_SIZE];
void *ringidx_to_socket[IPAUGENBLICK_CONNECTION_POOL_SIZE];
ipaugenblick_socket_t *g_ipaugenblick_sockets = NULL;
ipaugenblick_selector_t *g_ipaugenblick_selectors = NULL;
//unsigned long app_pid = 0;

static inline void process_commands()
{
    int ringset_idx;
    ipaugenblick_cmd_t *cmd;
    struct rte_mbuf *mbuf;
    struct socket *sock;
    char *p;
    sock_and_selector_idx_t sock_and_selector_idx;

    cmd = ipaugenblick_dequeue_command_buf();
    if(!cmd)
        return;
    switch(cmd->cmd) {
        case IPAUGENBLICK_OPEN_CLIENT_SOCKET_COMMAND:
           printf("open_client_sock %x %x %x %x\n",cmd->u.open_client_sock.my_ipaddress,cmd->u.open_client_sock.my_port,
                                             cmd->u.open_client_sock.peer_ipaddress,cmd->u.open_client_sock.peer_port);
           sock = create_client_socket2(cmd->u.open_client_sock.my_ipaddress,cmd->u.open_client_sock.my_port,
                                        cmd->u.open_client_sock.peer_ipaddress,cmd->u.open_client_sock.peer_port);
           if(sock) {
               printf("setting user data\n");
               RINGSET_IDX(sock_and_selector_idx) = cmd->ringset_idx;
               PARENT_IDX(sock_and_selector_idx) = cmd->parent_idx;
               app_glue_set_user_data(sock,(void *)sock_and_selector_idx.u.data);
               ringidx_to_socket[cmd->ringset_idx] = sock;
           }
           printf("Done\n");
           break;
        case IPAUGENBLICK_OPEN_LISTENING_SOCKET_COMMAND:
           printf("open_listening_sock %x %x\n",
                  cmd->u.open_listening_sock.ipaddress,cmd->u.open_listening_sock.port);
           sock = create_server_socket2(cmd->u.open_listening_sock.ipaddress,cmd->u.open_listening_sock.port);
           if(sock) {
               printf("setting user data\n");
               RINGSET_IDX(sock_and_selector_idx) = cmd->ringset_idx;
               PARENT_IDX(sock_and_selector_idx) = cmd->parent_idx;
               app_glue_set_user_data(sock,(void *)sock_and_selector_idx.u.data);
               ringidx_to_socket[cmd->ringset_idx] = sock;
           }
           break;
        case IPAUGENBLICK_OPEN_UDP_SOCKET_COMMAND:
           printf("open_udp_sock %x %x\n",cmd->u.open_udp_sock.ipaddress,cmd->u.open_udp_sock.port);
           sock = create_udp_socket2(cmd->u.open_udp_sock.ipaddress,cmd->u.open_udp_sock.port);
           if(sock) {
               printf("setting user data %d %p\n",cmd->ringset_idx,sock->sk);
               RINGSET_IDX(sock_and_selector_idx) = cmd->ringset_idx;
               PARENT_IDX(sock_and_selector_idx) = cmd->parent_idx;
               app_glue_set_user_data(sock,(void *)sock_and_selector_idx.u.data);
               ringidx_to_socket[cmd->ringset_idx] = sock;
           }
           break;
        case IPAUGENBLICK_OPEN_RAW_SOCKET_COMMAND:
           printf("open_raw_sock %x %x\n",cmd->u.open_raw_sock.ipaddress,cmd->u.open_raw_sock.protocol);
           sock = create_raw_socket2(cmd->u.open_raw_sock.ipaddress,cmd->u.open_raw_sock.protocol);
           if(sock) {
               printf("setting user data\n");
               RINGSET_IDX(sock_and_selector_idx) = cmd->ringset_idx;
               PARENT_IDX(sock_and_selector_idx) = cmd->parent_idx;
               app_glue_set_user_data(sock,(void *)sock_and_selector_idx.u.data);
               ringidx_to_socket[cmd->ringset_idx] = sock;
           }
           break;
        case IPAUGENBLICK_SOCKET_KICK_COMMAND:
           if(ringidx_to_socket[cmd->ringset_idx]) {
               user_kick_tx++;
               user_on_transmission_opportunity((struct socket *)ringidx_to_socket[cmd->ringset_idx]);
           }
           break;
        case IPAUGENBLICK_SET_SOCKET_RING_COMMAND:
           RINGSET_IDX(sock_and_selector_idx) = cmd->ringset_idx;
           PARENT_IDX(sock_and_selector_idx) = cmd->parent_idx;
           app_glue_set_user_data(cmd->u.set_socket_ring.socket_descr,sock_and_selector_idx.u.data);
           ringidx_to_socket[cmd->ringset_idx] = cmd->u.set_socket_ring.socket_descr;
           break;
        case IPAUGENBLICK_SET_SOCKET_SELECT_COMMAND:
           sock_and_selector_idx.u.data = app_glue_get_user_data((struct socket *)ringidx_to_socket[cmd->ringset_idx]);
       //    app_pid = cmd->u.set_socket_select.pid;
           PARENT_IDX(sock_and_selector_idx) = cmd->u.set_socket_select.socket_select;
           printf("%s %d %d %d\n",__FILE__,__LINE__,RINGSET_IDX(sock_and_selector_idx),PARENT_IDX(sock_and_selector_idx));
           app_glue_set_user_data((struct socket *)ringidx_to_socket[cmd->ringset_idx],sock_and_selector_idx.u.data);
           break;
        case IPAUGENBLICK_SOCKET_CONNECT_COMMAND:
           if(ringidx_to_socket[cmd->ringset_idx]) {
               struct sockaddr_in addr;
               addr.sin_family = AF_INET;
               addr.sin_addr.s_addr = cmd->u.socket_connect.ipaddr;
               addr.sin_port = cmd->u.socket_connect.port;
               if(kernel_connect((struct socket *)ringidx_to_socket[cmd->ringset_idx],(struct sockaddr *)&addr,sizeof(addr),0)) {
                   printf("failed to connect socket\n");
               }
               else {
                   printf("socket connected\n");
               }
           }
           else {
              printf("no socket to invoke command!!!\n");
           }
           break;
       case IPAUGENBLICK_SOCKET_CLOSE_COMMAND:
           printf("closing socket %d\n",cmd->ringset_idx);
           kernel_close((struct socket *)ringidx_to_socket[cmd->ringset_idx]);
           ringidx_to_socket[cmd->ringset_idx] = NULL;
           ipaugenblick_free_socket(cmd->ringset_idx);
           printf("%s %d\n",__FILE__,__LINE__);
           break;
        default:
           printf("unknown cmd %d\n",cmd->cmd);
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
                                 1000 /*timer_poll_interval*/,
                                 drv_poll_interval/(10*MAX_PKT_BURST),
                                drv_poll_interval/(60*MAX_PKT_BURST));
    ipaugenblick_service_api_init(128,1024,1024);
    printf("IPAugenblick service initialized\n");
    while(1) {
        process_commands();
	app_glue_periodic(1,ports_to_poll,1);	
    }
}
/*this is called in non-data-path thread */
void print_user_stats()
{
	printf("user_on_tx_opportunity_called %"PRIu64"\n",user_on_tx_opportunity_called);
	printf("user_on_tx_opportunity_api_nothing_to_tx %"PRIu64"\n",user_on_tx_opportunity_api_nothing_to_tx);
        printf("user_kick_tx user_kick_tx %"PRIu64"\n",user_kick_tx);
        printf("user_on_tx_opportunity_cannot_send %"PRIu64"\n",user_on_tx_opportunity_cannot_send);
	printf("user_on_tx_opportunity_cannot_get_buff %"PRIu64"\n",user_on_tx_opportunity_cannot_get_buff);
	printf("user_on_tx_opportunity_getbuff_called %"PRIu64"\n",user_on_tx_opportunity_getbuff_called);
	printf("user_on_tx_opportunity_api_failed %"PRIu64"\n",	user_on_tx_opportunity_api_failed);
	printf("user_on_rx_opportunity_called %"PRIu64"\n",user_on_rx_opportunity_called);
	printf("user_on_rx_opportunity_called_exhausted %"PRIu64"\n",user_on_rx_opportunity_called_exhausted);
	printf("user_rx_mbufs %"PRIu64"\n",user_rx_mbufs,user_rx_mbufs);
        printf("user_on_tx_opportunity_api_mbufs_sent %"PRIu64"\n",user_on_tx_opportunity_api_mbufs_sent);
}
