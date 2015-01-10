#include<stdio.h>
#include <sys/param.h>
#define MY_IP_ADDR "192.168.150.63"
#define PEER_IP_ADDR "192.168.150.62"

int main(int argc,char **argv)
{
	int ret = dpdk_linux_tcpip_init(argc, argv);
	if (ret < 0)
	{
		printf("Invalid arguments\n");
		return 1;
	}
	app_init(MY_IP_ADDR,7777);
	return 0;
}

