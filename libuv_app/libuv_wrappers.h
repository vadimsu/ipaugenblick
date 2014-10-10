
#ifndef __LIBUV_APP_H__
#define __LIBUV_APP_H__

extern int libuv_is_fd_known(int fd);

extern int libuv_app_socket(int family,int type,int port);

extern int libuv_app_bind(int fd,const struct sockaddr *addr,int addr_len);

extern int libuv_app_connect(int fd,const struct sockaddr *addr,int addr_len);

extern int libuv_app_listen(int fd,int backlog);

extern int libuv_app_close(int fd);

extern int libuv_app_setsockopt(int fd,int scope,int optname,const void *val,int valsize);

extern int libuv_app_getsockopt(int fd,int scope,int optname,void *val,int *valsize);

extern void libuv_app_set_user_data(int fd,void *data);

extern int libuv_app_recvmsg(int fd, void *arg, int len,int flags,void (*copy_to_iovec)(void *,char *,int));

extern int libuv_app_tcp_sendmsg(int fd,void *arg,int len,int flags, int (*copy_from_iovec)(void *,char *,int));

extern int libuv_app_udp_sendmsg(int fd,void *arg,int len,int flags, unsigned int addr,unsigned short port,int (*copy_from_iovec)(void *,char *,int));

extern int libuv_app_tcp_receive(int fd,char *buf,int len);

extern int libuv_app_accept(int fd);

extern int libuv_app_getsockname(int fd,struct sockaddr *addr,int *addrlen);

extern int libuv_app_getpeername(int fd,struct sockaddr *addr,int *addrlen);

extern int libuv_app_is_socket_writable(int fd);

#endif
