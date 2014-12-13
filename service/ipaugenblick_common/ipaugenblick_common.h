
#ifndef __IPAUGENBLICK__COMMON_H__
#define __IPAUGENBLICK_COMMON_H__

enum
{
    IPAUGENBLICK_DUMMY_COMMAND = 0,
    IPAUGENBLICK_OPEN_CLIENT_SOCKET_COMMAND,
    IPAUGENBLICK_OPEN_LISTENING_SOCKET_COMMAND,
    IPAUGENBLICK_OPEN_UDP_SOCKET_COMMAND,
    IPAUGENBLICK_OPEN_RAW_SOCKET_COMMAND
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
    int cmd;
    unsigned int ringset_idx;
    union {
        ipaugenblick_open_client_sock_cmd_t open_client_sock;
        ipaugenblick_open_listening_sock_cmd_t open_listening_sock;
        ipaugenblick_open_udp_sock_cmd_t open_udp_sock;
        ipaugenblick_open_raw_sock_cmd_t open_raw_sock;
    }u;
}__attribute__((packed))ipaugenblick_cmd_t;

#define COMMAND_POOL_SIZE 100
#define FREE_CONNECTIONS_RING "free_connections_ring"
#define COMMAND_RING_NAME "command_ring"
#define FREE_COMMAND_POOL_NAME "free_command_pool"
#define RX_RING_NAME_BASE "rx_ring"
#define TX_RING_NAME_BASE "tx_ring" 
#define IPAUGENBLICK_CONNECTION_POOL_SIZE 1000

#endif /* __IPAUGENBLICK_MEMORY_COMMON_H__ */
