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

#define DATAGRAM_LENGTH 1448

int main(int argc,char **argv)
{
    void **buff;
    int sock,len;
    char *p;
    int size = 0,ringset_idx;
    unsigned int from_ip;
    unsigned short from_port;
    unsigned long received_packets = 0;
    unsigned long  sent_packets = 0;

    if(ipaugenblick_app_init(argc,argv) != 0) {
        printf("cannot initialize memory\n");
        return 0;
    } 
    if((sock = ipaugenblick_open_udp(inet_addr("192.168.150.63"),7777)) < 0) {
        printf("cannot open UDP socket\n");
        return 0;
    }
    from_ip = inet_addr("192.168.150.62");
    from_port = htons(7777);
    printf("waiting for creation feedback %d\n",sock);
    sleep(1);
    ipaugenblick_socket_connect(sock,from_ip,ntohs(from_port));
    while(1) {
        buff = ipaugenblick_get_buffer(DATAGRAM_LENGTH);
        if(buff) {
            if(ipaugenblick_sendto(sock,buff,0,DATAGRAM_LENGTH,from_ip,from_port)) { 
//              if(ipaugenblick_send(sock,buff,0,60)) {
                    usleep(1000);
                    ipaugenblick_release_tx_buffer(buff);
        //            ipaugenblick_socket_kick(sock);
            }
            else {
                sent_packets++; 
                if(!(sent_packets%1000)) {
                    ipaugenblick_socket_kick(sock);
                }
                if(!(sent_packets%1000000)) {
                    printf("sent %u\n",sent_packets); 
                }
            }
        } 
    }
    return 0;
}

