RTE_SDK=$(CURRENT_DIR)dpdk-16.07
RTE_TARGET ?= x86_64-native-linuxapp-gcc
include $(RTE_SDK)/mk/rte.vars.mk
SRC_ROOT=$(CURRENT_DIR)
SRCS-y :=  net/ipv4/af_inet.c net/ipv4/arp.c net/ipv4/devinet.c \
net/ipv4/ip_input.c net/ipv4/ip_output.c net/ipv4/ip_options.c net/ipv4/ip_forward.c \
net/ipv4/ip_fragment.c net/ipv4/ip_sockglue.c net/ipv4/ipv4_datagram.c \
net/ipv4/tcp.c net/ipv4/tcp_ipv4.c net/ipv4/tcp_input.c net/ipv4/tcp_output.c \
net/ipv4/tcp_timer.c net/ipv4/tcp_minisocks.c net/ipv4/tcp_cong.c net/ipv4/tcp_fastopen.c \
net/ipv4/tcp_metrics.c net/ipv4/tcp_offload.c net/ipv4/fib_frontend.c net/ipv4/fib_trie.c \
net/ipv4/route.c net/ipv4/protocol.c net/ipv4/fib_semantics.c net/ipv4/inetpeer.c \
net/ipv4/raw.c net/ipv4/ping.c net/ipv4/syncookies.c \
net/ipv4/inet_connection_sock.c net/ipv4/inet_hashtables.c net/ipv4/inet_timewait_sock.c \
net/ipv4/inet_fragment.c net/ipv4/icmp.c net/ipv4/udp.c net/ipv4/udp_offload.c \
net/ipv4/udplite.c net/ipv4/udp_diag.c net/ipv4/cipso_ipv4.c \
net/ethernet/eth.c net/core/filter.c net/core/flow_dissector.c net/core/net_namespace.c \
net/core/neighbour.c net/core/dev.c net/core/dst.c net/core/dev_addr_lists.c net/core/secure_seq.c \
net/core/sock.c net/core/skbuff.c \
net/core/request_sock.c net/core/stream.c net/core/dummies.c net/core/datagram.c net/core/netevent.c \
net/socket.c net/netfilter/core.c net/netfilter/nf_sockopt.c net/netfilter/nf_log.c net/netfilter/nf_queue.c \
arch/x86/lib/csum-partial_64.c arch/x86/lib/csum-wrappers_64.c \
lib/kasprintf.c lib/kstrtox.c lib/md5.c lib/percpu_counter.c lib/find_next_bit.c lib/bitmap.c lib/sha1.c \
crypto/api.c \
drivers/net/loopback.c kernel/notifier.c porting/mm_porting.c porting/timer_porting.c \
porting/timing_porting.c porting/tasklet_workqueues_porting.c porting/app_glue.c \
porting/libinit.c porting/show_mib_stats.c \
drivers/net/dpdk/rx.c drivers/net/dpdk/tx.c \
drivers/net/dpdk/dpdk_sw_loop.c drivers/net/dpdk/device.c service/ipaugenblick_service_loop.c \
log/ipaugenblick_log.c
#CFLAGS += -g
CFLAGS += -Ofast   
CFLAGS += -Wno-deprecated-declarations
LDLIBS += -lrte_eal -lrte_timer -lrte_ring -lrte_mempool -lrte_ethdev -lrte_mbuf
LINUX_HEADERS=$(SRC_ROOT)stack_and_service/
DPDK_HEADERS=$(RTE_SDK)/x86_64-native-linuxapp-gcc/include
ALL_HEADERS = -I$(LINUX_HEADERS) -I$(DPDK_HEADERS) -I$(LINUX_HEADERS)service
CFLAGS += $(ALL_HEADERS) -DMAXCPU=32 -D__UAPI_DEF_IN6_ADDR=1 -D__UAPI_DEF_SOCKADDR_IN6=1 -D__UAPI_DEF_IN6_ADDR_ALT=1 -DCONFIG_INET\
-D__UAPI_DEF_IPPROTO_V6=1 -DCONFIG_SLAB -DCONFIG_HZ=1000 -DNR_CPUS=32 -DCONFIG_64BIT -DCONFIG_SMP \
-DCONFIG_NETFILTER -DCONFIG_NETLABEL -DCONFIG_NET_POLL_CONTROLLER \
-DCONFIG_X86_64 -DCONFIG_GENERIC_ATOMIC64 -DTCP_BIND_CACHE_SIZE=16384 \
-DINET_PEER_CACHE_SIZE=16384 -DSOCK_CACHE_SIZE=32768 -DRUN_TO_COMPLETE -DMAX_PKT_BURST=32 \
-DMULTIPLE_MEM_ALLOC=0 -DOPTIMIZE_SENDPAGES -DOPTIMIZE_TCP_RECEIVE -DCONFIG_SYN_COOKIES -DIPAUGENBLICK_UDP_OPTIMIZATION -DOFFLOAD_NOT_YET

SHARED = libnetinet.so
include $(RTE_SDK)/mk/rte.extshared.mk
