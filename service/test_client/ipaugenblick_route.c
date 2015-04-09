#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/errno.h>
#include "../ipaugenblick_app_api/ipaugenblick_api.h"
#include <string.h>

int main(int argc,char **argv)
{
    unsigned int dest_ip,mask,next_hop;

    if(ipaugenblick_app_init(argc,argv) != 0) {
        printf("cannot initialize memory\n");
        return 0;
    }
    dest_ip = inet_addr("192.168.150.0");
    mask = inet_addr("255.255.255.0");
    next_hop = inet_addr("192.168.150.63");
    printf("add %x/%x nh %x\n",dest_ip,mask,next_hop);
    if(ipaugenblick_add_v4_route(dest_ip,mask,next_hop)) {
        printf("cannot add route %s %d\n",__FILE__,__LINE__);
    }
    dest_ip = inet_addr("192.168.150.62");
    mask = inet_addr("255.255.255.255");
    next_hop = inet_addr("192.168.150.63");
    printf("add %x/%x nh %x\n",dest_ip,mask,next_hop);
    if(ipaugenblick_add_v4_route(dest_ip,mask,next_hop)) {
        printf("cannot add route %s %d\n",__FILE__,__LINE__);
    }
    dest_ip = inet_addr("44.44.44.0");
    mask = inet_addr("255.255.255.0");
    next_hop = inet_addr("192.168.150.62");
    printf("add %x/%x nh %x\n",dest_ip,mask,next_hop);
    if(ipaugenblick_add_v4_route(dest_ip,mask,next_hop)) {
        printf("cannot add route %s %d\n",__FILE__,__LINE__);
    }
    dest_ip = inet_addr("55.55.5.0");
    mask = inet_addr("255.255.255.0");
    next_hop = inet_addr("192.168.150.62");
    printf("add %x/%x nh %x\n",dest_ip,mask,next_hop);
    if(ipaugenblick_add_v4_route(dest_ip,mask,next_hop)) {
        printf("cannot add route %s %d\n",__FILE__,__LINE__);
    }
    dest_ip = inet_addr("66.66.66.0");
    mask = inet_addr("255.255.255.0");
    next_hop = inet_addr("192.168.150.123");
    printf("add %x/%x nh %x\n",dest_ip,mask,next_hop);
    if(ipaugenblick_add_v4_route(dest_ip,mask,next_hop)) {
        printf("cannot add route %s %d\n",__FILE__,__LINE__);
    }
    dest_ip = inet_addr("77.77.77.77");
    mask = inet_addr("255.255.255.255");
    next_hop = inet_addr("192.168.150.62");
    printf("add %x/%x nh %x\n",dest_ip,mask,next_hop);
    if(ipaugenblick_add_v4_route(dest_ip,mask,next_hop)) {
        printf("cannot add route %s %d\n",__FILE__,__LINE__);
    }
    dest_ip = inet_addr("88.88.88.88");
    mask = inet_addr("255.255.255.255");
    next_hop = inet_addr("192.168.160.62");
    printf("add %x/%x nh %x\n",dest_ip,mask,next_hop);
    if(ipaugenblick_add_v4_route(dest_ip,mask,next_hop)) {
        printf("cannot add route %s %d\n",__FILE__,__LINE__);
    }
    dest_ip = inet_addr("44.44.44.0");
    mask = inet_addr("255.255.255.0");
    next_hop = inet_addr("192.168.150.62");
    printf("del %x/%x nh %x\n",dest_ip,mask,next_hop);
    if(ipaugenblick_del_v4_route(dest_ip,mask,next_hop)) {
	printf("cannot del route %s %d\n",__FILE__,__LINE__);
    }
    dest_ip = inet_addr("55.55.5.0");
    mask = inet_addr("255.255.255.0");
    next_hop = inet_addr("192.168.150.62");
    printf("del %x/%x nh %x\n",dest_ip,mask,next_hop);
    if(ipaugenblick_del_v4_route(dest_ip,mask,next_hop)) {
        printf("cannot add route %s %d\n",__FILE__,__LINE__);
    }
    dest_ip = inet_addr("66.66.66.0");
    mask = inet_addr("255.255.255.0");
    next_hop = inet_addr("192.168.150.123");
    printf("del %x/%x nh %x\n",dest_ip,mask,next_hop);
    if(ipaugenblick_del_v4_route(dest_ip,mask,next_hop)) {
	printf("cannot del route %s %d\n",__FILE__,__LINE__);
    }
    dest_ip = inet_addr("77.77.77.77");
    mask = inet_addr("255.255.255.255");
    next_hop = inet_addr("192.168.150.123");
    printf("del %x/%x nh %x\n",dest_ip,mask,next_hop);
    if(ipaugenblick_del_v4_route(dest_ip,mask,next_hop)) {
	printf("cannot del route %s %d\n",__FILE__,__LINE__);
    }
    dest_ip = inet_addr("88.88.88.88");
    mask = inet_addr("255.255.255.255");
    next_hop = inet_addr("192.168.150.62");
    printf("del %x/%x nh %x\n",dest_ip,mask,next_hop);
    if(ipaugenblick_del_v4_route(dest_ip,mask,next_hop)) {
	printf("cannot del route %s %d\n",__FILE__,__LINE__);
    }
    return 0;
}

