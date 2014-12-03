
#ifndef __IPAUGENBLICK_MEMORY_COMMON_H__
#define __IPAUGENBLICK_MEMORY_COMMON_H__

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
    unsigned int ipaddress;
    unsigned int port;
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
    union {
        ipaugenblick_open_client_sock_cmd_t open_client_sock;
        ipaugenblick_open_listening_sock_cmd_t open_listening_sock;
        ipaugenblick_open_udp_sock_cmd_t open_udp_sock;
        ipaugenblick_open_raw_sock_cmd_t open_raw_sock;
    }u;
}__attribute__((packed))ipaugenblick_cmd_t;


void *ipaugenblick_shared_memory_init(int size);

#endif /* __IPAUGENBLICK_MEMORY_COMMON_H__ */
