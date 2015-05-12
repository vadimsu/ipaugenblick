/*
 * libinit.c
 *
 *  Created on: Jul 6, 2014
 *      Author: Vadim Suraev vadim.suraev@gmail.com
 *  Contains functions for initialization of
 *  Linux TCP/IP ported to userland and integrated with DPDK 1.6
 */
#include<stdio.h>
#include <sys/param.h>
#include <rte_config.h>
#include <rte_ethdev.h>
#include <rte_memcpy.h>
#include <getopt.h>
#include <specific_includes/dpdk_drv_iface.h>
#include <pools.h>
#define RTE_RX_DESC_DEFAULT (4096)
#define RTE_TX_DESC_DEFAULT 4096
#define RX_QUEUE_PER_PORT 1
#define TX_QUEUE_PER_PORT 1

//#define MBUFS_PER_RX_QUEUE (RTE_RX_DESC_DEFAULT*2/*+MAX_PKT_BURST*2*/)

//#define MBUFS_PER_TX_QUEUE /*((RTE_RX_DESC_DEFAULT+MAX_PKT_BURST*2)*RX_QUEUE_PER_PORT)*/4096*64
#define MBUF_SIZE (((2048 + sizeof(struct rte_mbuf) + RTE_PKTMBUF_HEADROOM) + RTE_CACHE_LINE_SIZE) & ~(RTE_CACHE_LINE_SIZE))
#define MAX_PORTS	RTE_MAX_ETHPORTS
/*
 * RX and TX Prefetch, Host, and Write-back threshold values should be
 * carefully set for optimal performance. Consult the network
 * controller's datasheet and supporting DPDK documentation for guidance
 * on how these parameters should be set.
 */
#define RX_PTHRESH 8/*1*/ /**< Default values of RX prefetch threshold reg. */
#define RX_HTHRESH 8/*1*/ /**< Default values of RX host threshold reg. */
#define RX_WTHRESH 4/*1*/ /**< Default values of RX write-back threshold reg. */

/*
 * These default values are optimized for use with the Intel(R) 82599 10 GbE
 * Controller and the DPDK ixgbe PMD. Consider using other values for other
 * network controllers and/or network drivers.
 */
#define TX_PTHRESH 36/*0*/ /**< Default values of TX prefetch threshold reg. */
#define TX_HTHRESH 0  /**< Default values of TX host threshold reg. */
#define TX_WTHRESH 0  /**< Default values of TX write-back threshold reg. */

/*
 *
 * Configurable number of RX/TX ring descriptors
 */
static uint16_t nb_rxd = RTE_RX_DESC_DEFAULT;
static uint16_t nb_txd = RTE_TX_DESC_DEFAULT;
/* list of enabled ports */
static uint32_t dst_ports[RTE_MAX_ETHPORTS];
/* mask of enabled ports */
static uint32_t enabled_port_mask = 0;
static unsigned int rx_queue_per_lcore = 1;

struct lcore_conf {
	int n_rtx_port;
	uint16_t tx_queue_id[MAX_PORTS];
	struct ipv4_frag_tbl *frag_tbl[RTE_MAX_LCORE];
	//struct ipv4_frag_death_row death_row;
	//struct mbuf_table tx_mbufs[RTE_MAX_ETHPORTS];
	//struct tx_lcore_stat tx_stat;
} __rte_cache_aligned;
struct port_statistics {
	uint64_t tx;
	uint64_t rx;
	uint64_t submitted_tx;
	uint64_t dropped;
	uint64_t tx_quota_exceeded;
	uint64_t tx_no_bufs;
} __rte_cache_aligned;
/*
 * This function may be called to calculate driver's optimal polling interval .
 * Paramters: a pointer to socket structure
 * Returns: None
 *
 */
int get_max_drv_poll_interval_in_micros(int port_num)
{
	struct rte_eth_link rte_eth_link;
	float bytes_in_sec,bursts_in_sec,bytes_in_burst;

	rte_eth_link_get(port_num,&rte_eth_link);
	switch(rte_eth_link.link_speed) {
	case ETH_LINK_SPEED_10:
		bytes_in_sec = 10/8;
		break;
	case ETH_LINK_SPEED_100:
		bytes_in_sec = 100/8;
		break;
	case ETH_LINK_SPEED_1000:
		bytes_in_sec = 1000/8;
		break;
	case ETH_LINK_SPEED_10000:
		bytes_in_sec = 10000/8;
		break;
	default:
		bytes_in_sec = 10000/8;
	}
	if(rte_eth_link.link_duplex == ETH_LINK_HALF_DUPLEX)
		bytes_in_sec /= 2;
	bytes_in_sec *= 1024*1024;/* x1M*/
	/*MTU*BURST_SIZE*/
	bytes_in_burst = 1448*MAX_PKT_BURST;
	bursts_in_sec = bytes_in_sec / bytes_in_burst;
	/* micros in sec div burst in sec = max poll interval in micros */
	return (int)(1000000/bursts_in_sec)/2/*safe side*/; /* casted to int, is not so large */
}

struct rte_mempool *pool_direct[RX_QUEUE_PER_PORT+1], *pool_indirect = NULL;

struct rte_mempool *get_direct_pool(uint16_t queue_id)
{
    if(queue_id < RX_QUEUE_PER_PORT) {
    	return pool_direct[queue_id];
    }
    printf("PANIC HERE %s %d\n",__FILE__,__LINE__);
    while(1);
    return NULL;
}

struct lcore_conf lcore_queue_conf[RTE_MAX_LCORE];

struct lcore_conf *get_lcore_conf(uint16_t lcore_id)
{
    return &lcore_queue_conf[lcore_id];
}
struct lcore_conf *get_this_lcore_conf()
{
    return &lcore_queue_conf[rte_lcore_id()];
}
/* This function initializes the rx queue rte_mbuf pool */
static void init_rx_queues_mempools()
{
	uint16_t queue_id;
        char pool_name[1024];

/* create the mbuf pools */
//	SET_MBUF_DEBUG_POOL(&g_direct_mbufs[0],&g_direct_mbuf_idx);
        for(queue_id = 0;queue_id < RX_QUEUE_PER_PORT;queue_id++) {
                sprintf(pool_name,"pool_direct%d",queue_id);
                pool_direct[queue_id] =
				rte_mempool_create(pool_name, MBUFS_PER_RX_QUEUE,
						   MBUF_SIZE, 0,
						   sizeof(struct rte_pktmbuf_pool_private),
						   rte_pktmbuf_pool_init, NULL,
						   rte_pktmbuf_init, NULL,
						   rte_socket_id(), 0);
			if (pool_direct[queue_id] == NULL)
				rte_panic("Cannot init direct mbuf pool\n");
        }
}

static const struct rte_eth_conf port_conf = {
	.rxmode = {
		.split_hdr_size = 0,
		.header_split   = 0, /**< Header Split disabled */
		.hw_ip_checksum = 0, /**< IP checksum offload disabled */
		.hw_vlan_filter = 0, /**< VLAN filtering disabled */
		.jumbo_frame    = 1, /**< Jumbo Frame Support disabled */
                .max_rx_pkt_len = 0x2000,
		.hw_strip_crc   = 0, /**< CRC stripped by hardware */
		.mq_mode = ETH_MQ_RX_NONE/*ETH_MQ_RX_RSS*/,
	},
	.txmode = {
		.mq_mode = ETH_MQ_TX_NONE/*ETH_MQ_TX_VMDQ_ONLY*/,
	},
};

static const struct rte_eth_rxconf rx_conf = {
	.rx_thresh = {
		.pthresh = RX_PTHRESH,
		.hthresh = RX_HTHRESH,
		.wthresh = RX_WTHRESH,
	},
        .rx_free_thresh = MAX_PKT_BURST/*0*/,
        .rx_drop_en = 0,
};

static const struct rte_eth_txconf tx_conf = {
	.tx_thresh = {
		.pthresh = TX_PTHRESH,
		.hthresh = TX_HTHRESH,
		.wthresh = TX_WTHRESH,
	},
	.tx_free_thresh = /*0*/MAX_PKT_BURST, /* Use PMD default values */
	.tx_rs_thresh = /*0*/MAX_PKT_BURST, /* Use PMD default values */
        .txq_flags = ((uint32_t)/*ETH_TXQ_FLAGS_NOMULTSEGS | \*/
			    /*ETH_TXQ_FLAGS_NOOFFLOADS*/0),
};

static int parse_portmask(const char *portmask)
{
	char *end = NULL;
	unsigned long pm;

	/* parse hexadecimal string */
	pm = strtoul(portmask, &end, 16);
	if ((portmask[0] == '\0') || (end == NULL) || (*end != '\0'))
		return -1;

	if (pm == 0)
		return -1;

	return pm;
}
#define DUMP(varname) printf("%s = %x\n", #varname, varname);
/* Parse the argument given in the command line of the application */
static int parse_args(int argc, char **argv)
{
	int opt, ret;
	char **argvopt;
	int option_index;
	char *prgname = argv[0];
	static struct option lgopts[] = {
		{NULL, 0, 0, 0}
	};
DUMP(argc);
	argvopt = argv;

	while ((opt = getopt_long(argc, argvopt, "p:a:T",
				  lgopts, &option_index)) != EOF) {

		switch (opt) {
		/* portmask */
		case 'p':
			enabled_port_mask = parse_portmask(optarg);
			if (enabled_port_mask == 0) {
				printf("invalid portmask\n");
				//l2fwd_usage(prgname);
				return -1;
			}
			break;

		/* nqueue */
		case 'q':
			//l2fwd_rx_queue_per_lcore = l2fwd_parse_nqueue(optarg);
			//if (l2fwd_rx_queue_per_lcore == 0) {
				//printf("invalid queue number\n");
				//l2fwd_usage(prgname);
				//return -1;
			//}
			break;

		default:
			//l2fwd_usage(prgname);
			return -1;
		}
	}

	if (optind >= 0)
		argv[optind-1] = prgname;

	ret = optind-1;
	optind = 0; /* reset getopt lib */
	return ret;
}

static struct rte_mempool *mbufs_mempool = NULL;

extern unsigned long tcp_memory_allocated;
extern uint64_t sk_stream_alloc_skb_failed;
extern uint64_t write_sockets_queue_len;
extern uint64_t read_sockets_queue_len;
extern struct rte_mempool *free_command_pool;
extern volatile unsigned long long jiffies;
void show_mib_stats(void);
/* This function only prints statistics, it it not called on data path */
static int print_stats(__attribute__((unused)) void *dummy)
{
	while(1) {
#if 1
		app_glue_print_stats();
		show_mib_stats();
        dpdk_dev_print_stats();
		print_user_stats();
		printf("sk_stream_alloc_skb_failed %"PRIu64"\n",sk_stream_alloc_skb_failed);
		printf("tcp_memory_allocated=%"PRIu64"\n",tcp_memory_allocated);
		printf("jiffies %"PRIu64"\n",jiffies);
		dump_header_cache();
		dump_head_cache();
		dump_fclone_cache();
		printf("rx pool free count %d\n",rte_mempool_count(pool_direct[0]));
		printf("stack pool free count %d\n",rte_mempool_count(mbufs_mempool));
                printf("write_sockets_queue_len %"PRIu64" read_sockets_queue_len %"PRIu64" command pool %d \n",
                       write_sockets_queue_len,read_sockets_queue_len,free_command_pool ? rte_mempool_count(free_command_pool) : -1);
		print_skb_iov_stats();
#endif
		sleep(1);
	}
	return 0;
}
/* This function allocates rte_mbuf */
void *get_buffer()
{
	return rte_pktmbuf_alloc(mbufs_mempool);
}
/* this function gets a pointer to data in the newly allocated rte_mbuf */
void *get_data_ptr(void *buf)
{
	struct rte_mbuf *mbuf = (struct rte_mbuf *)buf;
	return rte_pktmbuf_mtod(mbuf,void *);
}
/* this function returns an available mbufs count */
int get_buffer_count()
{
	return rte_mempool_count(mbufs_mempool);
}
/* this function releases the rte_mbuf */
void release_buffer(void *buf)
{
	struct rte_mbuf *mbuf = (struct rte_mbuf *)buf;
	rte_pktmbuf_free_seg(mbuf);
}
typedef struct
{
	int  port_number;
	char ip_addr_str[20];
	char ip_mask_str[20];
}dpdk_dev_config_t;
#define ALIASES_MAX_NUMBER 4
static dpdk_dev_config_t dpdk_dev_config[RTE_MAX_ETHPORTS*ALIASES_MAX_NUMBER];
static void *dpdk_devices[RTE_MAX_ETHPORTS];
/* This function returns a pointer to kernel's interface structure, required to access the driver */
void *get_dpdk_dev_by_port_num(int port_num)
{
	if(port_num >= RTE_MAX_ETHPORTS) {
		return NULL;
	}
	return dpdk_devices[port_num];
}

extern char *get_dev_name(void *netdev);
#define IFACE_NAME_SIZE 20

int get_all_addresses(unsigned char *buf)
{
    uint32_t dev_idx,offset = 0,prefix;
    uint8_t flags = (1<<0)|(1<<1)|(1<<6),family = 2 /*AF_INET*/, prefix_len = 32;

    for(dev_idx = 0;dev_idx < RTE_MAX_ETHPORTS;dev_idx++) {
	if((dpdk_devices[dev_idx])&&(dpdk_dev_config[dev_idx].port_number != -1)) {
		rte_memcpy(&buf[offset],&dev_idx,sizeof(dev_idx));
		offset += sizeof(dev_idx);
		rte_memcpy(&buf[offset],&flags,sizeof(flags));
		offset += sizeof(flags);
		rte_memcpy(&buf[offset],&family,sizeof(family));
		offset += sizeof(family);
		prefix = inet_addr(dpdk_dev_config[dev_idx].ip_addr_str);
		rte_memcpy(&buf[offset],&prefix,sizeof(prefix));
		offset += sizeof(prefix);
		rte_memcpy(&buf[offset],&prefix_len,sizeof(prefix_len));
		offset += sizeof(prefix_len);
		prefix = 0;
		rte_memcpy(&buf[offset],&prefix,sizeof(prefix));
		offset += sizeof(prefix);
		rte_memcpy(&buf[offset],&prefix_len,sizeof(prefix_len));
		offset += sizeof(prefix_len);
	}
    }
    return offset;
}

int get_all_devices(unsigned char *buf)
{
    uint32_t dev_idx,offset = 0,metric = 1,mtu = 1500,mtu6 = 1500,bandwidth = 0,hw_addr_len = 6;
    uint64_t flags = (1<<0)|(1<<1)|(1<<6);
    uint8_t status = 0,sockaddr_dl[6];

    for(dev_idx = 0;dev_idx < RTE_MAX_ETHPORTS;dev_idx++) {
	if(dpdk_devices[dev_idx]) {		
		rte_memcpy(&buf[offset],get_dev_name(dpdk_devices[dev_idx]),IFACE_NAME_SIZE);
		offset += IFACE_NAME_SIZE;
		rte_memcpy(&buf[offset],&dev_idx,sizeof(dev_idx));
		offset += sizeof(dev_idx);	
		rte_memcpy(&buf[offset],&status,sizeof(status));
		offset += sizeof(status);
		rte_memcpy(&buf[offset],&flags,sizeof(flags));
		offset += sizeof(flags);
		rte_memcpy(&buf[offset],&metric,sizeof(metric));
		offset += sizeof(metric);
		rte_memcpy(&buf[offset],&mtu,sizeof(mtu));
		offset += sizeof(mtu);
		rte_memcpy(&buf[offset],&mtu6,sizeof(mtu6));
		offset += sizeof(mtu6);
		rte_memcpy(&buf[offset],&bandwidth,sizeof(bandwidth));
		offset += sizeof(bandwidth);
		hw_addr_len = htonl(hw_addr_len);
		rte_memcpy(&buf[offset],&hw_addr_len,sizeof(hw_addr_len));
		offset += sizeof(hw_addr_len);
		rte_memcpy(&buf[offset],sockaddr_dl,sizeof(sockaddr_dl));
		offset += sizeof(sockaddr_dl);
	}
    }
    return offset;
}
/* Helper function to read configuration file */
static int get_dpdk_ip_stack_config()
{
	FILE *p_config_file;
	int i,rc;
	p_config_file = fopen("dpdk_ip_stack_config.txt","r");
	if(!p_config_file){
		printf("cannot open dpdk_ip_stack_config.txt");
		return -1;
	}
	for(i = 0;i < RTE_MAX_ETHPORTS*ALIASES_MAX_NUMBER;i++) {
		dpdk_dev_config[i].port_number = -1;
	}
	i = 0;
	while(!feof(p_config_file)) {
		rc = fscanf(p_config_file,"%d %s %s",&dpdk_dev_config[i].port_number,dpdk_dev_config[i].ip_addr_str,dpdk_dev_config[i].ip_mask_str);
		if(!rc){
			continue;
		}
		printf("retrieved config entry %d %s %s\n",dpdk_dev_config[i].port_number,dpdk_dev_config[i].ip_addr_str,dpdk_dev_config[i].ip_mask_str);
		i++;
		if(i == RTE_MAX_ETHPORTS*ALIASES_MAX_NUMBER) {
			break;
		}
	}
	fclose(p_config_file);
	return 0;
}
static dpdk_dev_config_t *get_dpdk_config_entry(int portnum)
{
	int i;
	for(i = 0;i < RTE_MAX_ETHPORTS*ALIASES_MAX_NUMBER;i++) {
		if(dpdk_dev_config[i].port_number == portnum) {
			return &dpdk_dev_config[i];
		}
	}
	return NULL;
}
/*
 * This function must be called prior any other in this package.
 * It initializes all the DPDK libs, reads the configuration, initializes the stack's
 * subsystems, allocates mbuf pools etc.
 * Parameters: refer to DPDK EAL parameters.
 * For example -c <core mask> -n <memory channels> -- -p <port mask>
 */
int dpdk_linux_tcpip_init(int argc,char **argv)
{
	int ret,sub_if_idx = 0;
	uint8_t nb_ports;
	uint8_t portid, last_port;
    uint16_t queue_id;
	struct lcore_conf *conf;
	struct rte_eth_dev_info dev_info;
	unsigned nb_ports_in_mask = 0;
	uint8_t nb_ports_available;
	unsigned lcore_id, core_count;
    int rx_lcore_id = 1;
	int tx_lcore_id = 2;
    unsigned main_loop_lcore_id;
	struct ether_addr mac_addr;
	dpdk_dev_config_t *p_dpdk_dev_config;

	if(get_dpdk_ip_stack_config() != 0){
		printf("%s %d\n",__FILE__,__LINE__);
		return -1;
	}
	memset(dpdk_devices,0,sizeof(dpdk_devices));
	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Invalid EAL arguments\n");
	init_lcores();
	net_ns_init();
	netfilter_init();
	net_dev_init();
	skb_init();
	inet_init();
	app_glue_init();
    argc -= ret;
	argv += ret;
    ret = parse_args(argc, argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Invalid APP arguments\n");
	/* init RTE timer library */
	rte_timer_subsystem_init();

	init_rx_queues_mempools();
	mbufs_mempool = rte_mempool_create("mbufs_mempool", APP_MBUFS_POOL_SIZE,
							   MBUF_SIZE, 0,
							   sizeof(struct rte_pktmbuf_pool_private),
							   rte_pktmbuf_pool_init, NULL,
							   rte_pktmbuf_init, NULL,
							   rte_socket_id(), 0);
	if(mbufs_mempool == NULL) {
		printf("%s %d\n",__FILE__,__LINE__);
		exit(0);
	}
	rte_set_log_type(RTE_LOGTYPE_PMD,1);
	rte_set_log_level(RTE_LOG_DEBUG);
#ifndef DPDK_SW_LOOP /* enable when at least one compatable NIC */
	/* init driver(s) */
#if 0
	if (rte_pmd_init_all() < 0)
		rte_exit(EXIT_FAILURE, "Cannot init pmd\n");

	if (rte_eal_pci_probe() < 0)
		rte_exit(EXIT_FAILURE, "Cannot probe PCI\n");
#else
#endif
	nb_ports = rte_eth_dev_count();
	if (nb_ports == 0)
		rte_exit(EXIT_FAILURE, "No Ethernet ports - bye\n");

	if (nb_ports > RTE_MAX_ETHPORTS)
		nb_ports = RTE_MAX_ETHPORTS;
	core_count = rte_lcore_count();
	/*
	 * Each logical core is assigned a dedicated TX queue on each port.
	 */
	for (portid = 0; portid < nb_ports; portid++) {
		/* skip ports that are not enabled */
		if ((enabled_port_mask & (1 << portid)) == 0)
			continue;

		if (nb_ports_in_mask % 2) {
			dst_ports[portid] = last_port;
			dst_ports[last_port] = portid;
		}
		else
			last_port = portid;

		nb_ports_in_mask++;

		rte_eth_dev_info_get(portid, &dev_info);
	}
	        
	/* Initialize the port/queue configuration of each logical core */
	for (portid = 0; portid < nb_ports; portid++) {
		/* skip ports that are not enabled */
		if ((enabled_port_mask & (1 << portid)) == 0)
			continue;
		for(lcore_id = 0;lcore_id < core_count;lcore_id++) {
        	    if(rte_lcore_is_enabled(lcore_id)) {
        	    	conf = get_lcore_conf(lcore_id);
        	        conf->n_rtx_port = -1;
        	        //			conf->tx_mbufs[portid].len = 0;
        	        printf("Lcore %u: Port %u\n", lcore_id,portid);
	            }
	    }
	}
	printf("MASTER LCORE %d\n",rte_get_master_lcore());


        nb_ports_available = nb_ports;

	/* Initialise each port */
	for (portid = 0; portid < nb_ports; portid++) {
		/* skip ports that are not enabled */
		if ((enabled_port_mask & (1 << portid)) == 0) {
			printf("Skipping disabled port %u\n", (unsigned) portid);
			nb_ports_available--;
			continue;
		}
		/* init port */
		printf("Initializing port %u... ", (unsigned) portid);
		fflush(stdout);
		ret = rte_eth_dev_configure(portid, RX_QUEUE_PER_PORT, TX_QUEUE_PER_PORT, &port_conf);
		if (ret < 0)
			rte_exit(EXIT_FAILURE, "Cannot configure device: err=%d, port=%u\n",
					ret, (unsigned) portid);

//                retrieve_eth_addr_for_port(portid);

		/* init one RX queue */
		fflush(stdout);
        for(queue_id = 0;queue_id < RX_QUEUE_PER_PORT;queue_id++) {
		    ret = rte_eth_rx_queue_setup(portid, queue_id, nb_rxd,
			   		rte_eth_dev_socket_id(portid), &rx_conf,
					get_direct_pool(queue_id));
			if (ret < 0)
				rte_exit(EXIT_FAILURE, "rte_eth_rx_queue_setup:err=%d, port=%u\n",
						ret, (unsigned) portid);
		}
		/* init one TX queue on each port */
		fflush(stdout);
		for(queue_id = 0;queue_id < TX_QUEUE_PER_PORT;queue_id++) {
			ret = rte_eth_tx_queue_setup(portid, queue_id, nb_txd,
					rte_eth_dev_socket_id(portid), &tx_conf);
			if (ret < 0)
				rte_exit(EXIT_FAILURE, "rte_eth_tx_queue_setup:err=%d, port=%u\n",
						ret, (unsigned) portid);
		}
		/* Start device */
		ret = rte_eth_dev_start(portid);
		if (ret < 0)
			rte_exit(EXIT_FAILURE, "rte_eth_dev_start:err=%d, port=%u\n",
					ret, (unsigned) portid);

		printf("done: \n");

//		rte_eth_promiscuous_enable(portid);
	}
	rte_set_log_type(RTE_LOGTYPE_PMD,1);
	rte_set_log_level(RTE_LOG_DEBUG);	
#endif
	for (portid = 0; portid < nb_ports; portid++) {
		/* skip ports that are not enabled */
		if ((enabled_port_mask & (1 << portid)) == 0)
			continue;
		p_dpdk_dev_config = get_dpdk_config_entry(portid);
		if(p_dpdk_dev_config) {
			dpdk_devices[portid] = create_netdev(portid);
		    rte_eth_macaddr_get(portid,&mac_addr);
		    set_dev_addr(dpdk_devices[portid],mac_addr.addr_bytes,p_dpdk_dev_config->ip_addr_str,p_dpdk_dev_config->ip_mask_str);
                    sub_if_idx = portid;
                    while(sub_if_idx < RTE_MAX_ETHPORTS*ALIASES_MAX_NUMBER) {
                        p_dpdk_dev_config++;
                        if(p_dpdk_dev_config->port_number != portid) {
                            printf("no more addressed for port %d\n",portid);
                            break;
                        }
                        printf("Adding port%d address %s\n",portid,p_dpdk_dev_config->ip_addr_str);
                        add_dev_addr(dpdk_devices[portid],sub_if_idx - portid,p_dpdk_dev_config->ip_addr_str,p_dpdk_dev_config->ip_mask_str);
                        sub_if_idx++;
                    }
		    printf("%s %d\n",__FILE__,__LINE__);
		}
	}
#ifdef DPDK_SW_LOOP
	init_dpdk_sw_loop();
#endif
#ifdef RUN_TO_COMPLETE
	rte_eal_remote_launch(print_stats, NULL, 1);
#endif
#if 0
	rte_eal_remote_launch(print_stats, NULL, /*CALL_MASTER*/3);
//	while(1)sleep(1000);
	RTE_LCORE_FOREACH_SLAVE(lcore_id) {
		if (rte_eal_wait_lcore(lcore_id) < 0)
			return -1;
	}
#endif
	return 0;
}

