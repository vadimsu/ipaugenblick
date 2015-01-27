#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/errno.h>
#include <string.h>
extern int dpdk_linux_tcpip_init(int argc,char **argv);
extern void ipaugenblick_main_loop(void);

int main(int argc,char **argv)
{
    int ret = dpdk_linux_tcpip_init(argc, argv);
    if (ret < 0) {
	printf("Invalid arguments\n");
	return 1;
    }
    ipaugenblick_main_loop();
    return 0;
}
