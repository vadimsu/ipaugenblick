#define _GNU_SOURCE
#include <sched.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <sys/param.h>

int app_main_loop(void *dummy);

int main(int argc,char **argv)
{
    int ret;
    pthread_t thread;
    cpu_set_t cpuset;

    ret = dpdk_linux_tcpip_init(argc, argv,app_main_loop);
    if (ret < 0) {
	printf("Invalid arguments\n");
	return 1;
    }
    return 0;
}

