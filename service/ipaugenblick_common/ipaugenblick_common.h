
#ifndef __IPAUGENBLICK__COMMON_H__
#define __IPAUGENBLICK__COMMON_H__

enum
{
    IPAUGENBLICK_DUMMY_COMMAND = 0,
    IPAUGENBLICK_OPEN_CLIENT_SOCKET_COMMAND,
    IPAUGENBLICK_OPEN_LISTENING_SOCKET_COMMAND,
    IPAUGENBLICK_OPEN_UDP_SOCKET_COMMAND,
    IPAUGENBLICK_OPEN_RAW_SOCKET_COMMAND,
    IPAUGENBLICK_OPEN_SOCKET_FEEDBACK,
    IPAUGENBLICK_SOCKET_TX_KICK_COMMAND,
    IPAUGENBLICK_SOCKET_RX_KICK_COMMAND,
    IPAUGENBLICK_SOCKET_ACCEPTED_COMMAND,
    IPAUGENBLICK_SET_SOCKET_RING_COMMAND,
    IPAUGENBLICK_SET_SOCKET_SELECT_COMMAND,
    IPAUGENBLICK_SOCKET_READY_FEEDBACK,
    IPAUGENBLICK_SOCKET_CONNECT_COMMAND,
    IPAUGENBLICK_SOCKET_CLOSE_COMMAND,
    IPAUGENBLICK_SOCKET_TX_POOL_EMPTY_COMMAND
};

typedef struct
{
    unsigned int my_ipaddress;
    unsigned int my_port;
    unsigned int peer_ipaddress;
    unsigned int peer_port;
}__attribute__((packed))ipaugenblick_open_client_sock_cmd_t;

typedef struct
{
    unsigned int ipaddress;
    unsigned int port;
}__attribute__((packed))ipaugenblick_open_listening_sock_cmd_t;

typedef struct
{
    unsigned int ipaddress;
    unsigned int port;
}__attribute__((packed))ipaugenblick_open_udp_sock_cmd_t;

typedef struct
{
    unsigned int ipaddress;
    unsigned int protocol;
}__attribute__((packed))ipaugenblick_open_raw_sock_cmd_t;

typedef struct
{
    unsigned long socket_descr;
}__attribute__((packed))ipaugenblick_open_accepted_socket_t;

typedef struct
{
    unsigned long socket_descr;
}__attribute__((packed))ipaugenblick_socket_kick_cmd_t;

typedef struct
{
    unsigned long socket_descr;
}__attribute__((packed))ipaugenblick_set_socket_ring_cmd_t;

typedef struct
{
    int socket_select;
    unsigned long pid;
}__attribute__((packed))ipaugenblick_set_socket_select_cmd_t;

typedef struct
{
    unsigned int ipaddr;
    unsigned short port;
}__attribute__((packed))ipaugenblick_socket_connect_cmd_t;

#define SOCKET_READABLE_BIT 1
#define SOCKET_WRITABLE_BIT 2
#define SOCKET_READY_SHIFT 16
#define SOCKET_READY_MASK 0xFFFF

typedef struct
{
    unsigned int bitmask;
}__attribute__((packed))ipaugenblick_socket_ready_feedback_t;

typedef struct
{
    int cmd;
    unsigned int ringset_idx;
    int          parent_idx;
    union {
        ipaugenblick_open_client_sock_cmd_t open_client_sock;
        ipaugenblick_open_listening_sock_cmd_t open_listening_sock;
        ipaugenblick_open_udp_sock_cmd_t open_udp_sock;
        ipaugenblick_open_raw_sock_cmd_t open_raw_sock;
        ipaugenblick_socket_kick_cmd_t socket_kick_cmd;
        ipaugenblick_open_accepted_socket_t accepted_socket;
        ipaugenblick_set_socket_ring_cmd_t set_socket_ring;
        ipaugenblick_set_socket_select_cmd_t set_socket_select;
        ipaugenblick_socket_ready_feedback_t socket_ready_feedback;
        ipaugenblick_socket_connect_cmd_t socket_connect;
    }u;
}__attribute__((packed))ipaugenblick_cmd_t;

typedef struct
{
    unsigned long connection_idx; /* to be aligned */
    rte_atomic16_t  read_ready_to_app;
    rte_atomic16_t  write_ready_to_app;
    rte_atomic16_t  write_done_from_app
}__attribute__((packed))ipaugenblick_socket_t;

typedef struct
{
    struct rte_ring  *ready_connections;
}__attribute__((packed))ipaugenblick_selector_t;

#define COMMAND_POOL_SIZE 8192
#define DATA_RINGS_SIZE 16384
#define FREE_CONNECTIONS_POOL_NAME "free_connections_pool"
#define FREE_CONNECTIONS_RING "free_connections_ring"
#define COMMAND_RING_NAME "command_ring"
#define FREE_COMMAND_POOL_NAME "free_command_pool"
#define RX_RING_NAME_BASE "rx_ring"
#define TX_RING_NAME_BASE "tx_ring"
#define ACCEPTED_RING_NAME "accepted_ring"
#define FREE_ACCEPTED_POOL_NAME "free_accepted_pool"
#define SELECTOR_POOL_NAME "selector_pool"
#define SELECTOR_RING_NAME "selector_ring"
#define IPAUGENBLICK_CONNECTION_POOL_SIZE 64
#define IPAUGENBLICK_SELECTOR_POOL_SIZE 64
#define COMMON_NOTIFICATIONS_POOL_NAME "common_notifications_pool_name"
#define COMMON_NOTIFICATIONS_RING_NAME "common_notifications_ring_name"

extern struct rte_mempool *free_command_pool;

static inline ipaugenblick_cmd_t *ipaugenblick_get_free_command_buf()
{
    ipaugenblick_cmd_t *cmd;
    if(rte_mempool_get(free_command_pool,(void **)&cmd))
        return NULL;
    return cmd;
}

#endif /* __IPAUGENBLICK_MEMORY_COMMON_H__ */
