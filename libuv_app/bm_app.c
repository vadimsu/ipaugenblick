#include "uv.h"
#include "task.h"
#include <stdio.h>
#include <stdlib.h>
#include "libuv_app/libuv_wrappers.h"

typedef struct {
  uv_write_t req;
  uv_buf_t buf;
} write_req_t;

static uv_loop_t* loop;

static int server_closed;
static stream_type serverType;
static uv_tcp_t tcpServer;
static uv_udp_t udpServer;
static uv_pipe_t pipeServer;
static uv_handle_t* server;

static void after_write(uv_write_t* req, int status);
static void after_read(uv_stream_t*, ssize_t nread, const uv_buf_t* buf);
static void on_close(uv_handle_t* peer);
static void on_server_close(uv_handle_t* handle);
static void on_connection(uv_stream_t*, int status);

static char gIpAddr[20];
static unsigned short gPort;

static void after_write(uv_write_t* req, int status) {
  write_req_t* wr;
printf("%s %d\n",__FILE__,__LINE__);
  /* Free the read/write buffer and the request */
  wr = (write_req_t*) req;
  free(wr->buf.base);

  if (status == 0) {
    free(wr);
    return;
  }

  fprintf(stderr,
          "uv_write error: %s - %s\n",
          uv_err_name(status),
          uv_strerror(status));

  if (!uv_is_closing((uv_handle_t*) req->handle))
    uv_close((uv_handle_t*) req->handle, on_close);
  free(wr);
}


static void after_read(uv_stream_t* handle,
                       ssize_t nread,
                       const uv_buf_t* buf) {
  int i;
  write_req_t *wr;
printf("%s %d\n",__FILE__,__LINE__);
  if (nread < 0) {
    /* Error or EOF */
    ASSERT(nread == UV_EOF);

    if (buf->base) {
      free(buf->base);
    }

    uv_close((uv_handle_t*) handle, on_close);
    return;
  }

  if (nread == 0) {
    /* Everything OK, but nothing read. */
    free(buf->base);
    return;
  }

  /*
   * Scan for the letter Q which signals that we should quit the server.
   * If we get QS it means close the stream.
   */
  if (!server_closed) {
    for (i = 0; i < nread; i++) {
      if (buf->base[i] == 'Q') {
        if (i + 1 < nread && buf->base[i + 1] == 'S') {
          free(buf->base);
          uv_close((uv_handle_t*)handle, on_close);
          return;
        } else {
          uv_close(server, on_server_close);
          server_closed = 1;
        }
      }
    }
  }

  wr = (write_req_t*) malloc(sizeof *wr);
  ASSERT(wr != NULL);
  wr->buf = uv_buf_init(buf->base, nread);

  if (uv_write(&wr->req, handle, &wr->buf, 1, after_write)) {
    FATAL("uv_write failed");
  }
}


static void on_close(uv_handle_t* peer) {
  free(peer);
}


static void echo_alloc(uv_handle_t* handle,
                       size_t suggested_size,
                       uv_buf_t* buf) {
  buf->base = malloc(suggested_size);
  buf->len = suggested_size;
}


static void on_connection(uv_stream_t* server, int status) {
  uv_stream_t* stream;
  int r;

  if (status != 0) {
    fprintf(stderr, "Connect error %s\n", uv_err_name(status));
  }
  ASSERT(status == 0);

  switch (serverType) {
  case TCP:
    stream = malloc(sizeof(uv_tcp_t));
    ASSERT(stream != NULL);
    r = uv_tcp_init(loop, (uv_tcp_t*)stream);
    ASSERT(r == 0);
    break;

  case PIPE:
    stream = malloc(sizeof(uv_pipe_t));
    ASSERT(stream != NULL);
    r = uv_pipe_init(loop, (uv_pipe_t*)stream, 0);
    ASSERT(r == 0);
    break;

  default:
    ASSERT(0 && "Bad serverType");
    abort();
  }

  /* associate server with stream */
  stream->data = server;

  r = uv_accept(server, stream);
  ASSERT(r == 0);

  r = uv_read_start(stream, echo_alloc, after_read);
  ASSERT(r == 0);
}


static void on_server_close(uv_handle_t* handle) {
  ASSERT(handle == server);
}


static void on_send(uv_udp_send_t* req, int status);


static void on_recv(uv_udp_t* handle,
                    ssize_t nread,
                    const uv_buf_t* rcvbuf,
                    const struct sockaddr* addr,
                    unsigned flags) {
  uv_udp_send_t* req;
  uv_buf_t sndbuf;
#if 0
  ASSERT(nread > 0);
  ASSERT(addr->sa_family == AF_INET);

  req = malloc(sizeof(*req));
  ASSERT(req != NULL);

  sndbuf = *rcvbuf;
  ASSERT(0 == uv_udp_send(req, handle, &sndbuf, 1, addr, on_send));
#else
//  ASSERT(nread > 0);
  if(nread == 0)
      return;
  if(!libuv_app_is_socket_writable(udpServer.io_watcher.fd))
      return;
  req = malloc(sizeof(*req));
  ASSERT(req != NULL);
  sndbuf = *rcvbuf;
  sndbuf.len = nread;
  ASSERT(0 == uv_udp_send(req, handle, &sndbuf, 1, addr, on_send));
#endif
}


static void on_send(uv_udp_send_t* req, int status) {
  ASSERT(status == 0);
  free(req);
}


static int tcp4_echo_start(int port) {
  struct sockaddr_in addr;
  int r;

  ASSERT(0 == uv_ip4_addr(gIpAddr, port, &addr));

  server = (uv_handle_t*)&tcpServer;
  serverType = TCP;

  r = uv_tcp_init(loop, &tcpServer);
  if (r) {
    /* TODO: Error codes */
    fprintf(stderr, "Socket creation error\n");
    return 1;
  }

  r = uv_tcp_bind(&tcpServer, (const struct sockaddr*) &addr, 0);
  if (r) {
    /* TODO: Error codes */
    fprintf(stderr, "Bind error\n");
    return 1;
  }
  
  r = uv_listen((uv_stream_t*)&tcpServer, SOMAXCONN, on_connection);
  if (r) {
    /* TODO: Error codes */
    fprintf(stderr, "Listen error %s\n", uv_err_name(r));
    exit(0);
    return 1;
  }
  libuv_app_set_user_data(tcpServer.io_watcher.fd, &tcpServer); 

  return 0;
}

static int udp4_echo_start(int port) {
  struct sockaddr_in addr;
  int r;

  server = (uv_handle_t*)&udpServer;
  serverType = UDP;
  
  ASSERT(0 == uv_ip4_addr(gIpAddr, port, &addr));

  r = uv_udp_init(loop, &udpServer);
  if (r) {
    fprintf(stderr, "uv_udp_init: %s\n", uv_strerror(r));
    return 1;
  }
  
  r = uv_udp_bind(server, (const struct sockaddr*) &addr, 0);
  ASSERT(r == 0);

  libuv_app_set_user_data(udpServer.io_watcher.fd, &udpServer);

  r = uv_udp_recv_start(&udpServer, echo_alloc, on_recv);
  if (r) {
    fprintf(stderr, "uv_udp_recv_start: %s\n", uv_strerror(r));
    return 1;
  }

  return 0;
}

int tcp4_echo_server() 
{
  if (tcp4_echo_start(gPort))
    return 1;

  return 0;
}

int udp4_echo_server() { 

  if (udp4_echo_start(gPort))
    return 1;
 
  return 0;
}

void app_main_loop(char *my_ip_addr,unsigned short port)
{
    loop = uv_default_loop();
#if 0
    uint8_t ports_to_poll[1] = { 0 };
	int drv_poll_interval = get_max_drv_poll_interval_in_micros(0);
	app_glue_init_poll_intervals(drv_poll_interval/(2*MAX_PKT_BURST),
	                             1000 /*timer_poll_interval*/,
	                             drv_poll_interval/(10*MAX_PKT_BURST),
	                             drv_poll_interval/(60*MAX_PKT_BURST));
	while(1) {
		app_glue_periodic(1,ports_to_poll,1);
	}
#endif
    strcpy(gIpAddr,my_ip_addr);
    gPort = port;
//    tcp4_echo_server();
    udp4_echo_server();
    uv_run(loop, UV_RUN_DEFAULT);
}

