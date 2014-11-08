#include<stdio.h>
#include <sys/param.h>

int main(int argc,char **argv)
{
	int ret = dpdk_linux_tcpip_init(argc, argv);
	if (ret < 0)
	{
		printf("Invalid arguments\n");
		return 1;
	}
	app_init("dpdk_if0");
	return 0;
}

