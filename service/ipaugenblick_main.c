#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/errno.h>
#include "ipaugenblick_memory_layout.h"
#include <string.h>

#define MEM_SIZE (2*1024*1024)
#define ADDR (void *)(0x0000000000000000UL)
#define FILE_NAME "/mnt/hugetlb"

void ipaugenblick_main_loop();

static struct ipaugenblick_memory *ipaugenblick_memory = NULL;

void *get_ipaugenblick_memory()
{
    return (void *)ipaugenblick_memory;
}

int main(int argc,char **argv)
{
    void *pa;
    int hugetlb_fd;
    
    pa = ADDR;
    hugetlb_fd = open(FILE_NAME,O_CREAT | O_RDWR, 0755);
    if(hugetlb_fd <= 0) {
        printf("cannot open hugetlb\n");
        return -1;
    }
    pa = mmap(pa, MEM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED/*MAP_PRIVATE*/ | MAP_ANONYMOUS |
MAP_POPULATE | MAP_HUGETLB, hugetlb_fd, 0);
    if(pa == MAP_FAILED) {
        printf("cannot mmap2\n");
        close(hugetlb_fd);
        return -1;
    }
    ipaugenblick_memory = ipaugenblick_init(pa,MEM_SIZE, 16,16);
    if(!ipaugenblick_memory) {
        printf("cannot initialize service memory\n");
        munmap(pa,MEM_SIZE);
        close(hugetlb_fd);
        return -1;
    }
    dpdk_linux_tcpip_init(argc, argv);
    ipaugenblick_main_loop(); 
    munmap(pa,MEM_SIZE);
    close(hugetlb_fd);
    return 0;
}
