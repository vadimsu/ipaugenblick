
#ifndef __LIBUV_APP_H__
#define __LIBUV_APP_H__

int libuv_is_fd_known(int fd);

int libuv_app_socket(int family,int type,int port);

int libuv_app_bind(int fd,struct sockaddr *addr,int addr_len);

int libuv_app_connect(int fd,struct sockaddr *addr,int addr_len);

int libuv_app_listen(int fd,int backlog);

int libuv_app_close(int fd);

int libuv_app_setsockopt(int fd,int scope,int optname,void *val,int valsize);

int libuv_app_getsockopt(int fd,int scope,int optname,void *val,int *valsize);

void libuv_app_set_user_data(int fd,void *data);

int libuv_app_recvmsg(int fd, void *arg, int len,int flags,void (*copy_to_iovec)(void *,char *,int));

int libuv_app_tcp_sendmsg(int fd,void *arg,int len,int flags, int (*copy_from_iovec)(void *,char *,int));

int libuv_app_udp_sendmsg(int fd,void *arg,int len,int flags, unsigned int addr,unsigned short port,int (*copy_from_iovec)(void *,char *,int));

int libuv_app_tcp_receive(int fd,char *buf,int len);

int libuv_app_getsockname(int fd,struct sockaddr *addr,int *addrlen);

int libuv_app_getpeername(int fd,struct sockaddr *addr,int *addrlen);

#endif __LIBUV_APP_H__
